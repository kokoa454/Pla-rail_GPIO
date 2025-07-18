cmake_minimum_required(VERSION 3.10)
project(PLA_RAIL_GPIO)

set(SOURCES
    main.c
    catchEcho.c
    measureDistance.c
    outputLog.c
    setGpio.c
    startSensor.c
    startTrain.c
    stopSensor.c
    stopTrain.c
)

add_executable(pla_rail ${SOURCES})
target_link_libraries(pla_rail lgpio)
