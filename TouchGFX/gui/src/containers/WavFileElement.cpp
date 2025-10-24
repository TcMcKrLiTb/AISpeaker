#include <gui/containers/WavFileElement.hpp>

WavFileElement::WavFileElement():
viewCallback(nullptr), fileId(0)
{

}

void WavFileElement::initialize()
{
    WavFileElementBase::initialize();
//    fileSelectButton.setTouchable(true);
}

void WavFileElement::setupFileElement(const char *fileName, int pFileId)
{
    Unicode::strncpy(fileNameTextBuffer, fileName, FILENAMETEXT_SIZE);
    fileNameText.invalidate();
    invalidate();
    this->fileId = pFileId;
}

void WavFileElement::setAction(GenericCallback<WavFileElement &> &callback)
{
    viewCallback = &callback;
}

void WavFileElement::selectAction()
{
    if (viewCallback->isValid()) {
        viewCallback->execute(*this);
    }
}

Unicode::UnicodeChar *WavFileElement::getSelectedFileName()
{
    return fileNameTextBuffer;
}

int WavFileElement::getfileId() const
{
    return this->fileId;
}
