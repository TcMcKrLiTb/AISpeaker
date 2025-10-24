#include <gui/audioplayer_screen/audioPlayerView.hpp>

#ifndef SIMULATOR

#include <cstdio>
#include <cstdlib>

#include "fatfs.h"
#include "audio_rec_play.h"
#include "stm32746g_discovery_audio.h"
#include "texts/TextKeysAndLanguages.hpp"

extern wavInfo wavInfoNow;
extern AUDIO_PLAYBACK_StateTypeDef audio_state;
extern uint32_t uwPauseEnabledStatus;
extern uint8_t audio_pause_flag;
extern osSemaphoreId_t audioSemHandle;

#endif

audioPlayerView::audioPlayerView()
: wavFileElementClickedCallback(this, &audioPlayerView::fileElementClicked)
{

}

void audioPlayerView::setupScreen()
{
    audioPlayerViewBase::setupScreen();
    fileList.setVisible(false);
    fileList.setTouchable(false);
    waveFileList.setVisible(false);
    waveFileList.setTouchable(false);
    errorText.setVisible(false);
    Unicode::snprintf(nowFileTextBuffer, NOWFILETEXT_SIZE, "NULL");
    nowFileText.invalidate();
    nowVolume = 50;
    VolumeSlider.setValue((int16_t)nowVolume);
#ifndef SIMULATOR
    if (BSP_SD_IsDetected() != SD_PRESENT) {
        Unicode::snprintf(errorTextBuffer, ERRORTEXT_SIZE, "%s",
                          touchgfx::TypedText(T_ERRORNOSDCARD).getText());
        errorText.setWildcard(errorTextBuffer);
        errorText.invalidate();
        errorText.setVisible(true);
        nowStatus = 1;
        invalidate();
        return;
    }
    BSP_SD_Init();
    MX_FATFS_Init();
    if (f_mount(&SDFatFS, (TCHAR const*)"", 1) != FR_OK) {
        Unicode::snprintf(errorTextBuffer, ERRORTEXT_SIZE, "%s",
                          touchgfx::TypedText(T_ERRORMOUNTFAILED).getText());
        errorText.setWildcard(errorTextBuffer);
        errorText.invalidate();
        errorText.setVisible(true);
        nowStatus = 1;
        invalidate();
        return;
    }
    printf("ALL OK!!!\r\n");
#endif
}

void audioPlayerView::tearDownScreen()
{
#ifndef SIMULATOR

    audioPlayerViewBase::tearDownScreen();
#endif
}

void audioPlayerView::volumeChanged(int value)
{
    nowVolume = value;
#ifndef SIMULATOR
    if (audio_state == AUDIO_STATE_PLAYING) {
        BSP_AUDIO_OUT_SetVolume((uint8_t)nowVolume);
    }
#endif
    Unicode::snprintf(volumeTextBuffer, VOLUMETEXT_SIZE, "%d", value);
    volumeText.invalidate();
}

void audioPlayerView::muteFunction()
{
    VolumeSlider.setValue(0);
}

void audioPlayerView::fileElementClicked(WavFileElement& element)
{
    uint32_t fileSize = 0, sampFreq = 0;
    Unicode::strncpy(nowFileTextBuffer, element.getSelectedFileName(), NOWFILETEXT_SIZE);
    nowFileText.invalidate();
    fileList.removeAll();
    waveFileList.invalidate();
    fileList.setVisible(false);
    fileList.setTouchable(false);
    fileList.invalidate();
    waveFileList.setVisible(false);
    waveFileList.setTouchable(false);
    waveFileList.invalidate();
#ifndef SIMULATOR
    char fileNameStr[40];
    wavInfo wavInfo1;
    snprintf(fileNameStr, 40, "Audio/%-33.33s", pDirectoryFiles[element.getfileId()]);
    if (Audio_WavHeader(fileNameStr, &wavInfo1) == 0) {
        sampFreq = wavInfo1.sampleRate;
        fileSize = wavInfo1.fileSize;
    } else {
        fileSize = sampFreq = 0;
    }
    nowFileId = element.getfileId();
#else
    fileSize = sampFreq = 0;
#endif
    Unicode::snprintf(fileSizeTextBuffer, FILESIZETEXT_SIZE, "%d", fileSize / 1024);
    volumeText.invalidate();
    Unicode::snprintf(sampleRateTextBuffer, SAMPLERATETEXT_SIZE, "%d", sampFreq);
    volumeText.invalidate();
    fileSelectButton.setTouchable(true);
    invalidate();
}

