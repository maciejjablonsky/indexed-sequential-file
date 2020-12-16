add_subdirectory(fmt)
set_target_properties(fmt PROPERTIES FOLDER 3rdParty/fmt)

add_subdirectory(cxxopts)
set_target_properties(options_test PROPERTIES FOLDER 3rdParty/cxxopts)
set_target_properties(link_test PROPERTIES FOLDER 3rdParty/cxxopts)
set_target_properties(example PROPERTIES FOLDER 3rdParty/cxxopts)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)
