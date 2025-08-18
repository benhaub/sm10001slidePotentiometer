cmake_minimum_required(VERSION 3.21.3)

#Use `cmake --help-policy <POLICY>` to learn more
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0067 NEW)

include(custom_functions.cmake)

set(CMAKE_CXX_STANDARD 23)

#python3 apf.py clean build --toolchain ccsToCMake/toolchain.cmake --target CC32xx
if (Cc32xx)
  message(STATUS "Building for Cc32xx")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  #Order matters!
  include(${CMAKE_SOURCE_DIR}/AbstractionLayer/cc32xx.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/ccsToCMake/Cc32xx/target.cmake)

  add_custom_command(TARGET ${PROJECT_NAME}${EXECUTABLE_SUFFIX} POST_BUILD
  COMMAND
    ${CMAKE_OBJCOPY} ARGS -O binary ${CMAKE_SOURCE_DIR}/Cc32xx_build/${PROJECT_NAME}${EXECUTABLE_SUFFIX} ${CMAKE_SOURCE_DIR}/Cc32xx_build/${PROJECT_NAME}.bin
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
    APP_SLIDE_POT_PIN_A=-1
    APP_SLIDE_POT_PIN_B=-1
    APP_PWM_TYPE=HBridgeTypes::PwmType::Gptm
    APP_CLOCK_FREQUENCY=0
    APP_EXTERNAL_CRYSTAL_FREQUENCY=0
    APP_DEFAULT_STACK_SIZE=2*1024
    APP_HBRIDGE_PART_NUMBER=HBridgeFactoryTypes::PartNumber::TiDrv8872
    APP_ADC_PERIPHERAL_NUMBER=PeripheralNumber::Zero
    APP_MAX_POTENTIOMETER_VOLTAGE_DROP=3.3f
    APP_MIN_POTENTIOMETER_VOLTAGE_DROP=0.0f
  )

#python3 apf.py clean build --toolchain ccsToCMake/toolchain.cmake --target Tm4c123
elseif (Tm4c123)
  message(STATUS "Building for Tm4c123")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  #Order matters!
  include(${CMAKE_CURRENT_LIST_DIR}/ccsToCMake/Tm4c123/target.cmake)
  include(${CMAKE_SOURCE_DIR}/AbstractionLayer/tm4c123.cmake)

  add_custom_command(TARGET ${PROJECT_NAME}${EXECUTABLE_SUFFIX} POST_BUILD
  COMMAND
  ${CMAKE_OBJCOPY} ARGS -O binary ${CMAKE_SOURCE_DIR}/Tm4c123_build/${PROJECT_NAME}${EXECUTABLE_SUFFIX} ${CMAKE_SOURCE_DIR}/Tm4c123_build/${PROJECT_NAME}.bin
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}
    COMMENT
      "Stripping executable"
    VERBATIM
  )

  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    MAIN_PARTITION_TYPE=FileSystemTypes::Implementation::Unknown
    APP_SLIDE_POT_PIN_A=0xD0
    APP_SLIDE_POT_PIN_B=0xD1
    APP_PWM_TYPE=HBridgeTypes::PwmType::Standalone
    APP_CLOCK_FREQUENCY=80E6
    APP_EXTERNAL_CRYSTAL_FREQUENCY=16E6
    APP_DEFAULT_STACK_SIZE=1024
    APP_HBRIDGE_PART_NUMBER=HBridgeFactoryTypes::PartNumber::TiDrv8872
    APP_ADC_PERIPHERAL_NUMBER=PeripheralNumber::Zero
    APP_MAX_POTENTIOMETER_VOLTAGE_DROP=3.3f
    APP_MIN_POTENTIOMETER_VOLTAGE_DROP=0.0f

  )

#python3 apf.py clean build --toolchain ccsToCMake/toolchain.cmake --target Raspbian12Pi4ModelB
elseif(Raspbian12Pi4ModelB)
  message(STATUS "Building for Raspbian 12 on Pi4 Model B")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  include(${CMAKE_SOURCE_DIR}/AbstractionLayer/raspbian12Pi4ModelB.cmake)

  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    #We are actually referring to the GPIO number, not the Pin number on the 40 pin header.
    APP_SLIDE_POT_PIN_A=19
    APP_SLIDE_POT_PIN_B=13
    APP_PWM_TYPE=HBridgeTypes::PwmType::Gptm
    APP_CLOCK_FREQUENCY=0
    APP_EXTERNAL_CRYSTAL_FREQUENCY=0
    APP_DEFAULT_STACK_SIZE=2*1024
    APP_HBRIDGE_PART_NUMBER=HBridgeFactoryTypes::PartNumber::TiDrv8872
    APP_ADC_PERIPHERAL_NUMBER=PeripheralNumber::Zero
    APP_MAX_POTENTIOMETER_VOLTAGE_DROP=3.3f
    APP_MIN_POTENTIOMETER_VOLTAGE_DROP=0.0f
  )

#idf.py build
elseif (ESP_PLATFORM)
  message(STATUS "Building for ESP")

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
    APP_SLIDE_POT_PIN_A=0
    APP_SLIDE_POT_PIN_B=2
    APP_PWM_TYPE=HBridgeTypes::PwmType::Gptm
    APP_CLOCK_FREQUENCY=0
    APP_EXTERNAL_CRYSTAL_FREQUENCY=0
    APP_DEFAULT_STACK_SIZE=16*1024
    APP_HBRIDGE_PART_NUMBER=HBridgeFactoryTypes::PartNumber::TiDrv8872
    APP_ADC_PERIPHERAL_NUMBER=PeripheralNumber::Zero
    APP_MAX_POTENTIOMETER_VOLTAGE_DROP=3.3f
    APP_MIN_POTENTIOMETER_VOLTAGE_DROP=0.0f
  )

