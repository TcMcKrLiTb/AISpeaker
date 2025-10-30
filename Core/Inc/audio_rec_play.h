//
// Created by 31613 on 2025/10/17.
//
#ifndef __AUDIO_REC_PLAY_H__
#define __AUDIO_REC_PLAY_H__

#define        4096//SRAM里的缓冲区大小，4K
#define AUDIO_DEFAULT_VOLUME    70

#define AUDIO_BLOCK_SIZE        (32768U)//32K,好像是录音时单次处理的数据块大小

#define I2S_AUDIOFREQ_16K       (16000U)

// make a 1MB buffer——事2MB罢
#define SDRAM_AUDIO_BLOCK_SIZE   ((uint32_t)(2 * 1024U * 1024U))//位于SDRAM的块大小2M，作为SRAM和SD卡的中继
// read in 32k at start to cut down delay
#define INITIAL_READ_BYTES       ((uint32_t)(200U * 1024U))
AUDIO_BUFFER_SIZE
#ifdef __cplusplus
extern "C" {
#endif

#include "fatfs.h"

typedef struct {
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    uint16_t numChannels;
    uint32_t fileSize;
    uint32_t dataChunkPos;
    uint32_t dataChunkSize;
} wavInfo;

typedef enum {
    AUDIO_STATE_IDLE = 0,
    AUDIO_STATE_INIT,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_RECORDING
}AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
    BUFFER_OFFSET_NONE = 0,
    BUFFER_OFFSET_HALF,
    BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

typedef struct {
    uint8_t buff[AUDIO_BLOCK_SIZE * 2];//SRAM总大小是64K，录音时候要从SRAM>SDRAM>SD卡，块是32K；播放时从SD卡>SDRAM>SRAM，块是2K，屌
    uint32_t fptr;
    BUFFER_StateTypeDef state;
}AUDIO_BufferTypeDef;

typedef enum {
    NONE=0,
    HALF,
    FULL
} dma_flag_t;

uint32_t Audio_WavHeader(const char* WavName, wavInfo *info);
FRESULT SD_ReadToSDRAM_block(uint32_t file_offset, uint32_t sdram_write_pos, uint32_t bytes_to_read);
uint8_t StartPlayback(const char* path, uint16_t initVolume);
uint8_t StartRecord();

#ifdef __cplusplus
}
#endif

#endif //__AUDIO_REC_PLAY_H__
