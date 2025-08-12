/***************************************************************************//**
* @author   Ben Haubrich
* @file     main.cpp
* @date     Wednesday, February 26th, 2025
*******************************************************************************/
//AbstractionLayer
#include "PowerResetClockManagementModule.hpp"
#include "GpioModule.hpp"
#include "Log.hpp"
//AutomaticPetFeeder
#include "Sm10001SlidePotentiometer.hpp"

static void startSlidePotentiometer(Sm10001SlidePotentiometer &slidePotentiometer) {
    Id slidePotentiometerId;

    OperatingSystem::Instance().createThread(OperatingSystemTypes::Priority::Normal,
                                             Sm10001SlidePotentiometer::slidePotentiometerThreadName,
                                             &slidePotentiometer,
                                             APP_DEFAULT_STACK_SIZE,
                                             startSlidePotentiometerThread,
                                             slidePotentiometerId);

    OperatingSystem::Instance().startScheduler();

    //Platforms (especially desktop platforms like Linux and Darwin) that don't have the concept of starting a scheduler will
    //join threads instead. Other targets that run on embedded RTOSs like Azure, Zephyr, and FreeRTOS never return after
    //starting the scheduler.
    assert(ErrorType::NoData != OperatingSystem::Instance().joinThread(Sm10001SlidePotentiometer::slidePotentiometerThreadName));
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
    prcm.setClockFrequency(Hertz(APP_CLOCK_FREQUENCY), Hertz(APP_EXTERNAL_CRYSTAL_FREQUENCY));

    
    gpio.configure(GpioTypes::GpioParams());
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
