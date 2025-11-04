#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32746g_discovery_audio.h"
#include "audio_rec_play.h"
#include "cmsis_os.h"
#include "wm8994.h"
#include "fatfs.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t external_buffer[SDRAM_AUDIO_BLOCK_SIZE] __attribute__((section("AudioBuffer"))) __attribute__((aligned(32)));

extern SDRAM_HandleTypeDef hsdram1;

__IO uint8_t sdFileOpened = 0;
__IO uint32_t sdFileReadOffset = 0; // offset of already read in to SDRAM
AUDIO_PLAYBACK_StateTypeDef audio_state;
uint32_t AudioFileSize;
uint32_t AudioStartAddress;
uint32_t AudioFilePlayed;
uint16_t SavedFileNum = 1;
__IO uint32_t uwVolume = 20;
__IO uint32_t uwPauseEnabledStatus = 0;
__IO uint8_t audio_out_error_flag;
__IO uint8_t audio_in_error_flag;
__IO uint8_t audio_pause_flag;
__IO uint8_t audio_save_flag;
__IO uint8_t immediate_save;
ALIGN_32BYTES (AUDIO_BufferTypeDef  buffer_ctl);
char playingFileName[40];
wavInfo wavInfoNow;

extern osSemaphoreId audioSemHandle;
extern osSemaphoreId stopRecordSemHandle;
extern osSemaphoreId saveFiniSemHandle;
extern osThreadId audioFillerTaskHandle;
extern uint32_t nowFileSize;

extern DMA_HandleTypeDef hdma_sai2_b;

// temperate buffer to storage some info
ALIGN_32BYTES (uint8_t sector[FILE_READ_ONCE_BYTES]);

void audioFiller_Task(const void *argument);

