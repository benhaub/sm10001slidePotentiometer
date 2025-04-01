#include "Sm10001SlidePotentiometer.hpp"
#include "Log.hpp"
#include "OperatingSystemModule.hpp"
#include "AdcModule.hpp"
#include "HBridge.hpp"
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"

ErrorType Sm10001SlidePotentiometer::slidePotentiometerThread() {
    ErrorType error = ErrorType::Failure;
    std::unique_ptr<Sm10001> sm10001;

    if (ErrorType::Success != (error = initSlidePot(PWM_TYPE, sm10001))) {
        PLT_LOGE(TAG, "Failed to initialize slide potentiometer <error: %u>", (uint8_t)error);
        return error;
    }

    Volts potentiometerVoltageDrop = 0.0f;
    Volts previousVoltageDrop = 0.0f;
    const Volts hysteresis = 0.1f;
    while (1) {
        assert(ErrorType::Success == sm10001->slideForward());
        OperatingSystem::Instance().delay(Milliseconds(5000));
        assert(ErrorType::Success == sm10001->slideBackward());
        OperatingSystem::Instance().delay(Milliseconds(5000));
        if (ErrorType::Success == (error = sm10001->getVoltageDrop(potentiometerVoltageDrop))) {
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

ErrorType Sm10001SlidePotentiometer::initSlidePot(Sm10001SlidePotentiometerTypes::PwmType pwmType, std::unique_ptr<Sm10001> &sm10001) {
    if (pwmType == Sm10001SlidePotentiometerTypes::PwmType::Gptm) {
        ErrorType error = ErrorType::Failure;

        std::unique_ptr<HBridgeAbstraction> hBridge = std::make_unique<HBridge>();
        assert(nullptr != hBridge.get());
        std::unique_ptr<AdcAbstraction> adc = std::make_unique<Adc>();
        assert(nullptr != adc.get());
        adc->peripheralNumber() = PeripheralNumber::Zero;
        adc->channel() = AdcTypes::Channel::Four;
        if (ErrorType::Success != (error = adc->init())) {
            if (ErrorType::NotImplemented == error) {
                PLT_LOGI(TAG, "Adc not implemented for this platform.");
            }
        }
        std::array<std::unique_ptr<GptmPwmAbstraction>, 2> gptPwms;
        gptPwms.at(0) = std::make_unique<GptmPwmModule>();
        gptPwms.at(0)->peripheralNumber() = PeripheralNumber::Zero;
        gptPwms.at(1) = std::make_unique<GptmPwmModule>();
        gptPwms.at(1)->peripheralNumber() = PeripheralNumber::One;

        hBridge->setPwms(gptPwms);

        sm10001 = std::make_unique<Sm10001>(hBridge, adc, PinNumber(SLIDE_POT_PIN_A), PinNumber(SLIDE_POT_PIN_B));
        assert(ErrorType::Success == sm10001->init());
        return ErrorType::Success;
    }
    else if (pwmType == Sm10001SlidePotentiometerTypes::PwmType::Standalone) {
        ErrorType error = ErrorType::Failure;

        std::unique_ptr<HBridgeAbstraction> hBridge = std::make_unique<HBridge>();
        assert(nullptr != hBridge.get());
        std::unique_ptr<AdcAbstraction> adc = std::make_unique<Adc>();
        assert(nullptr != adc.get());
        adc->peripheralNumber() = PeripheralNumber::Zero;
        adc->channel() = AdcTypes::Channel::Four;
        if (ErrorType::Success != (error = adc->init())) {
            if (ErrorType::NotImplemented == error) {
                PLT_LOGI(TAG, "Adc not implemented for this platform.");
            }
        }
        std::array<std::unique_ptr<PwmAbstraction>, 2> pwms;
        pwms.at(0) = std::make_unique<Pwm>();
        pwms.at(0)->peripheralNumber() = PeripheralNumber::One;
        pwms.at(1) = std::make_unique<Pwm>();
        pwms.at(1)->peripheralNumber() = PeripheralNumber::One;

        hBridge->setPwms(pwms);

        sm10001 = std::make_unique<Sm10001>(hBridge, adc, PinNumber(SLIDE_POT_PIN_A), PinNumber(SLIDE_POT_PIN_B));
        assert(ErrorType::Success == sm10001->init());
        return ErrorType::Success;
    }
    else {
        return ErrorType::InvalidParameter;
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
