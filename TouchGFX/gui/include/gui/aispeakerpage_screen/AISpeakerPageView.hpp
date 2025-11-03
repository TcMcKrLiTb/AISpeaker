#ifndef AISPEAKERPAGEVIEW_HPP
#define AISPEAKERPAGEVIEW_HPP

#include <gui_generated/aispeakerpage_screen/AISpeakerPageViewBase.hpp>
#include <gui/aispeakerpage_screen/AISpeakerPagePresenter.hpp>

#include "gui/containers/chooseElement.hpp"

#ifndef SIMULATOR
#include "network.h"
#else

typedef enum {
    SPEED_NORMAL = 0,
    SPEED_FAST,
    SPEED_SLOW,
    SPEED_MAX_NUM
} ReplySpeed;

typedef enum {
    EMOTION_MIDDLE = 0,
    EMOTION_HAPPY,
    EMOTION_SAD,
    EMOTION_SERIOUS,
    EMOTION_MAX_NUM
} ReplyEmotion;

#endif

#include "gui/containers/dialogElement.hpp"
#include "texts/TextKeysAndLanguages.hpp"

class AISpeakerPageView : public AISpeakerPageViewBase
{
public:
    AISpeakerPageView();
    virtual ~AISpeakerPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    static constexpr int maxDialogNum = 30;

    int dialogNum = 3;
    int nowTotalHeight = 0;

    void startTalk() override;
    void stopTalk() override;
    // void testFuncEveryNTicks() override;
    void handleTickEvent() override;

    virtual void saveCompleted();
    virtual void networkCompleted();

    void speedClickHandle(const TextAreaWithOneWildcard& b, const ClickEvent& e);
    void emotionClickHandle(const TextAreaWithOneWildcard& b, const ClickEvent& e);

    void chooseElementClicked(chooseElement& element, int classId, int chooseId);

protected:

    void addOneAIMessage(const char* messageStr, int nowFileId);
    void scrollWithAnimation();

    dialogElement tempDialogList[maxDialogNum];
    chooseElement speedChooseList[SPEED_MAX_NUM];
    chooseElement emotionChooseList[EMOTION_MAX_NUM];

    Callback<AISpeakerPageView, const TextAreaWithOneWildcard&, const ClickEvent&> speedClickedCallback;
    Callback<AISpeakerPageView, const TextAreaWithOneWildcard&, const ClickEvent&> emotionClickedCallback;

    Callback<AISpeakerPageView, chooseElement&, int, int> chooseElementClickedCallback;

private:

    // static constexpr uint32_t TICK_TESTINTER_INTERVAL = 40;
    int scrollGoal;
    int animationCounter;
    bool animationIsRunning;
    const TEXTS speedTextList[SPEED_MAX_NUM];
    const TEXTS emotionTextList[EMOTION_MAX_NUM];
    // uint32_t frameCountTestInterInterval;
};

#endif // AISPEAKERPAGEVIEW_HPP
