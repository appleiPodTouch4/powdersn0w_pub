INCLUDE(FindZLIB)

IF(NOT ZLIB_FOUND)
	message(FATAL_ERROR "zlib is required for libminizip!")
ENDIF(NOT ZLIB_FOUND)

add_definitions( -DNOCRYPT -DNOUNCRYPT )

include_directories(${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARIES})

add_library(minizip ioapi.c unzip.c zip.c)

# Use the located zlib explicitly instead of a bare "-lz": on MinGW, -lz
# prefers libz.dll.a (import lib) over libz.a, which collides with the static
# libz.a found by FindZLIB and produces "multiple definition of `crc32'".
target_link_libraries(minizip ${ZLIB_LIBRARIES})
