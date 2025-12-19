#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/containers/buttons/AbstractButtonContainer.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
private:
    touchgfx::Callback<Screen1View, const touchgfx::AbstractButtonContainer&> flexButtonCallback;
    void flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src);
    void moveBy90Degrees(bool rightDirection);
    void moveBy90DegreesSecondary(bool rightDirection);
};

#endif // SCREEN1VIEW_HPP
