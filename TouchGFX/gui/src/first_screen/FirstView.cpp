#include <gui/first_screen/FirstView.hpp>
#include <touchgfx/Unicode.hpp>
#ifndef SIMULATOR
#include "main.h"
#include "stdio.h"
#endif

FirstView::FirstView()
{

}

void FirstView::setupScreen()
{
    FirstViewBase::setupScreen();

    /* Set graph to display 0-3.3V input */
    dynamicGraph1.clear();
    dynamicGraph1.setGraphRangeY(0.0f, 3300.0f);
    textArea2.setWidth(240);
    textArea2.invalidate();

#ifndef SIMULATOR
    char tmp[20] = {0};
    sprintf(tmp, "%d", HAL_GetUIDw2());
    my_printf("%s\r\n", tmp);
#endif
}

void FirstView::func_btn_entr()
{
    // Override and implement this function in First
}

void FirstView::tearDownScreen()
{
    FirstViewBase::tearDownScreen();
}

void FirstView::updateWaveform(uint32_t frequencyHz, float vppMv, const uint16_t* samples, size_t sampleCount)
{
    const float vppVolts = vppMv / 1000.0f;
    Unicode::snprintfFloat(textArea1Buffer, TEXTAREA1_SIZE, "%.2f", vppVolts);
    textArea1.resizeToCurrentText();
    textArea1.invalidate();

    Unicode::snprintf(textArea2Buffer, TEXTAREA2_SIZE, "%d", static_cast<int>(frequencyHz));
    textArea2.invalidate();

    dynamicGraph1.clear();
    const float scale = 3300.0f / 4095.0f;
    for (size_t i = 0; i < sampleCount; ++i)
    {
        dynamicGraph1.addDataPoint(samples[i] * scale);
    }
    // Force full redraw so traces from previous frames are cleared when amplitude changes
    dynamicGraph1.invalidate();
}

void FirstView::handleTickEvent()
{
    // No periodic work; updates pushed from presenter.
}
