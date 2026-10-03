INCLUDE(FindOpenSSL)
INCLUDE(FindZLIB)

IF(NOT OPENSSL_FOUND)
	message(FATAL_ERROR "openssl is required for dmg!")
ENDIF(NOT OPENSSL_FOUND)

IF(NOT ZLIB_FOUND)
	message(FATAL_ERROR "zlib is required for dmg!")
ENDIF(NOT ZLIB_FOUND)

include_directories(${ZLIB_INCLUDE_DIR})

link_directories(${PROJECT_BINARY_DIR}/common ${PROJECT_BINARY_DIR}/hfs)

add_library(dmg checksum.c dmgfile.c dmglib.c filevault.c io.c partition.c resources.c udif.c)

IF(OPENSSL_FOUND)
	add_definitions(-DHAVE_CRYPT)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(dmg ${OPENSSL_LIBRARIES})
	IF(WIN32)
		# gdi32 for the image code; ws2_32/crypt32/bcrypt/userenv are
		# required by the statically linked OpenSSL.
		TARGET_LINK_LIBRARIES(dmg gdi32 ws2_32 crypt32 bcrypt userenv)
	ENDIF(WIN32)
ENDIF(OPENSSL_FOUND)

target_link_libraries(dmg common hfs ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
