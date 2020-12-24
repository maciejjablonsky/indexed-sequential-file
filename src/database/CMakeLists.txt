set(PRJ_SOURCES
    include/database/DBMS.hpp
    include/database/Record.hpp
    include/database/Key.hpp
    src/Database.hpp
    src/DBMS.cpp
    src/DBMS-internal.hpp
    src/DBMS-internal.cpp
    src/DBMS.cpp
    src/Index.hpp
    src/Page.hpp
    src/DiskAccess.hpp
    src/PageDispositor.hpp
    src/Memory.hpp
)

set(PRJ_PRIVATE_LIBS
    commands
    fmt
    nlohmann_json::nlohmann_json
    my_concepts
)

set(PRJ_PUBLIC_LIBS
    overloaded
)

add_lib_target(
    database
    SOURCES ${PRJ_SOURCES}
    PRIVATE_LIBS ${PRJ_PRIVATE_LIBS}
    PUBLIC_LIBS ${PRJ_PUBLIC_LIBS}
)