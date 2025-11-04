#ifndef CHOOSEELEMENT_HPP
#define CHOOSEELEMENT_HPP

#include <gui_generated/containers/chooseElementBase.hpp>

#include "texts/TextKeysAndLanguages.hpp"

class chooseElement : public chooseElementBase
{
public:
    chooseElement();
    virtual ~chooseElement() {}

    virtual void initialize();

    void setupChooseElement(TEXTS textToShow, int theClassId, int theChooseNum);

    void setAction(GenericCallback< chooseElement&, int, int >& callback);

    void selectAction() override;

protected:
    GenericCallback< chooseElement&, int, int>* viewCallback;
private:
    int chooseNum;
    int classId;
};

#endif // CHOOSEELEMENT_HPP
