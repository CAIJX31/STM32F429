#include <gui/screen1_screen/Screen1View.hpp>
#ifndef SIMULATOR
#include "main.h"
#endif

namespace
{
#ifndef SIMULATOR
// Update to match the driver microstep setting.
static const uint32_t kStepsPerRevolution = 3200U;
static const uint32_t kStepsPerNinetyDegrees = kStepsPerRevolution / 4U;
// Increase for slower speed, decrease for faster speed.
static const uint32_t kPulseDelayCycles = 3200U;
// Set to 1 to invert left/right direction mapping if wiring is reversed.
static const uint8_t kInvertDirection = 0U;
static const uint8_t kInvertDirectionSecondary = 0U;
// Delay after DIR change so the driver can latch direction.
static const uint32_t kDirSetupDelayCycles = 3200U;
// Set to GPIO_PIN_SET if ENA is active high.
static const GPIO_PinState kEnableActiveState = GPIO_PIN_RESET;
static const GPIO_PinState kDirLeftState = GPIO_PIN_RESET;
static const GPIO_PinState kDirRightState = GPIO_PIN_SET;

void stepperDelayCycles(uint32_t cycles)
{
    while (cycles-- > 0U)
    {
        __NOP();
    }
}

void stepperMove90Degrees(GPIO_TypeDef* port,
                          uint16_t enaPin,
                          uint16_t pulPin,
                          uint16_t dirPin,
                          bool rightDirection,
                          uint8_t invertDirection)
{
    if (invertDirection != 0U)
    {
        rightDirection = !rightDirection;
    }

    HAL_GPIO_WritePin(port, enaPin, kEnableActiveState);
    HAL_GPIO_WritePin(port, dirPin, rightDirection ? kDirRightState : kDirLeftState);
    HAL_GPIO_WritePin(port, pulPin, GPIO_PIN_RESET);
    stepperDelayCycles(kDirSetupDelayCycles);

    for (uint32_t i = 0; i < kStepsPerNinetyDegrees; ++i)
    {
        HAL_GPIO_WritePin(port, pulPin, GPIO_PIN_SET);
        stepperDelayCycles(kPulseDelayCycles);
        HAL_GPIO_WritePin(port, pulPin, GPIO_PIN_RESET);
        stepperDelayCycles(kPulseDelayCycles);
    }
}
#endif
}

Screen1View::Screen1View()
    : flexButtonCallback(this, &Screen1View::flexButtonCallbackHandler)
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    flexButton1.setAction(flexButtonCallback);
    flexButton2.setAction(flexButtonCallback);
    flexButton3.setAction(flexButtonCallback);
    flexButton4.setAction(flexButtonCallback);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src)
{
    if (&src == &flexButton1)
    {
        moveBy90Degrees(false);
    }
    else if (&src == &flexButton2)
    {
        moveBy90Degrees(true);
    }
    else if (&src == &flexButton3)
    {
        moveBy90DegreesSecondary(false);
    }
    else if (&src == &flexButton4)
    {
        moveBy90DegreesSecondary(true);
    }
}

void Screen1View::moveBy90Degrees(bool rightDirection)
{
#ifndef SIMULATOR
    stepperMove90Degrees(GPIOB, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, rightDirection, kInvertDirection);
#else
    (void)rightDirection;
#endif
}

void Screen1View::moveBy90DegreesSecondary(bool rightDirection)
{
#ifndef SIMULATOR
    stepperMove90Degrees(GPIOD, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, rightDirection, kInvertDirectionSecondary);
#else
    (void)rightDirection;
#endif
}
