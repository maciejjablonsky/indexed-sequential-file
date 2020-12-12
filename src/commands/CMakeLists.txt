set(PRJ_SOURCES
    include/commands/FileReader.hpp
    src/FileReader.cpp
    include/commands/Interpreter.hpp
    src/Interpreter.cpp
    include/commands/Prompt.hpp
    src/Prompt.cpp
)

set(PRJ_PRIVATE_LIBS
    fmt
)

add_lib_target(
    commands
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
)