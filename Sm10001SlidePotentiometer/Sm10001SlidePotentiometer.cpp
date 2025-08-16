#include "Sm10001SlidePotentiometer.hpp"

ErrorType Sm10001SlidePotentiometer::slidePotentiometerThread() {
    Sm10001 sm10001;
    ErrorType error = sm10001.init(HBRIDGE_PART_NUMBER,
                                   PWM_TYPE,
                                   ADC_PERIPHERAL_NUMBER,
                                   AdcTypes::Channel::Four,
                                   PinNumber(SLIDE_POT_PIN_A),
                                   PinNumber(SLIDE_POT_PIN_B),
                                   Volts(MAX_POTENTIOMETER_VOLTAGE_DROP),
                                   Volts(MIN_POTENTIOMETER_VOLTAGE_DROP));

    if (ErrorType::Success == error) {
        Volts potentiometerVoltageDrop = 0.0f;

        PLT_LOGI(TAG, "Starting calibration in 5 seconds. Ensure the slide potentiometer is powered on");
        OperatingSystem::Instance().delay(Milliseconds(5000));
        error = sm10001.calibrate(Count(150), Volts(0.2), Milliseconds(300));

        if (ErrorType::Success != error) {
            PLT_LOGE(TAG, "Failed to calibrate slide potentiometer <error: %u>", (uint8_t)error);
        }
        else {
            PLT_LOGI(Sm10001Types::TAG, "Calibrated <minimumForwardSlideTime (ms):%d, minimumBackwardSlideTime (ms):%d, forward slide voltage effect:%u>",
                sm10001.minimumForwardSlideTime(), sm10001.minimumBackwardSlideTime(), (uint8_t)sm10001.forwardSlideVoltageEffect());
        }

        while (1) {
            assert(ErrorType::Success == sm10001.slideToVoltage(0.0f, 0.1f));
            sm10001.getVoltageDrop(potentiometerVoltageDrop, Sm10001Types::AdcMultiSamples);
            PLT_LOGI(TAG, "Voltage drop: %f", potentiometerVoltageDrop);
            assert(ErrorType::Success == sm10001.slideToVoltage(100.0f, 0.1f));
            sm10001.getVoltageDrop(potentiometerVoltageDrop, Sm10001Types::AdcMultiSamples);
            PLT_LOGI(TAG, "Voltage drop: %f", potentiometerVoltageDrop);
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
