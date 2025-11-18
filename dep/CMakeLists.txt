add_library(dep-lib STATIC)

set(SRC_dep-lib json-builder/src/json-builder.c json-parser/src/json.c sds/src/sds.c sds/src/sdsalloc.h)
set_source_files_properties(
    ${SRC_dep-lib}
    PROPERTIES LANGUAGE C
)
target_sources(dep-lib PRIVATE ${SRC_dep-lib})

target_sources(dep-lib
    PUBLIC
    FILE_SET pub_headers
    TYPE HEADERS
    BASE_DIRS
    json-builder/include
    json-parser/include
    sds/include
    uthash/include
    intl/include
)

add_subdirectory(nowide)
target_link_libraries(dep-lib PUBLIC nowide-lib)