uint32_t Audio_WavHeader(const char* WavName, wavInfo *info)
{
    FIL headerFile;
    UINT br;
    if (f_open(&headerFile, (TCHAR const*)WavName, FA_READ)) {
        f_close(&headerFile);
        return 2; // open failed
    }
    sdFileOpened = 1;
    retSD = f_lseek(&headerFile, 0);
    if (retSD != FR_OK) {
        f_close(&headerFile);
        return 3; // seek failed
    }
    retSD = f_read(&headerFile, sector, 44, &br);

    if (retSD != FR_OK || br < 44) {
        f_close(&headerFile);
        return 4; // file is too small
    }
    // check "RIFF"
    if (memcmp(sector, "RIFF", 4) != 0)
        return 5; // not a RIFF file
    // check "WAVE"
    if (memcmp(sector + 8, "WAVE", 4) != 0)
        return 6; // not a WAVE file

    // read in RIFF size
    uint32_t fileSize = *(uint8_t *) (sector + 4);
    fileSize |= (*(uint8_t *) (sector + 5)) << 8;
    fileSize |= (*(uint8_t *) (sector + 6)) << 16;
    fileSize |= (*(uint8_t *) (sector + 7)) << 24;
    info->fileSize = fileSize + 8;

    uint8_t foundFmt = 0;
    uint8_t foundData = 0;
    // start from RIFF chunk ended
    uint32_t filePos = 0xC;
    retSD = f_lseek(&headerFile, filePos);
    if (retSD != FR_OK) {
        f_close(&headerFile);
        return 3; // seek failed
    }
    while (1) {
        uint32_t chunkSize = 0;
        // read in chunk flag and size
        retSD = f_read(&headerFile, sector, 8, &br);
        if (retSD != FR_OK || br < 8) {
            break; // no more chunks
        }

        if (strncmp("fmt ", (char*)sector, 4) == 0 && foundFmt == 0) {
            // fmt chunk must be at least 16 bytes for PCM-like V1
            chunkSize = *(uint8_t *) (sector + 4);
            chunkSize |= (*(uint8_t *) (sector + 5)) << 8;
            chunkSize |= (*(uint8_t *) (sector + 6)) << 16;
            chunkSize |= (*(uint8_t *) (sector + 7)) << 24;
            if (chunkSize < 16) {
                f_close(&headerFile);
                return 7; // fmt chunk is too short. hei hei
            }
            // jump above the fmt header
            filePos += 8;
            retSD = f_lseek(&headerFile, filePos);
            if (retSD != FR_OK) {
                f_close(&headerFile);
                return 3; // seek failed
            }
            // read in fmt chunk
            retSD = f_read(&headerFile, sector, chunkSize, &br);
            if (retSD != FR_OK || br < chunkSize) {
                // file is too small
                f_close(&headerFile);
                return 4;
            }
            uint16_t audioFormat = *(uint8_t *) sector;
            audioFormat |= *(uint8_t*)(sector + 1) << 8;
            if (audioFormat != 1) {
                // only support PCM encoding
                f_close(&headerFile);
                return 8;
            }
            info->numChannels = *(uint8_t *)(sector + 2);
            info->numChannels |= *(uint8_t *)(sector + 3) << 8;
            info->sampleRate = *(uint8_t *)(sector + 4);
            info->sampleRate |= *(uint8_t *)(sector + 5) << 8;
            info->sampleRate |= *(uint8_t *)(sector + 6) << 16;
            info->sampleRate |= *(uint8_t *)(sector + 7) << 24;
            info->bitsPerSample = *(uint8_t *)(sector + 14);
            info->bitsPerSample |= *(uint8_t *)(sector + 15);
            // read any extra fmt bytes (for extensible or other)
            const uint32_t fmtRead = 16;
            if (chunkSize > fmtRead) {
                // todo: process extra fmt bytes
            }
            foundFmt = 1;
            // jump over chunkSize
            filePos += chunkSize;
        } else if (strncmp("data", (char*)sector, 4) == 0 && foundData == 0) {
            chunkSize = *(uint8_t *) (sector + 4);
            chunkSize |= (*(uint8_t *) (sector + 5)) << 8;
            chunkSize |= (*(uint8_t *) (sector + 6)) << 16;
            chunkSize |= (*(uint8_t *) (sector + 7)) << 24;
            info->dataChunkSize = chunkSize;
            info->dataChunkPos = filePos + 8;
            filePos += 8;
            foundData = 1;
        } else {
            filePos += 2;
        }
        // all found
        if (foundData == 1 && foundFmt == 1) {
            break;
        }
        // test next pos
        retSD = f_lseek(&headerFile, filePos);
        if (retSD != FR_OK) {
            f_close(&headerFile);
            return 3; // seek failed
        }
    }

    const FSIZE_t fileRealSize = f_size(&headerFile);

    // if file real size smaller than header described, fix it.
    if (fileRealSize < (info->fileSize)) {
        info->dataChunkSize = fileRealSize - info->fileSize;
        info->fileSize = fileRealSize;
    }

    retSD = f_close(&headerFile);
    if (retSD != FR_OK)
        return 8; // close failed
    sdFileOpened = 0;
    if (foundData == 1 && foundFmt == 1) {
        return 0;
    }
    // file is not complete
    return 9;
}

static uint32_t GetData(void *pdata, const uint32_t offset, uint8_t *pBuf, const uint32_t NbrOfData)
{
    const uint8_t *lPtr = pdata;

    uint32_t ReadDataNbr = 0;
    while(((offset + ReadDataNbr) < AudioFileSize * 2 / wavInfoNow.numChannels) && (ReadDataNbr < NbrOfData)) {
        pBuf[ReadDataNbr]= lPtr [offset + ReadDataNbr];
        ReadDataNbr++;
    }
    return ReadDataNbr;
}

