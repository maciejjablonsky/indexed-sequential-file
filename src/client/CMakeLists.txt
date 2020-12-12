set(PRJ_SOURCES
    src/main.cpp
)

set(PRJ_LIBS
    fmt
    cxxopts
    database
    commands
)

add_executable_target(
    indexed-sequential-file-client
    SOURCES ${PRJ_SOURCES}
    LIBS ${PRJ_LIBS}
)
