#ifndef FATFSTESTERVIEW_HPP
#define FATFSTESTERVIEW_HPP

#include <gui_generated/fatfstester_screen/fatFSTesterViewBase.hpp>
#include <gui/fatfstester_screen/fatFSTesterPresenter.hpp>

class fatFSTesterView : public fatFSTesterViewBase
{
public:
    fatFSTesterView();
    virtual ~fatFSTesterView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // FATFSTESTERVIEW_HPP