#Even if cross-compiling, your host system name will still be present so cross compiled targets should be evaluated first.
#python3 apf.py build
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "Building for Linux")

  project(Sm10001SlidePotentiometer LANGUAGES C CXX)

  set(EXECUTABLE_SUFFIX ".elf")
  add_executable(${PROJECT_NAME}${EXECUTABLE_SUFFIX} main.cpp)

  include(AbstractionLayer/linux.cmake)

  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    #We are actually referring to the GPIO number, not the Pin number on the 40 pin header.
    APP_SLIDE_POT_PIN_A=-1
    APP_SLIDE_POT_PIN_B=-1
    APP_PWM_TYPE=HBridgeTypes::PwmType::Gptm
    APP_CLOCK_FREQUENCY=0
    APP_EXTERNAL_CRYSTAL_FREQUENCY=0
    APP_DEFAULT_STACK_SIZE=2*1024
    APP_HBRIDGE_PART_NUMBER=HBridgeFactoryTypes::PartNumber::TiDrv8872
    APP_ADC_PERIPHERAL_NUMBER=PeripheralNumber::Zero
    APP_MAX_POTENTIOMETER_VOLTAGE_DROP=3.3f
    APP_MIN_POTENTIOMETER_VOLTAGE_DROP=0.0f
  )

else()
  message(FATAL_ERROR "Invalid or unsupported target specified")
endif()

#python3 app.py build --build-type Release
#or idf.py build -DRELEASE_BUILD=1
if (${RELEASE_BUILD})
  message(STATUS "Building release build")
elseif (${SANITIZE_BUILD})
  message(STATUS "Building sanitize build")
elseif (${PROFILE_BUILD})
  message(STATUS "Building profile build")
else ()
  #Default to debug build
  #or idf.py build -DRELEASE_BUILD=0
  message(STATUS "Building debug build")
endif()

target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PUBLIC
  -ffunction-sections
  -fdata-sections
  -Wall
  #https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:condition
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>

  $<$<BOOL:${RELEASE_BUILD}>:-fomit-frame-pointer>
  $<$<BOOL:${RELEASE_BUILD}>:-O2>

  $<$<BOOL:${SANITIZE_BUILD}>:-O1>
  $<$<BOOL:${SANITIZE_BUILD}>:-g>
  $<$<BOOL:${SANITIZE_BUILD}>:-fsanitize=${SANITIZE_CHECK}>
  $<$<BOOL:${SANITIZE_BUILD}>:-fno-omit-frame-pointer>

  $<$<BOOL:${PROFILE_BUILD}>:-O2>
  $<$<BOOL:${PROFILE_BUILD}>:-g>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Linux>>:-pg>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Darwin>>:-fprofile-instr-generate>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Darwin>>:-fcoverage-mapping>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Darwin>>:-fno-omit-frame-pointer>

  # Debug build flags (default as in the case of a lack of any other build type specified).
  $<$<AND:$<NOT:$<BOOL:${RELEASE_BUILD}>>,$<NOT:$<BOOL:${SANITIZE_BUILD}>>,$<NOT:$<BOOL:${PROFILE_BUILD}>>>:-O0>
  $<$<AND:$<NOT:$<BOOL:${RELEASE_BUILD}>>,$<NOT:$<BOOL:${SANITIZE_BUILD}>>,$<NOT:$<BOOL:${PROFILE_BUILD}>>>:-g>
  $<$<AND:$<NOT:$<BOOL:${RELEASE_BUILD}>>,$<NOT:$<BOOL:${SANITIZE_BUILD}>>,$<NOT:$<BOOL:${PROFILE_BUILD}>>>:-gdwarf-3>
  $<$<AND:$<NOT:$<BOOL:${RELEASE_BUILD}>>,$<NOT:$<BOOL:${SANITIZE_BUILD}>>,$<NOT:$<BOOL:${PROFILE_BUILD}>>>:-gstrict-dwarf>
)

target_link_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PUBLIC
  $<$<BOOL:${SANITIZE_BUILD}>:-fsanitize=${SANITIZE_CHECK}>
  
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Linux>>:-pg>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Darwin>>:-fprofile-instr-generate>
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Darwin>>:-fcoverage-mapping>
)

target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PUBLIC
  $<$<BOOL:${RELEASE_BUILD}>:CLOUD_CLIENT_LOGGING_ENABLED=0>
  $<$<BOOL:${RELEASE_BUILD}>:CLOUD_CLIENT_VERBOSE_LOGGING_ENABLED=0>
  $<$<BOOL:${RELEASE_BUILD}>:DEBUG_BUILD=0>
  
  $<$<AND:$<BOOL:${PROFILE_BUILD}>,$<STREQUAL:${CMAKE_HOST_SYSTEM_NAME},Linux>>:-pg>
  $<$<AND:$<NOT:$<BOOL:${RELEASE_BUILD}>>,$<NOT:$<BOOL:${SANITIZE_BUILD}>>,$<NOT:$<BOOL:${PROFILE_BUILD}>>>:DEBUG_BUILD=1>
)

#Cross-platform appName core code
add_subdirectory(Sm10001SlidePotentiometer)