static uint32_t SaveDataToSDCard()
{
    FIL saveFile;
    UINT bw;
    FILINFO fno;
    DIR dir;
    char patternStr[40];
    // open file
    if (sdFileOpened) {
        f_close(&saveFile);
        sdFileOpened = 0;
    }
    do {
        snprintf(patternStr, 40, "RECORDED%d.WAV", SavedFileNum);
        retSD = f_findfirst(&dir, &fno, "/Audio", patternStr);
        if (retSD != FR_OK) {
            printf("Failed to search existing recorded files! ret:%d\r\n", retSD);
            return 1;
        }
    } while (fno.fname[0] != 0 && ++SavedFileNum < 1000);
    snprintf((char *)sector, 100, "/Audio/%s", patternStr);
    retSD = f_open(&saveFile, (char *)sector, FA_CREATE_ALWAYS | FA_WRITE);
    sdFileOpened = 1;
    if (retSD != FR_OK) {
        printf("Failed to open file for recording save! ret:%d\r\nfile name is:%s\r\n", retSD, sector);
        f_close(&saveFile);
        sdFileOpened = 0;
        return 1;
    }
    // write WAV header
    uint8_t wavHeader[44] = {0};
    // RIFF header
    memcpy(wavHeader, "RIFF", 4);
    uint32_t fileSize = buffer_ctl.fptr + 36;
    nowFileSize = fileSize + 8;
    wavHeader[4] = (uint8_t)(fileSize & 0xFF);
    wavHeader[5] = (uint8_t)((fileSize >> 8) & 0xFF);
    wavHeader[6] = (uint8_t)((fileSize >> 16) & 0xFF);
    wavHeader[7] = (uint8_t)((fileSize >> 24) & 0xFF);
    memcpy(wavHeader + 8, "WAVE", 4);
    // fmt chunk
    memcpy(wavHeader + 12, "fmt ", 4);
    wavHeader[16] = 16; // Subchunk1Size for PCM
    wavHeader[20] = 1; // AudioFormat PCM
    wavHeader[22] = DEFAULT_AUDIO_IN_CHANNEL_NBR; // NumChannels
    wavHeader[24] = (uint8_t)(I2S_AUDIOFREQ_16K & 0xFF);
    wavHeader[25] = (uint8_t)((I2S_AUDIOFREQ_16K >> 8) & 0xFF);
    wavHeader[26] = (uint8_t)((I2S_AUDIOFREQ_16K >> 16) & 0xFF);
    wavHeader[27] = (uint8_t)((I2S_AUDIOFREQ_16K >> 24) & 0xFF);
    const uint32_t byteRate = I2S_AUDIOFREQ_16K * DEFAULT_AUDIO_IN_CHANNEL_NBR * (DEFAULT_AUDIO_IN_BIT_RESOLUTION / 8);
    wavHeader[28] = (uint8_t)(byteRate & 0xFF);
    wavHeader[29] = (uint8_t)((byteRate >> 8) & 0xFF);
    wavHeader[30] = (uint8_t)((byteRate >> 16) & 0xFF);
    wavHeader[31] = (uint8_t)((byteRate >> 24) & 0xFF);
    const uint16_t blockAlign = DEFAULT_AUDIO_IN_CHANNEL_NBR * (DEFAULT_AUDIO_IN_BIT_RESOLUTION / 8);
    wavHeader[32] = (uint8_t)(blockAlign & 0xFF);
    wavHeader[33] = (uint8_t)((blockAlign >> 8) & 0xFF);
    wavHeader[34] = DEFAULT_AUDIO_IN_BIT_RESOLUTION; // BitsPerSample
    // data chunk
    memcpy(wavHeader + 36, "data", 4);
    wavHeader[40] = (uint8_t)(buffer_ctl.fptr & 0xFF);
    wavHeader[41] = (uint8_t)((buffer_ctl.fptr >> 8) & 0xFF);
    wavHeader[42] = (uint8_t)((buffer_ctl.fptr >> 16) & 0xFF);
    wavHeader[43] = (uint8_t)((buffer_ctl.fptr >> 24) & 0xFF);
    retSD = f_write(&saveFile, wavHeader, 44, &bw);
    if (retSD != FR_OK || bw < 44) {
        printf("Failed to write WAV header!ret:%d\r\n, file name is:%s\r\n", retSD, (char*)sector);
        f_close(&saveFile);
        sdFileOpened = 0;
        return 2;
    }
    // write audio data
    retSD = f_write(&saveFile, (uint32_t *)AudioStartAddress, buffer_ctl.fptr, &bw);
    if (retSD != FR_OK || bw < buffer_ctl.fptr) {
        printf("Failed to write audio data!ret:%d\r\n, file name is:%s\r\n", retSD, (char*)sector);
        f_close(&saveFile);
        sdFileOpened = 0;
        return 3;
    }
    retSD = f_close(&saveFile);
    if (retSD != FR_OK) {
        printf("Failed to close file after recording save!ret:%d\r\n", retSD);
        sdFileOpened = 0;
        return 4;
    }
    sdFileOpened = 0;
    return 0;
}

