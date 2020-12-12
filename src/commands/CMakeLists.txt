set(PRJ_SOURCES
    src/FileReader.hpp
    src/FileReader.cpp
    include/commands/Interpreter.hpp
    src/Interpreter.cpp
    src/Prompt.hpp
    src/Prompt.cpp
    include/commands/source.hpp
)

set(PRJ_PRIVATE_LIBS
    fmt
)

add_lib_target(
    commands
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
)