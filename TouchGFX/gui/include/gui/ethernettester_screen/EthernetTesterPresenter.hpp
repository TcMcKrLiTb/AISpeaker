#ifndef ETHERNETTESTERPRESENTER_HPP
#define ETHERNETTESTERPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class EthernetTesterView;

class EthernetTesterPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    EthernetTesterPresenter(EthernetTesterView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~EthernetTesterPresenter() {}

private:
    EthernetTesterPresenter();

    EthernetTesterView& view;
};

#endif // ETHERNETTESTERPRESENTER_HPP
