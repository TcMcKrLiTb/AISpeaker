#ifndef WAVFILEELEMENT_HPP
#define WAVFILEELEMENT_HPP

#include <gui_generated/containers/WavFileElementBase.hpp>

class WavFileElement : public WavFileElementBase
{
public:
    WavFileElement();
    virtual ~WavFileElement() {}

    virtual void initialize();
protected:
};

#endif // WAVFILEELEMENT_HPP
