#ifndef ETHERNETTESTERVIEW_HPP
#define ETHERNETTESTERVIEW_HPP

#include <gui_generated/ethernettester_screen/EthernetTesterViewBase.hpp>
#include <gui/ethernettester_screen/EthernetTesterPresenter.hpp>

class EthernetTesterView : public EthernetTesterViewBase
{
public:
    EthernetTesterView();
    virtual ~EthernetTesterView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void function1() override;

protected:
};

#endif // ETHERNETTESTERVIEW_HPP
