set(PUBLIC_HEADERS
    include/concepts/comparable.hpp
    include/concepts/memory_access.hpp
)
add_header_only_lib_target(
    my_concepts
    HEADERS ${PUBLIC_HEADERS}
    IDE_FOLDER common/concepts
)