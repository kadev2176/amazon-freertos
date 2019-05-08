# -------------------------------------------------------------------------------------------------
# Utilities
# -------------------------------------------------------------------------------------------------
# Set AFR source root.
get_filename_component(__root_dir "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(AFR_ROOT_DIR ${__root_dir} CACHE INTERNAL "Amazon FreeRTOS source root.")

# Get all supported vendors.
function(afr_get_vendors arg_vendors)
    set(vendors_dir "${AFR_ROOT_DIR}/vendors")
    file(GLOB __vendors RELATIVE "${vendors_dir}" "${vendors_dir}/*")
    foreach(__vendor IN LISTS __vendors)
        if(IS_DIRECTORY "${vendors_dir}/${__vendor}")
            list(APPEND ${arg_vendors} "${__vendor}")
        endif()
    endforeach()

    set(${arg_vendors} "${${arg_vendors}}" PARENT_SCOPE)
endfunction()

# Get all supported boards from a vendor.
function(afr_get_vendor_boards arg_vendor arg_boards)
    set(vendors_dir "${AFR_ROOT_DIR}/vendors")
    include("${vendors_dir}/${arg_vendor}/manifest.cmake")

    set(${arg_boards} "${AFR_MANIFEST_SUPPORTED_BOARDS}" PARENT_SCOPE)
endfunction()

# Get all supported boards.
function(afr_get_boards arg_boards)
    afr_get_vendors(afr_vendors)
    foreach(__vendor IN LISTS afr_vendors)
        afr_get_vendor_boards(${__vendor} vendor_boards)
        foreach(__board IN LISTS vendor_boards)
            list(APPEND ${arg_boards} "${__vendor}.${__board}")
        endforeach()
    endforeach()

    set(${arg_boards} "${${arg_boards}}" PARENT_SCOPE)
endfunction()

function(afr_cache_append arg_cache_var)
    if(AFR_DEBUG_CMAKE)
        # Verify ${arg_cache_var} is indeed a cache entry.
        get_property(type CACHE ${arg_cache_var} PROPERTY TYPE)
        if(NOT type)
            message(FATAL_ERROR "${arg_cache_var} is not a cache entry.")
        endif()

        # Check duplicate.
        foreach(item IN ITEMS ${ARGN})
            if(${item} IN_LIST ${arg_cache_var})
                message(WARNING "Item ${item} already in list ${arg_cache_var}.")
            endif()
        endforeach()
    endif()

    set(result "${${arg_cache_var}}")
    list(APPEND result ${ARGN})
    get_property(docstring CACHE ${arg_cache_var} PROPERTY HELPSTRING)
    set(${arg_cache_var} "${result}" CACHE INTERNAL "${docstring}")
endfunction()

function(afr_cache_remove arg_cache_var)
    if(AFR_DEBUG_CMAKE)
        # Verify ${arg_cache_var} is indeed a cache entry.
        get_property(type CACHE ${arg_cache_var} PROPERTY TYPE)
        if(NOT type)
            message(FATAL_ERROR "${arg_cache_var} is not a cache entry.")
        endif()
    endif()

    if(ARGN)
        set(result "${${arg_cache_var}}")
        list(REMOVE_ITEM result ${ARGN})
        get_property(docstring CACHE ${arg_cache_var} PROPERTY HELPSTRING)
        set(${arg_cache_var} "${result}" CACHE INTERNAL "${docstring}")
    endif()
endfunction()

function(afr_cache_remove_duplicates arg_cache_var)
    set(result "${${arg_cache_var}}")
    list(REMOVE_DUPLICATES result)
    get_property(docstring CACHE ${arg_cache_var} PROPERTY HELPSTRING)
    set(${arg_cache_var} "${result}" CACHE INTERNAL "${docstring}")
endfunction()

# Gather files under a folder.
function(afr_glob_files arg_files)
    cmake_parse_arguments(
        PARSE_ARGV 1
        "ARG"               # Prefix results with "ARG_".
        "RECURSE"           # Whether to traverse all subdirectories.
        "DIRECTORY;GLOBS"   # Specify folder location and semicolon separated glob expressions.
        ""                  # No multi-value arguments.
    )

    if(NOT DEFINED ARG_DIRECTORY)
        message(FATAL_ERROR "Missing DIRECTORY arguments.")
    endif()

    if(ARG_RECURSE)
        set(__glob_mode GLOB_RECURSE)
    else()
        set(__glob_mode GLOB)
    endif()

    if(NOT DEFINED ARG_GLOBS)
        set(__glob_list "*;.*")
    else()
        set(__glob_list "${ARG_GLOBS}")
    endif()

    set(__file_list "")
    foreach(__glob_exp IN LISTS __glob_list)
        file(
            ${__glob_mode} glob_result
            LIST_DIRECTORIES false
            CONFIGURE_DEPENDS
            "${ARG_DIRECTORY}/${__glob_exp}"
        )
        list(APPEND __file_list ${glob_result})
    endforeach()
    list(REMOVE_DUPLICATES __file_list)

    # Set output variable.
    set(${arg_files} "${__file_list}" PARENT_SCOPE)
endfunction()

# Gather source files under a folder.
function(afr_glob_src arg_files)
    cmake_parse_arguments(
        PARSE_ARGV 1
        "ARG"                   # Prefix results with "ARG_".
        "RECURSE"               # Whether to traverse all subdirectories.
        "DIRECTORY;EXTENSIONS"  # Specify folder location and semicolon separated extensions.
        ""                      # No multi-value arguments.
    )

    # Default to C and assembly files if no extension is specified.
    if("${ARG_EXTENSIONS}" STREQUAL "")
        set(__extensions "c;C;h;H;s;S;asm;ASM")
    else()
        set(__extensions "${ARG_EXTENSIONS}")
    endif()

    set(__glob_list)
    foreach(__ext IN LISTS __extensions)
        list(APPEND __glob_list "*.${__ext}")
    endforeach()

    if(ARG_RECURSE)
        afr_glob_files(${arg_files} RECURSE DIRECTORY ${ARG_DIRECTORY} GLOBS "${__glob_list}")
    else()
        afr_glob_files(${arg_files} DIRECTORY ${ARG_DIRECTORY} GLOBS "${__glob_list}")
    endif()

    # Set output variable.
    set(${arg_files} "${${arg_files}}" PARENT_SCOPE)
endfunction()

# Pretty print status information
function(afr_status arg_msg)
    if(${ARGC} EQUAL 1)
        message("${arg_msg}")
        return()
    endif()

    if(NOT ${ARGC} EQUAL 2)
        message(FATAL_ERROR "Expect at most 2 arguments")
    endif()

    set(status_name "${arg_msg}")
    set(status_val "${ARGV1}")
    list(SORT status_val)
    list(JOIN status_val ", " status_val)

    # Calculate indent.
    string(LENGTH "${status_name}" indent_len)
    math(EXPR indent_len "${indent_len} - 1")
    set(indent "")
    foreach(i RANGE ${indent_len})
        string(APPEND indent " ")
    endforeach()

    # Format output.
    set(str "${status_name}${status_val}")
    string(LENGTH "${str}" len)
    while(len GREATER 90)
        string(SUBSTRING "${str}" 0 90 split)
        string(FIND "${split}" " " idx REVERSE)
        math(EXPR idx "${idx} + 1")

        string(SUBSTRING "${str}" ${idx} -1 next_str)
        string(SUBSTRING "${str}" 0 ${idx} str)
        message("${str}")
        string(PREPEND next_str "${indent}")
        string(LENGTH "${next_str}" len)
        set(str "${next_str}")
    endwhile()
    message("${str}")
endfunction()

function(afr_get_target_dependencies arg_target arg_dependencies)
    if(NOT TARGET ${arg_target})
        message(FATAL_ERROR "${arg_target} is not a target.")
    endif()

    get_target_property(dependencies ${arg_target} INTERFACE_LINK_LIBRARIES)
    get_target_property(type ${arg_target} TYPE)
    if(NOT "${type}" STREQUAL "INTERFACE_LIBRARY")
        get_target_property(more_dependencies ${arg_target} LINK_LIBRARIES)
        list(APPEND dependencies ${more_dependencies})
    endif()

    set(${arg_dependencies} ${dependencies} PARENT_SCOPE)
endfunction()

# Create a target to print the generator expression.
function(afr_print_generator_expr target_name expression)
    add_custom_target(${target_name}
        ${CMAKE_COMMAND} -E echo "\"${expression}\""
    )
endfunction()
