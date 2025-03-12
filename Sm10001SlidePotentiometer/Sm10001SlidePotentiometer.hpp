/***************************************************************************//**
* @author   Ben Haubrich
* @file     Sm10001.hpp
* @details  \b Synopsis: \n Application code for SM10001
*******************************************************************************/
#ifndef __SM10001_SLIDE_POTENTIOMETER_HPP__
#define __SM10001_SLIDE_POTENTIOMETER_HPP__

//AbstractionLayer
#include "Global.hpp"
#include "Error.hpp"
#include "Sm10001.hpp"

namespace Sm10001SlidePotentiometerTypes {
    enum class PwmType : uint8_t {
        Unknown = 0,
        Gptm,
        Standalone
    };
}

class Sm10001SlidePotentiometer : public Global<Sm10001SlidePotentiometer> {

    public:
    static constexpr char TAG[] = "Sm10001SlidePotentiometer";

    ErrorType slidePotentiometerThread();

    private:
    ErrorType initSlidePot(Sm10001SlidePotentiometerTypes::PwmType pwmType, std::unique_ptr<Sm10001> &sm10001);
};

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Creates the slide potentiometer code.
*/
void *startSlidePotentiometerThread(void *arg);
#ifdef __cplusplus
}
#endif
#endif // __SM10001_SLIDE_POTENTIOMETER_HPP__