/**
 * AI is suck!
 * This function create by myself works great
 * btr should always be actualSize * numChannels, for example, if the file only contains one channel, btr should
 * always be divided by 2.
 * @param fileHandler
 * @param fileOffset
 * @param SDRAMOffset
 * @param btr
 * @param numChannels
 * @return
 */
int8_t readDataFromFileToSDRAM(FIL* fileHandler, const uint32_t fileOffset, const uint32_t SDRAMOffset,
                               const uint32_t btr, const uint8_t numChannels)
{
    uint32_t bytesAlreadyRead = 0;
    UINT bytesRead = 0;
    retSD = f_lseek(fileHandler, fileOffset);
    if (retSD != FR_OK) {
        printf("lseek file failed, ret: %d\r\n", retSD);
        return 1;
    }
    if (numChannels == 1) {
        while (bytesAlreadyRead < btr) {
            const UINT toRead = (btr - bytesAlreadyRead) < FILE_READ_ONCE_BYTES ? btr - bytesAlreadyRead : FILE_READ_ONCE_BYTES;
            retSD = f_read(fileHandler, sector, toRead, &bytesRead);
            if (retSD != FR_OK) {
                printf("read file failed, ret: %d\r\n", retSD);
                return 2;
            }
            SCB_CleanDCache_by_Addr((uint32_t *) &sector[0], FILE_READ_ONCE_BYTES);
            for (uint32_t i = 0; i < bytesRead; i++) {
                external_buffer[SDRAMOffset + (bytesAlreadyRead << 1) + (i << 1)] = sector[i];
                external_buffer[SDRAMOffset + (bytesAlreadyRead << 1) + (i << 1) + 1] = sector[i];
            }
            SCB_CleanDCache_by_Addr((uint32_t *) &sector[0], FILE_READ_ONCE_BYTES);
            bytesAlreadyRead += bytesRead;
        }
    } else {
        while (bytesAlreadyRead < btr) {
            const UINT toRead = (btr - bytesAlreadyRead) < FILE_READ_ONCE_BYTES ? btr - bytesAlreadyRead : FILE_READ_ONCE_BYTES;
            f_read(fileHandler, sector, toRead, &bytesRead);
            if (retSD != FR_OK) {
                printf("read file failed, ret: %d\r\n", retSD);
                return 2;
            }
            SCB_CleanDCache_by_Addr((uint32_t *) &sector[0], FILE_READ_ONCE_BYTES);
            for (uint32_t i = 0; i < bytesRead; i++) {
                external_buffer[SDRAMOffset + bytesAlreadyRead + i] = sector[i];
            }
            SCB_CleanDCache_by_Addr((uint32_t *) &sector[0], FILE_READ_ONCE_BYTES);
            bytesAlreadyRead += bytesRead;
        }
    }
    return 0;
}

/**
 * start to play wav File first time to read start DMA
 * @param path
 * @param initVolume
 * @return
 */
