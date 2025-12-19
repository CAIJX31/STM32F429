#ifndef FIRSTPRESENTER_HPP
#define FIRSTPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class FirstView;

class FirstPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    FirstPresenter(FirstView& v);

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

    virtual void NotifyWaveform(uint32_t frequencyHz, float vppMv, const uint16_t* samples, size_t sampleCount);

    virtual ~FirstPresenter() {};

private:
    FirstPresenter();

    FirstView& view;
};

#endif // FIRSTPRESENTER_HPP
