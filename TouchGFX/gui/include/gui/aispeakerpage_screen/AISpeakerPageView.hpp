#ifndef AISPEAKERPAGEVIEW_HPP
#define AISPEAKERPAGEVIEW_HPP

#include <gui_generated/aispeakerpage_screen/AISpeakerPageViewBase.hpp>
#include <gui/aispeakerpage_screen/AISpeakerPagePresenter.hpp>

class AISpeakerPageView : public AISpeakerPageViewBase
{
public:
    AISpeakerPageView();
    virtual ~AISpeakerPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // AISPEAKERPAGEVIEW_HPP
