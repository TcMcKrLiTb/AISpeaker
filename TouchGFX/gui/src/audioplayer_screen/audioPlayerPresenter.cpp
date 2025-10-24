#include <gui/audioplayer_screen/audioPlayerView.hpp>
#include <gui/audioplayer_screen/audioPlayerPresenter.hpp>

#ifndef SIMULATOR

#include <cstdio>

extern uint16_t SavedFileNum;

#endif

audioPlayerPresenter::audioPlayerPresenter(audioPlayerView& v)
    : view(v)
{

}

void audioPlayerPresenter::activate()
{

}

void audioPlayerPresenter::deactivate()
{

}

void audioPlayerPresenter::saveCompleted()
{

}
