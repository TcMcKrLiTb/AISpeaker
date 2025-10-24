#ifndef WAVFILECONTAINER_HPP
#define WAVFILECONTAINER_HPP

#include <gui_generated/containers/WavFileContainerBase.hpp>
#include "texts/TextKeysAndLanguages.hpp"

class WavFileContainer : public WavFileContainerBase
{
public:
    WavFileContainer();
    virtual ~WavFileContainer() {}

    void setupWavFileElement(char* fileName);


    virtual void initialize();
protected:
};

#endif // WAVFILECONTAINER_HPP
