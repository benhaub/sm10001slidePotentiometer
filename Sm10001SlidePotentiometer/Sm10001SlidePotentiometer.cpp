#include "Sm10001SlidePotentiometer.hpp"
#include "AdcModule.hpp"
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"

ErrorType Sm10001SlidePotentiometer::slidePotentiometerThread() {
    Sm10001 sm10001;
    ErrorType error = sm10001.init(HBRIDGE_PART_NUMBER, PWM_TYPE, PeripheralNumber::Zero, AdcTypes::Channel::Four, PinNumber(SLIDE_POT_PIN_A), PinNumber(SLIDE_POT_PIN_B), Volts(POTENTIOMETER_DROP_MAX));

    if (ErrorType::Success == error) {
        Volts potentiometerVoltageDrop = 0.0f;
        Volts previousVoltageDrop = 0.0f;
        const Volts hysteresis = 0.1f;

        while (1) {
            assert(ErrorType::Success == sm10001.slideForward());
            OperatingSystem::Instance().delay(Milliseconds(5000));
            assert(ErrorType::Success == sm10001.slideBackward());
            OperatingSystem::Instance().delay(Milliseconds(5000));

            if (ErrorType::Success == (error = sm10001.getVoltageDrop(potentiometerVoltageDrop))) {
                if (std::abs(potentiometerVoltageDrop - previousVoltageDrop) > hysteresis) {
                    previousVoltageDrop = potentiometerVoltageDrop;
                    PLT_LOGI(TAG, "Voltage drop: %f", potentiometerVoltageDrop);
                }
            }
            else {
                const bool isCriticalError = !(error == ErrorType::NotImplemented || error == ErrorType::NotSupported || error == ErrorType::NotAvailable);

                if (isCriticalError) {
                    PLT_LOGE(TAG, "Failed to get voltage drop: %d", (uint8_t)error);
                    return error;
                }
            }
        }
    }
    else {
        PLT_LOGE(TAG, "Failed to initialize slide potentiometer <error: %u>", (uint8_t)error);
    }

    return error;
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
