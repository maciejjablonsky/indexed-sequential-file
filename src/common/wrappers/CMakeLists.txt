set(PRJ_HEADERS
    include/wrappers/ref.hpp
    include/wrappers/opt.hpp
    include/wrappers/optref.hpp
)
add_header_only_lib_target(
    wrappers
    HEADERS ${PRJ_HEADERS}
    IDE_FOLDER common
)