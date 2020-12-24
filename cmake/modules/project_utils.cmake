function(add_lib_target PRJ_NAME)
    set(PRIVATE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(NOT EXISTS ${PRIVATE_SRC_DIR})
        message(FATAL_ERROR "Main source directory for project '${PRJ_NAME}' does not exist! Path: ${PRIVATE_SRC_DIR}")
    endif()

    set(SHARED_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(NOT EXISTS ${SHARED_HEADERS_DIR})
        message(FATAL_ERROR "Shared sources directory for project '${PRJ_NAME}' does not exist! Path: ${SHARED_HEADERS_DIR}")
    endif()

    cmake_parse_arguments(
        PARSED_ARGS # prefix of output variables
        "" # list of names of the boolean arguments (only defined ones will be true)
        "IDE_FOLDER" # list of names of mono-valued arguments
        "SOURCES;PRIVATE_LIBS;PUBLIC_LIBS" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "No sources for library project '${PRJ_NAME}' were provided!")
    endif()

    message(STATUS "Adding library '${PRJ_NAME}'")
    message(STATUS "Sources: ${PARSED_ARGS_SOURCES}")

    add_library(${PRJ_NAME} STATIC ${PARSED_ARGS_SOURCES})

    target_include_directories(${PRJ_NAME} PUBLIC ${SHARED_HEADERS_DIR})
    target_include_directories(${PRJ_NAME} PRIVATE ${PRIVATE_SRC_DIR})

    if(PARSED_ARGS_PRIVATE_LIBS)
        message(STATUS "Private libraries: ${PARSED_ARGS_PRIVATE_LIBS}")
        target_link_libraries(${PRJ_NAME} PRIVATE ${PARSED_ARGS_PRIVATE_LIBS})
    endif()

    if(PARSED_ARGS_PUBLIC_LIBS)
        message(STATUS "Public libraries: ${PARSED_ARGS_PUBLIC_LIBS}")
        target_link_libraries(${PRJ_NAME} PUBLIC ${PARSED_ARGS_PUBLIC_LIBS})
    endif()

    if(PARSED_ARGS_IDE_FOLDER)
        set_target_properties(${PRJ_NAME} PROPERTIES FOLDER ${PARSED_ARGS_IDE_FOLDER})
    endif()
endfunction()


function(add_header_only_lib_target PRJ_NAME)
    set(SHARED_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if (NOT EXISTS ${SHARED_HEADERS_DIR})
        message(FATAL_ERROR  "Shared headers directory for project '${PRJ_NAME}' does not exist! Path: ${SHARED_HEADERS_DIR}")
    endif()

    cmake_parse_arguments(
        PARSED_ARGS
        ""
        "IDE_FOLDER"
        "HEADERS;PUBLIC_LIBS"
        ${ARGN}
    )

    foreach (var IN ITEMS ${PARSED_ARGS_HEADERS})
        # ../any/combination/of/../header/path.hpp
        if (NOT "${var}" MATCHES "[A-Za-z/\\\.]*.\.(h|hpp)$")
            message(FATAL_ERROR "Header-only library must contain only header files. Unexpected file: ${var}")
        endif()
    endforeach()

    if(NOT PARSED_ARGS_HEADERS)
        message(FATAL_ERROR "No header files for library project '${PRJ_NAME}' were provided!")
    endif()

    message(STATUS "Adding header-only library '${PRJ_NAME}'")
    message(STATUS "Headers: ${PARSED_ARGS_HEADERS}")

    add_library(${PRJ_NAME} OBJECT ${PARSED_ARGS_HEADERS})
    set_target_properties(${PRJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(${PRJ_NAME}
        PUBLIC ${SHARED_HEADERS_DIR}
    )

    if(PARSED_ARGS_PUBLIC_LIBS)
        message(STATUS "Public libraries: ${PARSED_ARGS_PUBLIC_LIBS}")
        target_link_libraries(${PRJ_NAME} PUBLIC ${PARSED_ARGS_PUBLIC_LIBS})
    endif()

    if(PARSED_ARGS_IDE_FOLDER)
        set_target_properties(${PRJ_NAME} PROPERTIES FOLDER ${PARSED_ARGS_IDE_FOLDER})
    endif()
endfunction()

function(add_executable_target PRJ_NAME)
    set(PRIVATE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(NOT EXISTS ${PRIVATE_SRC_DIR})
        message(FATAL_ERROR "Main source directory for project '${PRJ_NAME}' does not exist! Path: ${PRIVATE_SRC_DIR}")
    endif()

    cmake_parse_arguments(
        PARSED_ARGS # prefix of output variables
        "" # list of names of the boolean arguments (only defined ones will be true)
        "IDE_FOLDER" # list of names of mono-valued arguments
        "SOURCES;LIBS" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "No sources for executable project '${PRJ_NAME}' were provided!")
    endif()

    message(STATUS "Adding executable '${PRJ_NAME}'")
    message(STATUS "Sources: ${PARSED_ARGS_SOURCES}")

    add_executable(${PRJ_NAME} ${PARSED_ARGS_SOURCES})

    target_include_directories(${PRJ_NAME} PRIVATE ${PRIVATE_SRC_DIR})

    if(PARSED_ARGS_LIBS)
        message(STATUS "Libraries: ${PARSED_ARGS_LIBS}")
        target_link_libraries(${PRJ_NAME} PRIVATE ${PARSED_ARGS_LIBS})
    endif()

    if(PARSED_ARGS_IDE_FOLDER)
        set_target_properties(${PRJ_NAME} PROPERTIES FOLDER ${PARSED_ARGS_IDE_FOLDER})
    endif()
endfunction()