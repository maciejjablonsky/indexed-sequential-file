set(PRJ_SOURCES
    include/commands/FileReader.hpp
    src/FileReader.cpp
    include/commands/Interpreter.hpp
    src/Interpreter.cpp
    include/commands/Prompt.hpp
    src/Prompt.cpp
    include/commands/source.hpp
    include/commands/commands.hpp
)

set(PRJ_PRIVATE_LIBS
    fmt
    database
)

set(PRJ_PUBLIC_LIBS
    overloaded
)


add_lib_target(
    commands
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
    PUBLIC_LIBS ${PRJ_PUBLIC_LIBS}
)