#include <cstdio>
#include <cstring>
#include <gui/containers/dialogElement.hpp>

dialogElement::dialogElement():
fileId(0){

}

int16_t dialogElement::setupDialogElement(const char *dialogText, const int nowFileId)
{
    Unicode::fromUTF8(reinterpret_cast<const uint8_t *>(dialogText), dialogInfoBuffer, strlen(dialogText));
    dialogInfo.setWideTextAction(touchgfx::WIDE_TEXT_CHARWRAP);
    dialogInfo.resizeHeightToCurrentText();
    dialogInfo.invalidate();
    boxWithBorder1.setHeight(static_cast<int16_t>(dialogInfo.getHeight() + 11));
    this->setHeight(static_cast<int16_t>(dialogInfo.getHeight() + 18));
    invalidate();
    this->fileId = nowFileId;
    return this->getHeight();
}


void dialogElement::initialize()
{
    dialogElementBase::initialize();
}
