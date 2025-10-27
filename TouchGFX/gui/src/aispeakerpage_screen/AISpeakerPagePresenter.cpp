#include <gui/aispeakerpage_screen/AISpeakerPageView.hpp>
#include <gui/aispeakerpage_screen/AISpeakerPagePresenter.hpp>

AISpeakerPagePresenter::AISpeakerPagePresenter(AISpeakerPageView& v)
    : view(v)
{

}

void AISpeakerPagePresenter::activate()
{

}

void AISpeakerPagePresenter::deactivate()
{

}

void AISpeakerPagePresenter::saveCompleted()
{
    view.saveCompleted();
}