void audioPlayerView::disFileList()
{
    fileSelectButton.setTouchable(false);
    fileList.setHeight(0);

#ifndef SIMULATOR
    if (BSP_SD_IsDetected() != SD_PRESENT) {
        if (nowStatus != 1)
            FATFS_UnLinkDriver(SDPath);
        Unicode::snprintf(errorTextBuffer, ERRORTEXT_SIZE, "%s",
                          touchgfx::TypedText(T_ERRORNOSDCARD).getText());
        errorText.setWildcard(errorTextBuffer);
        errorText.invalidate();
        errorText.setVisible(true);
        nowStatus = 1;
        fileSelectButton.setTouchable(true);
        invalidate();
        return;
    }
    if (nowStatus == 1) {
        BSP_SD_Init();
        MX_FATFS_Init();
        if (f_mount(&SDFatFS, (TCHAR const*)"", 1) != FR_OK) {
            Unicode::snprintf(errorTextBuffer, ERRORTEXT_SIZE, "%s",
                              touchgfx::TypedText(T_ERRORMOUNTFAILED).getText());
            errorText.setWildcard(errorTextBuffer);
            errorText.invalidate();
            errorText.setVisible(true);
            nowStatus = 1;
            fileSelectButton.setTouchable(true);
            invalidate();
            return;
        } else {
            nowStatus = 0;
            errorText.setVisible(false);
        }
    }
    FILINFO fno;
    DIR dir;
    uint32_t index = 0;
    retSD = f_findfirst(&dir, &fno, "/Audio", "*.wav");
    printf("find ret:%d\r\n", retSD);
    while (fno.fname[0]) {
        if(retSD == FR_OK)
        {
            if(index < maxFileNum)
            {
                snprintf(pDirectoryFiles[index++], NOWFILETEXT_SIZE, "%s", fno.fname);
            }
            /* Search for next item */
            retSD = f_findnext(&dir, &fno);
        }
        else
        {
            index = 0;
            break;
        }
    }

    f_closedir(&dir);
    if (index == 0) {
        tempFileList[0].setupFileElement("NULL\0", 255);
        fileNum = 1;
    } else {
        fileNum = (int) index;
    }
#else
    tempFileList[0].setupFileElement("test1\0", 0);
    tempFileList[1].setupFileElement("test2\0", 1);
    tempFileList[2].setupFileElement("test3\0", 2);

    fileNum = 3;
#endif

    for (uint8_t i = 0; i < (fileNum > maxFileNum ? maxFileNum : fileNum); i++)
    {
        tempFileList[i].setupFileElement(pDirectoryFiles[i], i);
        tempFileList[i].setAction(wavFileElementClickedCallback);
        fileList.add(tempFileList[i]);
    }

    fileList.setVisible(true);
    fileList.setTouchable(true);
    fileList.invalidate();
    waveFileList.setVisible(true);
    waveFileList.setTouchable(true);
    waveFileList.invalidate();
    invalidate();
}

void audioPlayerView::playFunction()
{
#ifndef SIMULATOR
    char fileNameStr[40];
    snprintf(fileNameStr, 40, "Audio/%-33.33s", pDirectoryFiles[this->nowFileId]);

    if (StartPlayback(fileNameStr, this->nowVolume)) {
        printf("Start Play Failed!!\r\n");
    } else {
        printf("Start Play OK!!\r\n");
    }
#endif
}

void audioPlayerView::pauseFunction()
{
#ifndef SIMULATOR
    if (audio_state != AUDIO_STATE_PLAYING) {
        return;
    }
    audio_pause_flag = 1;
    osSemaphoreRelease(audioSemHandle);
#endif
}

void audioPlayerView::stopFunction()
{
#ifndef SIMULATOR

#endif
}