uint8_t StartPlayback(const char* path, const uint16_t initVolume)
{
    FIL startFile;

    osThreadDef(audioFillerTask, audioFiller_Task, osPriorityHigh, 0, 2048);
    audioFillerTaskHandle = osThreadCreate(osThread(audioFillerTask), NULL);

    memcpy(playingFileName, path, strlen(path));

    if (sdFileOpened) {
        sdFileOpened = 0;
    }

    if (Audio_WavHeader(path, &wavInfoNow)) {
        return 1; // wav header error
    }
    // openFile
    retSD = f_open(&startFile, playingFileName, FA_READ);
    if (retSD != FR_OK) {
        f_close(&startFile);
        return 2; // open failed
    }
    sdFileOpened = 1;

    // only support 16-bit stereo PCM
    if (wavInfoNow.bitsPerSample != 16) {// || wavInfoNow.numChannels != 2) {
        f_close(&startFile);
        sdFileOpened = 0;
        return 3; // unsupported format
    }

    // reset pointers
    sdFileReadOffset = 0;

    // firstly move wav data chunk first Initial_read_bytes read in SDRAM (start in dataChunkPos)
    uint32_t toRead = INITIAL_READ_BYTES * wavInfoNow.numChannels / 2;
    // // read all the file into SDRAM
    // toRead = wavInfoNow.dataChunkSize;
    // if (readDataFromFileToSDRAM(wavInfoNow.dataChunkPos + sdFileReadOffset, 0, toRead) != FR_OK) {
    //     f_close(&startFile);
    //     sdFileOpened = 0;
    //     return 4; // read failed
    // }
    if (toRead > wavInfoNow.dataChunkSize) {
        toRead = wavInfoNow.dataChunkSize;
    }
    if (readDataFromFileToSDRAM(&startFile, wavInfoNow.dataChunkPos, 0, toRead,
        wavInfoNow.numChannels) != 0) {
        f_close(&startFile);
        sdFileOpened = 0;
        return 4; // read failed
    }
    // use actual bytes read
    sdFileReadOffset += toRead;

    retSD = f_close(&startFile);
    if (retSD != FR_OK)
        return 5; // close failed
    sdFileOpened = 0;

    uwPauseEnabledStatus = 0; /* 0 when audio is running, 1 when Pause is on */
    audio_pause_flag = 0;
    uwVolume = initVolume;

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_SPEAKER, uwVolume, wavInfoNow.sampleRate) == 0) {
        printf("AUDIO CODEC   OK  \r\n");
    }
    else {
        printf("AUDIO CODEC   FAILED \r\n");
        if (sdFileOpened) {
            f_close(&startFile);
            sdFileOpened = 0;
        }
        return 6; // audio init failed
    }

    buffer_ctl.state = BUFFER_OFFSET_NONE;
    AudioStartAddress = (uint32_t)external_buffer;
    // actual data size should be chunkSize * channelNum
    AudioFileSize = wavInfoNow.dataChunkSize * wavInfoNow.numChannels;
    AudioFilePlayed = 0; //INITIAL_READ_BYTES * wavInfoNow.numChannels / 2;
    const uint32_t bytesRead = GetData((void *) AudioStartAddress,
                                 0,
                                 &buffer_ctl.buff[0],
                                 AUDIO_BUFFER_SIZE);

    // start play audio
    if(bytesRead > 0)
    {
        /* Clean Data Cache to update the content of the SRAM */
        SCB_CleanDCache_by_Addr((uint32_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE/2);

        BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE);
        audio_state = AUDIO_STATE_PLAYING;
        buffer_ctl.fptr = bytesRead;
        return 0;
    }

    return 7; // start error
}

uint8_t StartRecord()
{
    immediate_save = 0;

    osThreadDef(audioFillerTask, audioFiller_Task, osPriorityHigh, 0, 2048);
    audioFillerTaskHandle = osThreadCreate(osThread(audioFillerTask), NULL);

    if (BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MICROPHONE_2,
                            I2S_AUDIOFREQ_16K,
                            DEFAULT_AUDIO_IN_BIT_RESOLUTION,
                            DEFAULT_AUDIO_IN_CHANNEL_NBR) == AUDIO_OK) {
        printf("AUDIO RECORD CODEC OK!!!\r\n");
    } else {
        printf("AUDIO RECORD CODEC FAILED!!!\r\n");
        return 1;
    }

    AudioStartAddress = (uint32_t)external_buffer;
    buffer_ctl.state = BUFFER_OFFSET_NONE;
    buffer_ctl.fptr = 0;
    audio_state = AUDIO_STATE_RECORDING;
    return BSP_AUDIO_IN_Record((uint16_t*)&buffer_ctl.buff[0], AUDIO_BLOCK_SIZE);
}

