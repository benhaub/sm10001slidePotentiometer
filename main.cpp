/***************************************************************************//**
* @author   Ben Haubrich
* @file     main.cpp
* @date     Wednesday, February 26th, 2025
*******************************************************************************/
//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "PowerResetClockManagementModule.hpp"
#include "GpioModule.hpp"
#include "Log.hpp"
//AutomaticPetFeeder
#include "Sm10001SlidePotentiometer.hpp"

static void startSlidePotentiometer(Sm10001SlidePotentiometer &slidePotentiometer) {
    constexpr uint16_t kilobytes = 1024;
#if __XTENSA__
    constexpr uint16_t thisMany = 16;
#else
    constexpr uint16_t thisMany = 2;
#endif

    Id slidePotentiometerId;
    const std::string slidePotentiometerThreadName("slidePotentiometerThread");

    OperatingSystem::Instance().createThread(OperatingSystemConfig::Priority::Normal,
                                             slidePotentiometerThreadName,
                                             &slidePotentiometer,
                                             thisMany*kilobytes,
                                             startSlidePotentiometerThread,
                                             slidePotentiometerId);

    OperatingSystem::Instance().startScheduler();

    //Platforms (especially desktop platforms like Linux and Darwin) that don't have the concept of starting a scheduler will
    //join threads instead. Other targets that run on embedded RTOSs like Azure, Zephyr, and FreeRTOS never return after
    //starting the scheduler.
    OperatingSystem::Instance().joinThread(slidePotentiometerThreadName);
}

static void initGlobals() {
    OperatingSystem::Init();
    Logger::Init();
    Sm10001SlidePotentiometer::Init();
}

#if __XTENSA__
extern "C" void app_main() {
#else
int main(void) {
#endif
    PowerResetClockManagement prcm;
    Gpio gpio;

    prcm.init();

    gpio.setHardwareConfig(nullptr, PinNumber(-1), GpioTypes::PinDirection::DigitalUnknown, GpioTypes::InterruptMode::Unknown, false, false);
    gpio.init();

    initGlobals();

    OperatingSystem::Instance().setTimeOfDay(UnixTime(0), Seconds(0));

    startSlidePotentiometer(Sm10001SlidePotentiometer::Instance());

#if __XTENSA__
    return;
#else
    return 0;
#endif
}