#ifndef AISPEAKERPAGEPRESENTER_HPP
#define AISPEAKERPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class AISpeakerPageView;

class AISpeakerPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    AISpeakerPagePresenter(AISpeakerPageView& v);

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

    virtual ~AISpeakerPagePresenter() {}

    virtual void saveCompleted();

private:
    AISpeakerPagePresenter();

    AISpeakerPageView& view;
};

#endif // AISPEAKERPAGEPRESENTER_HPP
