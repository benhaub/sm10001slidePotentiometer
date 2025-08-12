/***************************************************************************//**
* @author   Ben Haubrich
* @file     Sm10001.hpp
* @details  \b Synopsis: \n Application code for SM10001
*******************************************************************************/
#ifndef __SM10001_SLIDE_POTENTIOMETER_HPP__
#define __SM10001_SLIDE_POTENTIOMETER_HPP__

//AbstractionLayer
#include "Global.hpp"
#include "Sm10001.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <optional>

class Sm10001SlidePotentiometer : public Global<Sm10001SlidePotentiometer> {

    public:
    static constexpr char TAG[] = "Sm10001SlidePotentiometer";

    static constexpr std::array<char, OperatingSystemTypes::MaxThreadNameLength> slidePotentiometerThreadName = {"slidePot"};

    ErrorType slidePotentiometerThread();
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