#include <gui/audiorecorder_screen/audioRecorderView.hpp>
#include <texts/TextKeysAndLanguages.hpp>

#ifndef SIMULATOR

#include <cstdio>

#include "main.h"
#include "../../../Drivers/BSP/Components/wm8994/wm8994.h"
#include "stm32746g_discovery_audio.h"
#include "stm32f7xx_hal_i2s.h"
#include "audio_rec_play.h"

extern uint16_t SavedFileNum;
extern uint8_t audio_save_flag;
extern AUDIO_PLAYBACK_StateTypeDef audio_state;
extern osSemaphoreId stopRecordSemHandle;
extern osSemaphoreId saveFiniSemHandle;
extern osThreadId audioFillerTaskHandle;

#endif

audioRecorderView::audioRecorderView()
{

}

void audioRecorderView::setupScreen()
{
    audioRecorderViewBase::setupScreen();

    TEXTS initText = T_DEVICEFAILED;

#ifndef SIMULATOR
    osThreadResume(audioFillerTaskHandle);

    if (BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION,
                          1) == AUDIO_OK) {
        initText = T_DEVICEREADY;
    } else {
        initText = T_DEVICEFAILED;
    }
#endif

    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(initText).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
    textArea3.setVisible(false);
    textArea3.invalidate();

    saveButton.setTouchable(false);
}

void audioRecorderView::tearDownScreen()
{
    audioRecorderViewBase::tearDownScreen();

#ifndef SIMULATOR
    osThreadSuspend(audioFillerTaskHandle);
#endif
}

void audioRecorderView::startRecord()
{
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(T_STARTRECORD).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
    startRecordButton.setTouchable(false);
    stopRecordButton.setTouchable(true);

#ifndef SIMULATOR
    if (StartRecord()) {
        printf("Start Record Failed!!\r\n");
    } else {
        printf("Start Record OK!!\r\n");
    }
#endif
}

void audioRecorderView::stopRecord()
{
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(T_STOPRECORD).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
    startRecordButton.setTouchable(true);
    stopRecordButton.setTouchable(false);
    saveButton.setTouchable(true);

#ifndef SIMULATOR
    if (audio_state == AUDIO_STATE_RECORDING) {
        osSemaphoreRelease(stopRecordSemHandle);
    }
#endif
}

void audioRecorderView::startSaving()
{
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(T_SAVING).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
    saveButton.setTouchable(false);

#ifndef SIMULATOR
    audio_save_flag = 1;
#endif
}

void audioRecorderView::saveCompleted()
{
#ifndef SIMULATOR
    if (audio_save_flag == 0)
        return;
    audio_save_flag = 0;
    Unicode::snprintf(textArea3Buffer, TEXTAREA3_SIZE, "RECORDED%d.WAV", SavedFileNum);
#endif
    textArea3.setWildcard(textArea3Buffer);
    textArea3.setVisible(true);
    textArea3.invalidate();
}
