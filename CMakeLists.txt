cmake_minimum_required(VERSION 3.21.3)

#Use `cmake --help-policy <POLICY>` to learn more
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0067 NEW)

include(custom_functions.cmake)

#python3 apf.py clean build --toolchain ccsToCMake/toolchain.cmake --target CC32xx
if (Cc32xx)
  message(STATUS "Building for Cc32xx")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  #Order matters!
  include(${CMAKE_SOURCE_DIR}/AbstractionLayer/cc32xx.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/ccsToCMake/cc32xx.cmake)

  add_custom_command(TARGET ${PROJECT_NAME}${EXECUTABLE_SUFFIX} POST_BUILD
  COMMAND
    ${CMAKE_OBJCOPY} ARGS -O binary ${CMAKE_SOURCE_DIR}/customToolchain_build/${PROJECT_NAME}${EXECUTABLE_SUFFIX} ${CMAKE_SOURCE_DIR}/customToolchain_build/${PROJECT_NAME}.bin
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}
    COMMENT
      "Stripping executable"
    VERBATIM
  )

  #There is only one partition. So the type could be anything.
  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    MAIN_PARTITION_TYPE=FileSystemTypes::Implementation::Spiffs
  )

#python3 apf.py clean build --toolchain ccsToCMake/toolchain.cmake --target Raspbian12Pi4ModelB
elseif(Raspbian12Pi4ModelB)
  message(STATUS "Building for Raspbian 12 on Pi4 Model B")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  set(CMAKE_C_FLAGS "-std=gnu2x ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-std=gnu++23 ${CMAKE_CXX_FLAGS}")

  include(${CMAKE_SOURCE_DIR}/AbstractionLayer/raspbian12Pi4ModelB.cmake)

  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    #We are actually referring to the GPIO number, not the Pin number on the 40 pin header.
    SLIDE_POT_PIN_A=19
    SLIDE_POT_PIN_B=13
    PWM_TYPE=Sm10001SlidePotentiometerTypes::PwmType::Gptm
  )

#idf.py build
elseif (ESP_PLATFORM)
  message(STATUS "Building for ESP")

  set(CMAKE_C_FLAGS "-std=gnu2x ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-std=gnu++23 ${CMAKE_CXX_FLAGS}")

  #If not set, defaults to all components. Set this to reduce the amount of
  #components that are built at compile time. Required and linked components
  #for the ESP main component are located in the "main component" CMakeLists.txt file.
  set(COMPONENTS
    main
  )
  #Internal IDF cmake files set the target to end in .elf so we don't have much of a choice here.
  set(EXECUTABLE_SUFFIX ".elf")
  #include directive must come after the set(COMPONENTS*) directive
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  #ESP projects require the project line to be declared after the include
  project(SlidePotentiometer)
  include(AbstractionLayer/esp.cmake)

  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    #We are actually referring to the GPIO number, not the Pin number on the 40 pin header.
    SLIDE_POT_PIN_A=0
    SLIDE_POT_PIN_B=2
    PWM_TYPE=Sm10001SlidePotentiometerTypes::PwmType::Gptm
  )

#Even if cross-compiling, your host system name will still be present so cross compiled targets should be evaluated first.
#python3 apf.py build
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "Building for Linux")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  set(CMAKE_C_FLAGS "-std=gnu2x ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-std=gnu++23 ${CMAKE_CXX_FLAGS}")
  include(AbstractionLayer/linux.cmake)

else()
  message(FATAL_ERROR "Invalid or unsupported target specified")
endif()

if (${RELEASE_BUILD})
  message(STATUS "Building release build")
  target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    -O3
)

#Default to debug build
else ()
  message(STATUS "Building debug build")
  target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PUBLIC
    -O0
    -g
    -gdwarf-3
    -gstrict-dwarf
)
endif()

#Options for all builds.
target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PUBLIC
  -ffunction-sections
  -fdata-sections
  -fno-exceptions
  -Wall
)

add_subdirectory(Sm10001SlidePotentiometer)
