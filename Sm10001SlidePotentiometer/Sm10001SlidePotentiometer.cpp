#include "Sm10001SlidePotentiometer.hpp"
#include "Sm10001.hpp"
#include "Log.hpp"
#include "OperatingSystemModule.hpp"
#include "AdcModule.hpp"

ErrorType Sm10001SlidePotentiometer::slidePotentiometerThread() {
    std::unique_ptr<Sm10001Drivers::HBridge> hBridge = std::make_unique<Sm10001Drivers::Drv8872>();
    assert(nullptr != hBridge.get());
    std::unique_ptr<Adc> adc = std::make_unique<Adc>();
    assert(nullptr != adc.get());
    adc->peripheralNumber() = PeripheralNumber::Zero;
    adc->channel() = AdcTypes::Channel::Four;
    assert(ErrorType::Success == adc->init());
    std::vector<GptmPwmModule> gptPwms;
    gptPwms.reserve(2);
    gptPwms.emplace_back(GptmPwmModule());
    gptPwms.at(0).peripheralNumber() = PeripheralNumber::Zero;
    gptPwms.emplace_back(GptmPwmModule());
    gptPwms.at(1).peripheralNumber() = PeripheralNumber::One;

    hBridge->setPwms(gptPwms);

    Sm10001 sm10001(hBridge, adc, PinNumber(0), PinNumber(2));
    assert(ErrorType::Success == sm10001.init());

    Volts potentiometerVoltageDrop = 0.0f;
    Volts previousVoltageDrop = 0.0f;
    const Volts hysteresis = 0.1f;
    while (1) {
        assert(ErrorType::Success == sm10001.slideForward());
        OperatingSystem::Instance().delay(Milliseconds(5000));
        assert(ErrorType::Success == sm10001.slideBackward());
        OperatingSystem::Instance().delay(Milliseconds(5000));
        assert(ErrorType::Success == sm10001.getVoltageDrop(potentiometerVoltageDrop));
        if (std::abs(potentiometerVoltageDrop - previousVoltageDrop) > hysteresis) {
            previousVoltageDrop = potentiometerVoltageDrop;
            PLT_LOGI(TAG, "Voltage drop: %f", potentiometerVoltageDrop);
        }
    }
}

#ifdef __cplusplus
extern "C" {
#endif
void *startSlidePotentiometerThread(void *arg) {
    ErrorType error = static_cast<Sm10001SlidePotentiometer *>(arg)->slidePotentiometerThread();
    PLT_LOGW(Sm10001SlidePotentiometer::TAG, "Slide potentiometer thread exited with error %u", (uint8_t)error);
    return nullptr;
}
#ifdef __cplusplus
}
#endif