set(PRJ_SOURCES
    src/main.cpp
)

set(PRJ_LIBS
    fmt
    cxxopts
)

add_executable_target(client
    SOURCES ${PRJ_SOURCES}
    LIBS ${PRJ_LIBS}
)