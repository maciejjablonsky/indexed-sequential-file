set(PRJ_SOURCES
    src/Database.hpp
    src/Database.cpp
    include/database/DBMS.hpp
    src/DBMS.cpp
    src/DBMS-internal.hpp
    src/DBMS-internal.cpp
    src/DBMS.cpp
    src/Entry.hpp
    src/Entry.cpp
    src/Link.hpp
    src/Link.cpp
    include/database/Record.hpp
    src/Record.cpp
    include/database/Key.hpp
    src/Key.cpp
    src/Index.hpp
    src/Index.cpp
    src/Area.hpp
    src/Area.cpp
    src/EntryPage.hpp
    src/EntryPage.cpp
)

set(PRJ_PRIVATE_LIBS
    commands
    fmt
    nlohmann_json::nlohmann_json
)

add_lib_target(
    database
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
)