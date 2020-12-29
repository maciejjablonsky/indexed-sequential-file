set(PUBLIC_HEADERS
    include/overloaded/overloaded.hpp
)
add_header_only_lib_target(
    overloaded
    HEADERS ${PUBLIC_HEADERS}
    IDE_FOLDER common
)