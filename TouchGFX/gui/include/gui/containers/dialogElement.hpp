#ifndef DIALOGELEMENT_HPP
#define DIALOGELEMENT_HPP

#include <gui_generated/containers/dialogElementBase.hpp>

class dialogElement : public dialogElementBase
{
public:
    dialogElement();
    virtual ~dialogElement() {}

    int16_t setupDialogElement(const char *dialogText, const int nowFileId);

    virtual void initialize();
protected:

private:
    int fileId = 0;
};

#endif // DIALOGELEMENT_HPP
