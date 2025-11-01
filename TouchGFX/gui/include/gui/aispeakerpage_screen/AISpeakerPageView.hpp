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

    void startTalk() override;
    void stopTalk() override;

    virtual void saveCompleted();
    virtual void networkCompleted();

protected:

    dialogElement tempDialogList[maxDialogNum];
};

#endif // AISPEAKERPAGEVIEW_HPP
