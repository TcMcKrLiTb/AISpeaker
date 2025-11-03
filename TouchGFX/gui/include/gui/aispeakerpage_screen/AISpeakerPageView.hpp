#ifndef AISPEAKERPAGEVIEW_HPP
#define AISPEAKERPAGEVIEW_HPP

#include <gui_generated/aispeakerpage_screen/AISpeakerPageViewBase.hpp>
#include <gui/aispeakerpage_screen/AISpeakerPagePresenter.hpp>
#include "gui/containers/dialogElement.hpp"

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

protected:

    void addOneAIMessage(const char* messageStr, int nowFileId);
    void scrollWithAnimation();

    dialogElement tempDialogList[maxDialogNum];
private:

    // static constexpr uint32_t TICK_TESTINTER_INTERVAL = 40;
    int scrollGoal;
    int animationCounter;
    bool animationIsRunning;
    // uint32_t frameCountTestInterInterval;
};

#endif // AISPEAKERPAGEVIEW_HPP
