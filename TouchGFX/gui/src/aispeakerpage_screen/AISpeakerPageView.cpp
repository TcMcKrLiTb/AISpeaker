#include <gui/aispeakerpage_screen/AISpeakerPageView.hpp>

#ifndef SIMULATOR

#include <cstdio>

#include "../../../Drivers/BSP/Components/wm8994/wm8994.h"
#include "audio_rec_play.h"
#include "network.h"

extern uint8_t audio_save_flag;
extern uint8_t immediate_save;
extern uint16_t SavedFileNum;
extern AUDIO_PLAYBACK_StateTypeDef audio_state;
extern osSemaphoreId stopRecordSemHandle;
extern osSemaphoreId saveFiniSemHandle;
extern osThreadId audioFillerTaskHandle;
extern uint16_t AIRepliedFileNum;

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

    tempDialogList[0].setupDialogElement("test这是一条\0", 0);
    tempDialogList[1].setupDialogElement("这个是一条非常长的内容你好！北京交通大学是一所以工科为主的知名大学，特别是在交通运输、信息与通信工程等领域有很强的实力。学校坐落在首都北京，拥有悠久的历史和丰富的学术资源。如果你对工科感兴趣，北交大是一个不错的选择哦！\0",
                                         1);
    tempDialogList[2].setupDialogElement("这个还是一条非常长的内容：ST是法国一家专注于半导体和微电子的公司。它成立于1965年，总部位于法国里昂。ST在芯片设计、制造和销售方面都有很强的实力，产品广泛应用于汽车电子、工业控制、消费电子等领域。ST在全球范围内都有广泛的业务网络和合作伙伴。想了解更多详细信息吗？\0",
                                         2);
    tempDialogList[3].setupDialogElement("test这是一条\0", 0);
    tempDialogList[4].setupDialogElement("test这是一条\0", 0);
    tempDialogList[5].setupDialogElement("test这是一条\0", 0);
    tempDialogList[6].setupDialogElement("test这是一条\0", 0);
    tempDialogList[7].setupDialogElement("test这是一条\0", 0);
    tempDialogList[8].setupDialogElement("test这是一条\0", 0);
    tempDialogList[9].setupDialogElement("test这是一条\0", 0);
    tempDialogList[10].setupDialogElement("test这是一条\0", 0);

    dialogNum = 11;

    for (int i = 0; i < dialogNum; i++) {
        dialogList.add(tempDialogList[i]);
    }
    dialogList.invalidate();
    dialogContainer.invalidate();
    invalidate();
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
    startFileSending();
    // this->networkCompleted();
#endif
}

void AISpeakerPageView::networkCompleted()
{
#ifndef SIMULATOR
    printf("network task completed!, now start to play the audio\r\n");
    char fileNameStr[40];

    snprintf(fileNameStr, 40, "/AIReplied/Audio%d.wav", AIRepliedFileNum);

    if (StartPlayback(fileNameStr, 100)) {
        printf("Start Play Failed!!\r\n");
    } else {
        printf("Start Play OK!!\r\n");
    }
#endif

    animatedImage1.stopAnimation();
    animatedImage1.setVisible(false);
    animatedImage1.invalidate();
    startTalkButton.setTouchable(true);
}
