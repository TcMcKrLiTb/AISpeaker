#include <gui/audiorecorder_screen/audioRecorderView.hpp>
#include <gui/audiorecorder_screen/audioRecorderPresenter.hpp>

audioRecorderPresenter::audioRecorderPresenter(audioRecorderView& v)
        : view(v)
{

}

void audioRecorderPresenter::activate()
{

}

void audioRecorderPresenter::deactivate()
{

}

void audioRecorderPresenter::saveCompleted()
{
    view.saveCompleted();
}
