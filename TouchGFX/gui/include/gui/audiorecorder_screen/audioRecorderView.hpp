#ifndef AUDIORECORDERVIEW_HPP
#define AUDIORECORDERVIEW_HPP

#include <gui_generated/audiorecorder_screen/audioRecorderViewBase.hpp>
#include <gui/audiorecorder_screen/audioRecorderPresenter.hpp>

class audioRecorderView : public audioRecorderViewBase
{
public:
    audioRecorderView();
    virtual ~audioRecorderView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void startRecord();
    virtual void stopRecord();
    virtual void startSaving();

protected:
};

#endif // AUDIORECORDERVIEW_HPP
