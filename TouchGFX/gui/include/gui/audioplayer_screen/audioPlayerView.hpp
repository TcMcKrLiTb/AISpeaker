#ifndef AUDIOPLAYERVIEW_HPP
#define AUDIOPLAYERVIEW_HPP

#include <gui_generated/audioplayer_screen/audioPlayerViewBase.hpp>
#include <gui/audioplayer_screen/audioPlayerPresenter.hpp>
#include "gui/containers/WavFileElement.hpp"

class audioPlayerView : public audioPlayerViewBase
{
public:
    audioPlayerView();
    virtual ~audioPlayerView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    static const int maxFileNum = 25;

    int fileNum = 3, nowFileId = 0, nowVolume = 0;
    int nowStatus = 0; // 0-Ready, 1-ErrorSD, 2-ErrorAllocate, 3-NoFile
    char pDirectoryFiles[maxFileNum][NOWFILETEXT_SIZE];

    void volumeChanged(int value) override;
    void muteFunction() override;
    void disFileList() override;
    void playFunction() override;
    void pauseFunction() override;
    void stopFunction() override;

    void fileElementClicked(WavFileElement& element);

protected:

    WavFileElement tempFileList[maxFileNum];

    Callback<audioPlayerView, WavFileElement&> wavFileElementClickedCallback;
};

#endif // AUDIOPLAYERVIEW_HPP
