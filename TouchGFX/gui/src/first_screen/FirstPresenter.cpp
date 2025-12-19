#include <gui/first_screen/FirstView.hpp>
#include <gui/first_screen/FirstPresenter.hpp>

FirstPresenter::FirstPresenter(FirstView& v)
    : view(v)
{

}

void FirstPresenter::activate()
{

}

void FirstPresenter::deactivate()
{

}

void FirstPresenter::NotifyWaveform(uint32_t frequencyHz, float vppMv, const uint16_t* samples, size_t sampleCount)
{
    view.updateWaveform(frequencyHz, vppMv, samples, sampleCount);
}
