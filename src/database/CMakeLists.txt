set(PRJ_SOURCES
    include/database/Database.hpp
    src/Database.cpp
    include/database/DBMS.hpp
    src/DBMS.cpp
    src/Entry.hpp
    src/Entry.cpp
    include/database/Link.hpp
    include/database/Record.hpp
    include/database/Key.hpp
    src/Index.hpp
    src/Index.cpp
)

set(PRJ_PRIVATE_LIBS
    commands
    fmt
)

add_lib_target(
    database
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
)