set(PRJ_SOURCES
    include/database/Database.hpp
    src/Database.cpp
    include/database/Dbms.hpp
    src/Dbms.cpp
)

set(PRJ_PRIVATE_LIBS
    commands
)

add_lib_target(
    database
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
)