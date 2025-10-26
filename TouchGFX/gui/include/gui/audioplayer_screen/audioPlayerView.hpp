#ifndef AUDIOPLAYERVIEW_HPP
#define AUDIOPLAYERVIEW_HPP

#include <gui_generated/audioplayer_screen/audioPlayerViewBase.hpp>
#include <gui/audioplayer_screen/audioPlayerPresenter.hpp>

class audioPlayerView : public audioPlayerViewBase
{
public:
    audioPlayerView();
    virtual ~audioPlayerView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // AUDIOPLAYERVIEW_HPP
