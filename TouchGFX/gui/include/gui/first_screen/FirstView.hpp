#ifndef FIRSTVIEW_HPP
#define FIRSTVIEW_HPP

#include <gui_generated/first_screen/FirstViewBase.hpp>
#include <gui/first_screen/FirstPresenter.hpp>
#include <cstddef>

class FirstView : public FirstViewBase
{
public:
    FirstView();
    virtual ~FirstView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void updateWaveform(uint32_t frequencyHz, float vppMv, const uint16_t* samples, size_t sampleCount);
protected:
		virtual void handleTickEvent();
		//virtual	void moveImage(uint16_t time,uint16_t step);
		virtual void func_btn_entr();
};

#endif // FIRSTVIEW_HPP
