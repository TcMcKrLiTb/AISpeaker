#include <gui/aispeakerpage_screen/AISpeakerPageView.hpp>

#ifndef SIMULATOR

#include <cstdio>

#include "main.h"
#include "../../../Drivers/BSP/Components/wm8994/wm8994.h"
#include "audio_rec_play.h"

extern uint8_t audio_save_flag;
extern uint8_t immediate_save;
extern uint16_t SavedFileNum;
extern AUDIO_PLAYBACK_StateTypeDef audio_state;
extern osSemaphoreId stopRecordSemHandle;
extern osSemaphoreId saveFiniSemHandle;
extern osThreadId audioFillerTaskHandle;

#endif

AISpeakerPageView::AISpeakerPageView()
= default;

void AISpeakerPageView::setupScreen()
{
    AISpeakerPageViewBase::setupScreen();
    animatedImage1.stopAnimation();
    animatedImage1.setVisible(false);
    animatedImage1.invalidate();
    startTalkButton.setVisible(true);
    startTalkButton.setTouchable(true);
    stopTalkButton.setVisible(false);
    stopTalkButton.setTouchable(false);
}

void AISpeakerPageView::tearDownScreen()
{
    AISpeakerPageViewBase::tearDownScreen();
}

void AISpeakerPageView::startTalk()
{
    startTalkButton.setVisible(false);
    startTalkButton.setTouchable(false);
    stopTalkButton.setVisible(true);
    stopTalkButton.setTouchable(true);
    startTalkButton.invalidate();
    stopTalkButton.invalidate();

#ifndef SIMULATOR
    if (StartRecord()) {
        printf("Start Record Failed!!\r\n");
    } else {
        printf("Start Record OK!!\r\n");
    }
#endif
}

void AISpeakerPageView::stopTalk()
{
    startTalkButton.setVisible(true);
    stopTalkButton.setVisible(false);
    stopTalkButton.setTouchable(false);
    animatedImage1.setVisible(true);
    animatedImage1.startAnimation(false, true, true);

#ifndef SIMULATOR
    immediate_save = 1;
    if (audio_state == AUDIO_STATE_RECORDING) {
        osSemaphoreRelease(stopRecordSemHandle);
    } else {
        animatedImage1.stopAnimation();
        animatedImage1.setVisible(false);
        startTalkButton.setTouchable(true);
    }
#endif
}

void AISpeakerPageView::saveCompleted()
{
#ifndef SIMULATOR
    if (audio_save_flag == 0) {
        // not in saving state
        return;
    }
    audio_save_flag = 0;
    printf("save completed!, file is: RECORDED%d.WAV\r\n", SavedFileNum);
#endif
    animatedImage1.stopAnimation();
    animatedImage1.setVisible(false);
    animatedImage1.invalidate();
    startTalkButton.setTouchable(true);
}
