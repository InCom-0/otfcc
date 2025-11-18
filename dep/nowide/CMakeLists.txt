add_library(nowide-lib STATIC)

set(SRC_nowide-lib src/console_buffer.cpp src/console_buffer.hpp src/cstdio.cpp src/cstdlib.cpp src/filebuf.cpp src/iostream.cpp src/stat.cpp)
set_source_files_properties(
    ${SRC_nowide-lib}
    PROPERTIES LANGUAGE CXX
)
target_sources(nowide-lib PRIVATE ${SRC_nowide-lib})

target_sources(nowide-lib
    PUBLIC
    FILE_SET pub_headers
    TYPE HEADERS
    BASE_DIRS
    include
)
