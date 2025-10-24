#include <gui/fatfstester_screen/fatFSTesterView.hpp>

#ifndef SIMULATOR
extern char *pDirectoryFiles[];
extern uint8_t ubNumberOfFiles;
#endif

fatFSTesterView::fatFSTesterView()
{

}

void fatFSTesterView::setupScreen()
{
    fatFSTesterViewBase::setupScreen();
#ifndef SIMULATOR

    Unicode::snprintf(fileNumTextBuffer, FILENUMTEXT_SIZE, "%d", ubNumberOfFiles);
    fileNumText.invalidate();

    Unicode::strncpy(firstFileTextBuffer, pDirectoryFiles[0], FIRSTFILETEXT_SIZE);
    firstFileText.invalidate();
#endif
}

void fatFSTesterView::tearDownScreen()
{
    fatFSTesterViewBase::tearDownScreen();
}
