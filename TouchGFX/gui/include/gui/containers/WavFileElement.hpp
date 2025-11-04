#ifndef WAVFILEELEMENT_HPP
#define WAVFILEELEMENT_HPP

#include <gui_generated/containers/WavFileElementBase.hpp>

class WavFileElement : public WavFileElementBase
{
public:
    WavFileElement();
    virtual ~WavFileElement() {}

    void setupFileElement(const char *fileName, int pFileId);

    void setAction(GenericCallback< WavFileElement& >& callback);

    void selectAction() override;

    Unicode::UnicodeChar *getSelectedFileName();

    int getfileId() const;

    virtual void initialize();
protected:
    GenericCallback< WavFileElement&>* viewCallback;

private:
    // use to specify which file is referred by this element
    int fileId;
};

#endif // WAVFILEELEMENT_HPP
