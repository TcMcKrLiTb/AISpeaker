#include <gui/containers/chooseElement.hpp>

chooseElement::chooseElement():
viewCallback(nullptr), chooseNum(0)
{

}

void chooseElement::initialize()
{
    chooseElementBase::initialize();
}

void chooseElement::setupChooseElement(const TEXTS textToShow, const int theClassId, const int theChooseNum)
{
    Unicode::snprintf(chooseInfoBuffer, CHOOSEINFO_SIZE, "%s",
        touchgfx::TypedText(textToShow).getText());
    chooseInfo.resizeToCurrentText();
    chooseInfo.invalidate();
    invalidate();
    this->chooseNum = theChooseNum;
    this->classId = theClassId;
}

void chooseElement::selectAction()
{
    if (viewCallback->isValid()) {
        viewCallback->execute(*this, this->classId, this->chooseNum);
    }
}

void chooseElement::setAction(GenericCallback<chooseElement &, int, int> &callback)
{
    viewCallback = &callback;
}