void audioFiller_Task(const void *argument)
{
    uint32_t bytesRead;
    FIL fillFile;
    (void)argument;

    /* infinite loop */
    for (;;) {
        if (audio_out_error_flag) {
            audio_out_error_flag = 0;
            printf("Audio ERROR callback occurred, stopping playback\r\n");
            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            sdFileOpened = 0;
            osThreadTerminate(audioFillerTaskHandle);
            break;
        }

        if (audio_in_error_flag) {
            audio_in_error_flag = 0;
            printf("Audio ERROR callback occurred, stopping record\r\n");
            BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
            osThreadTerminate(audioFillerTaskHandle);
            break;
        }

        if (audio_save_flag) {
            printf("Saving recorded data to SD card...\r\n");
            if (SaveDataToSDCard() == 0) {
                printf("Recording saved successfully!\r\n");
            } else {
                printf("Failed to save recording!\r\n");
            }
            osSemaphoreRelease(saveFiniSemHandle);
            osThreadTerminate(audioFillerTaskHandle);
            break;
        }

        switch (audio_state) {
            case AUDIO_STATE_PLAYING:
                if (buffer_ctl.fptr * wavInfoNow.numChannels / 2 + AudioFilePlayed >= AudioFileSize) {
                    /* Play audio sample again ... */
                    buffer_ctl.fptr = 0;
                    printf("play stopped!\r\n");
                    audio_state = AUDIO_STATE_IDLE;
                    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                    osThreadTerminate(audioFillerTaskHandle);
                    break;
                }

                /* 1st half buffer played; so fill it and continue playing from bottom*/
                if (buffer_ctl.state == BUFFER_OFFSET_HALF) {
                    bytesRead = GetData((void *) AudioStartAddress,
                                        buffer_ctl.fptr,
                                        &buffer_ctl.buff[0],
                                        AUDIO_BUFFER_SIZE / 2);
                    if (bytesRead > 0) {
                        buffer_ctl.state = BUFFER_OFFSET_NONE;
                        buffer_ctl.fptr += bytesRead;

                        /* Clean Data Cache to update the content of the SRAM */
                        SCB_CleanDCache_by_Addr((uint32_t *) &buffer_ctl.buff[0], AUDIO_BUFFER_SIZE / 2);
                    }
                }

                /* 2nd half buffer played; so fill it and continue playing from top */
                if (buffer_ctl.state == BUFFER_OFFSET_FULL) {
                    bytesRead = GetData((void *) AudioStartAddress,
                                        buffer_ctl.fptr,
                                        &buffer_ctl.buff[AUDIO_BUFFER_SIZE / 2],
                                        AUDIO_BUFFER_SIZE / 2);
                    if (bytesRead > 0) {
                        buffer_ctl.state = BUFFER_OFFSET_NONE;
                        buffer_ctl.fptr += bytesRead;

                        /* Clean Data Cache to update the content of the SRAM */
                        SCB_CleanDCache_by_Addr((uint32_t *) &buffer_ctl.buff[AUDIO_BUFFER_SIZE / 2],
                                                AUDIO_BUFFER_SIZE / 2);
                    }
                }
                if (buffer_ctl.fptr == EVERYTIME_READ_BYTES) {
                    UINT toRead = EVERYTIME_READ_BYTES * wavInfoNow.numChannels / 2;
                    if (toRead > wavInfoNow.fileSize - sdFileReadOffset - wavInfoNow.dataChunkPos) {
                        toRead = wavInfoNow.fileSize - sdFileReadOffset - wavInfoNow.dataChunkPos;
                    }
                    retSD = f_open(&fillFile, playingFileName, FA_READ);
                    if (retSD != FR_OK) {
                        f_close(&fillFile);
                        printf("error when refilling SDRAM, open file failed, ret:%d", retSD);
                        audio_out_error_flag = 1;
                    }
                    sdFileOpened = 1;
                    if (readDataFromFileToSDRAM(&fillFile, wavInfoNow.dataChunkPos + sdFileReadOffset,
                        0, toRead, wavInfoNow.numChannels) != 0) {
                        f_close(&fillFile);
                        printf("error when refilling SDRAM, read file failed");
                        audio_out_error_flag = 1;

                    }
                    sdFileReadOffset += toRead;
                    retSD = f_close(&fillFile);
                    if (retSD != FR_OK) {
                        printf("error when refilling SDRAM, close file failed, ret:%d", retSD);
                        audio_out_error_flag = 1;
                    }
                }
                if (buffer_ctl.fptr == INITIAL_READ_BYTES) {
                    UINT toRead = EVERYTIME_READ_BYTES * wavInfoNow.numChannels / 2;
                    if (toRead > wavInfoNow.fileSize - sdFileReadOffset - wavInfoNow.dataChunkPos) {
                        toRead = wavInfoNow.fileSize - sdFileReadOffset - wavInfoNow.dataChunkPos;
                    }
                    retSD = f_open(&fillFile, playingFileName, FA_READ);
                    if (retSD != FR_OK) {
                        f_close(&fillFile);
                        printf("error when refilling SDRAM, open file failed, ret:%d", retSD);
                        audio_out_error_flag = 1;
                    }
                    sdFileOpened = 1;
                    if (readDataFromFileToSDRAM(&fillFile, wavInfoNow.dataChunkPos + sdFileReadOffset,
                        EVERYTIME_READ_BYTES, toRead, wavInfoNow.numChannels) != 0) {
                        f_close(&fillFile);
                        printf("error when refilling SDRAM, read file failed");
                        audio_out_error_flag = 1;
                    }
                    sdFileReadOffset += toRead;
                    retSD = f_close(&fillFile);
                    if (retSD != FR_OK) {
                        printf("error when refilling SDRAM, close file failed, ret:%d", retSD);
                        audio_out_error_flag = 1;
                    }
                    buffer_ctl.fptr = 0;
                    AudioFilePlayed += INITIAL_READ_BYTES * wavInfoNow.numChannels / 2;
                    printf("play returned to 0 of SDRAM\r\n");
                }
                break;
            case AUDIO_STATE_RECORDING:
                /* 1st half buffer recorded; so write these data into SDRAM*/
                if (buffer_ctl.state == BUFFER_OFFSET_HALF) {
                    buffer_ctl.state = BUFFER_OFFSET_NONE;
                    memcpy((uint32_t *)(AudioStartAddress + buffer_ctl.fptr),
                           &buffer_ctl.buff[0],
                           AUDIO_BLOCK_SIZE);

                    buffer_ctl.fptr += AUDIO_BLOCK_SIZE;
                    printf("buffer Half, Recorded %lu bytes\r\n", buffer_ctl.fptr);
                }

                /* 2nd half buffer recorded; so write these data into SDRAM*/
                if (buffer_ctl.state == BUFFER_OFFSET_FULL) {
                    buffer_ctl.state = BUFFER_OFFSET_NONE;

                    memcpy((uint32_t *)(AudioStartAddress + buffer_ctl.fptr),
                           &buffer_ctl.buff[AUDIO_BLOCK_SIZE],
                           AUDIO_BLOCK_SIZE);

                    buffer_ctl.fptr += AUDIO_BLOCK_SIZE;

                    printf("buffer Full, Recorded %lu bytes\r\n", buffer_ctl.fptr);
                }

                if (buffer_ctl.fptr >= SDRAM_AUDIO_BLOCK_SIZE) {
                    printf("Data Overflow! Recording stopped!!\r\n");
                    BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
                    audio_state = AUDIO_STATE_IDLE;
                }
                break;
            case AUDIO_STATE_IDLE:
                break;

            default:
                printf("audio error not ready!\r\n");
                break;
        }
        osDelay(1);
        if (osOK == osSemaphoreWait(audioSemHandle, 0) && audio_state == AUDIO_STATE_PLAYING) {
            if (buffer_ctl.state == BUFFER_OFFSET_HALF || buffer_ctl.state == BUFFER_OFFSET_FULL) {
                /* delay this job, do it in next loop */
                osSemaphoreRelease(audioSemHandle);
                printf("Audio busy, skipping pause/resume\r\n");
            } else if (audio_pause_flag) {
                if (uwPauseEnabledStatus == 1) {
                    const int ret = BSP_AUDIO_OUT_Resume();
                    if (ret == 0) {
                        printf("Resume OK!!\r\n");
                        uwPauseEnabledStatus = 0;
                    } else {
                        printf("Resume FAILED (ret=%d). Stopping audio to reset state.\r\n", ret);
                        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                        audio_state = AUDIO_STATE_IDLE;
                        uwPauseEnabledStatus = 0;
                    }
                    osDelay(1);
                } else {
                    const int ret = BSP_AUDIO_OUT_Pause();
                    if (ret == 0) {
                        printf("Pause OK!!\r\n");
                        uwPauseEnabledStatus = 1;
                    } else {
                        printf("Pause FAILED (ret=%d). Stopping audio to reset state.\r\n", ret);
                        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                        audio_state = AUDIO_STATE_IDLE;
                        uwPauseEnabledStatus = 0;
                    }
                    osDelay(1);
                }
                audio_pause_flag = 0;
                osDelay(1);
            }
        }
        if (osOK == osSemaphoreWait(stopRecordSemHandle, 0) && audio_state == AUDIO_STATE_RECORDING) {
            BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
            uint32_t dmaCurrentPos = __HAL_DMA_GET_COUNTER(&hdma_sai2_b);
            uint32_t remainingDataSize = dmaCurrentPos > (AUDIO_BLOCK_SIZE / 2) ?
                    dmaCurrentPos - AUDIO_BLOCK_SIZE / 2 :
                    AUDIO_BLOCK_SIZE / 2 - dmaCurrentPos;
            dmaCurrentPos = dmaCurrentPos > (AUDIO_BLOCK_SIZE / 2) ?
                    AUDIO_BLOCK_SIZE / 2 :
                    0;
            if (remainingDataSize > 0) {
                // copy remaining data to SDRAM
                memcpy((uint32_t *)(AudioStartAddress + buffer_ctl.fptr),
                       &buffer_ctl.buff[dmaCurrentPos],
                       remainingDataSize);
                buffer_ctl.fptr += remainingDataSize;
                printf("Remaining data copied to SDRAM, total recorded size: %lu bytes\r\n", buffer_ctl.fptr);
            } else {
                printf("No remaining data to copy, total recorded size: %lu bytes\r\n", buffer_ctl.fptr);
            }
            audio_state = AUDIO_STATE_IDLE;
            printf("Recording stopped by user!!\r\n");
            if (immediate_save == 1) {
                audio_save_flag = 1;
                immediate_save = 0;
            }
        }
    }
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    if(audio_state == AUDIO_STATE_PLAYING)
    {
        /* allows AUDIO_Process() to refill 2nd part of the buffer  */
        buffer_ctl.state = BUFFER_OFFSET_FULL;
    }
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    if(audio_state == AUDIO_STATE_PLAYING)
    {
        /* allows AUDIO_Process() to refill 1st part of the buffer  */
        buffer_ctl.state = BUFFER_OFFSET_HALF;
    }
}

void BSP_AUDIO_OUT_Error_CallBack(void)
{
    audio_state = AUDIO_STATE_IDLE;
    audio_out_error_flag = 1;
}

void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
    if(audio_state == AUDIO_STATE_RECORDING) {
        /* allows AUDIO_Process() to refill 1st part of the buffer  */
        buffer_ctl.state = BUFFER_OFFSET_FULL;
    } else {
        audio_in_error_flag = 1;
    }
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
    if(audio_state == AUDIO_STATE_RECORDING) {
        /* allows AUDIO_Process() to refill 1st part of the buffer  */
        buffer_ctl.state = BUFFER_OFFSET_HALF;
    } else {
        audio_in_error_flag = 1;
    }
}

void BSP_AUDIO_IN_Error_CallBack(void)
{
    audio_state = AUDIO_STATE_IDLE;
    audio_in_error_flag = 1;
}

void HAL_SDRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
    printf("DMA Done!!\r\n");
}

#ifdef __cplusplus
};
#endif

