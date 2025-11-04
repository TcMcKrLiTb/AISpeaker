#include <gui/aispeakerpage_screen/AISpeakerPageView.hpp>
#include <touchgfx/EasingEquations.hpp>

#include <cstdio>

#ifndef SIMULATOR

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
extern uint8_t streamBuffer[];
extern ReplySpeed nowSettingSpeed;
extern ReplyEmotion nowSettingEmotion;

#endif

AISpeakerPageView::AISpeakerPageView():
speedClickedCallback(this, &AISpeakerPageView::speedClickHandle),
emotionClickedCallback(this, &AISpeakerPageView::emotionClickHandle),
chooseElementClickedCallback(this, &AISpeakerPageView::chooseElementClicked),
scrollGoal(0),animationCounter(0),
animationIsRunning(false),
speedTextList{T_NORMALSPEED, T_FASTSPEED, T_SLOWSPEED},
emotionTextList{T_MIDDLEEMOTION, T_HAPPYEMOTION, T_SADEMOTION, T_SERIOUSEMOTION}
{

}

void AISpeakerPageView::setupScreen()
{
    dialogList.setHeight(0);
    AISpeakerPageViewBase::setupScreen();
    animatedImage1.stopAnimation();
    animatedImage1.setVisible(false);
    animatedImage1.invalidate();
    startTalkButton.setVisible(true);
    startTalkButton.setTouchable(true);
    stopTalkButton.setVisible(false);
    stopTalkButton.setTouchable(false);
    dialogList.invalidate();
    dialogContainer.invalidate();
    Unicode::snprintf(speedInstructionBuffer, SPEEDINSTRUCTION_SIZE, "%s",
        touchgfx::TypedText(speedTextList[SPEED_NORMAL]).getText());
    Unicode::snprintf(emotionInstructionBuffer, EMOTIONINSTRUCTION_SIZE, "%s",
        touchgfx::TypedText(emotionTextList[EMOTION_MIDDLE]).getText());
    speedInstruction.resizeToCurrentText();
    speedInstruction.invalidate();
    emotionInstruction.resizeToCurrentText();
    emotionInstruction.invalidate();
    speedInstruction.setClickAction(speedClickedCallback);
    emotionInstruction.setClickAction(emotionClickedCallback);

    speedList.setHeight(0);
    emotionList.setHeight(0);
    for (int i = 0; i < SPEED_MAX_NUM; i++) {
        speedChooseList[i].setupChooseElement(speedTextList[i], 0, i);
        speedChooseList[i].setAction(chooseElementClickedCallback);
        speedList.add(speedChooseList[i]);
    }
    for (int i = 0; i < EMOTION_MAX_NUM; i++) {
        emotionChooseList[i].setupChooseElement(emotionTextList[i], 1, i);
        emotionChooseList[i].setAction(chooseElementClickedCallback);
        emotionList.add(emotionChooseList[i]);
    }

    speedList.setVisible(false);
    speedContainer.setVisible(false);
    speedList.setTouchable(false);
    speedContainer.setTouchable(false);
    speedList.invalidate();
    speedContainer.invalidate();
    emotionList.setVisible(false);
    emotionContainer.setVisible(false);
    emotionList.setTouchable(false);
    emotionContainer.setTouchable(false);
    emotionList.invalidate();
    emotionContainer.invalidate();


    invalidate();
}

void AISpeakerPageView::addOneAIMessage(const char* messageStr, const int nowFileId)
{
    this->dialogNum++;
    int16_t changeHeight;
    if (dialogNum > maxDialogNum) {
        changeHeight = tempDialogList[this->dialogNum % maxDialogNum].getHeight();
        dialogList.remove(tempDialogList[this->dialogNum % maxDialogNum]);
        this->nowTotalHeight -= changeHeight;
    }
    changeHeight = tempDialogList[this->dialogNum % maxDialogNum].
            setupDialogElement(messageStr, nowFileId);
    dialogList.add(tempDialogList[this->dialogNum % maxDialogNum]);
    this->nowTotalHeight += changeHeight;
    dialogList.invalidate();
    dialogContainer.invalidate();
    scrollGoal = 0 - changeHeight;
    animationIsRunning = true;
    dialogList.invalidate();
    dialogContainer.invalidate();
}
//
// void AISpeakerPageView::testFuncEveryNTicks()
// {
//     static int times = 0;
//     times++;
//     char str[100];
//     snprintf(str, 100, "测试一下刷屏%d嘿嘿", times);
//     addOneAIMessage(str, 0);
// }

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

    addOneAIMessage(reinterpret_cast<char *>(streamBuffer), AIRepliedFileNum);
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

void AISpeakerPageView::handleTickEvent()
{
    // frameCountTestInterInterval++;
    // if(frameCountTestInterInterval == TICK_TESTINTER_INTERVAL)
    // {
    //     //TestInter
    //     //When every N tick call virtual function
    //     //Call testFuncEveryNTicks
    //     testFuncEveryNTicks();
    //     frameCountTestInterInterval = 0;
    // }
    if (animationIsRunning)
    {
        scrollWithAnimation();
    }
}

void AISpeakerPageView::scrollWithAnimation()
{
    constexpr int duration = 20;

    if (animationCounter <= duration)
    {
        const int16_t delta = EasingEquations::linearEaseIn(animationCounter, 0, static_cast<int16_t>(scrollGoal), duration);
        dialogContainer.doScroll(0, delta);  //If scrolling vertically
        dialogContainer.invalidate();
        animationCounter++;
    }
    else
    {
        animationCounter = 0;
        animationIsRunning = false;
    }
}

void AISpeakerPageView::speedClickHandle(const TextAreaWithOneWildcard &b, const ClickEvent &e)
{
    if (e.getType() == ClickEvent::PRESSED) {
        speedList.setVisible(true);
        speedContainer.setVisible(true);
        speedContainer.setTouchable(true);
        speedList.invalidate();
        speedContainer.invalidate();
    }
}

void AISpeakerPageView::emotionClickHandle(const TextAreaWithOneWildcard &b, const ClickEvent &e)
{
    if (e.getType() == ClickEvent::PRESSED) {
        emotionList.setVisible(true);
        emotionContainer.setVisible(true);
        emotionContainer.setTouchable(true);
        emotionList.invalidate();
        emotionContainer.invalidate();
    }
}

void AISpeakerPageView::chooseElementClicked(chooseElement& element, const int classId, const int chooseId)
{
    if (classId == 0) {
        speedList.setVisible(false);
        speedContainer.setVisible(false);
        speedContainer.setTouchable(false);
        speedList.invalidate();
        speedContainer.invalidate();
#ifndef SIMULATOR
        nowSettingSpeed = static_cast<ReplySpeed>(chooseId);
#endif
        Unicode::snprintf(speedInstructionBuffer, SPEEDINSTRUCTION_SIZE, "%s",
            touchgfx::TypedText(speedTextList[chooseId]).getText());
        speedInstruction.resizeToCurrentText();
        speedInstruction.invalidate();
    } else if (classId == 1) {
        emotionList.setVisible(false);
        emotionContainer.setVisible(false);
        emotionContainer.setTouchable(false);
        emotionList.invalidate();
        emotionContainer.invalidate();
#ifndef SIMULATOR
        nowSettingEmotion = static_cast<ReplyEmotion>(chooseId);
#endif
        Unicode::snprintf(emotionInstructionBuffer, EMOTIONINSTRUCTION_SIZE, "%s",
            touchgfx::TypedText(emotionTextList[chooseId]).getText());
        emotionInstruction.resizeToCurrentText();
        emotionInstruction.invalidate();
    }
}
