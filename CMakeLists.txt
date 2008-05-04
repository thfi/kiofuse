project (kiofuse)

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} )

set(kiofuse_SRCS
        main.cpp
        kiofuseops.cpp
        kiofuseapp.cpp
        basejobhelper.cpp
        jobhelpers.cpp
        cache.cpp
        fusethread.cpp)

kde4_add_executable(kiofuse ${kiofuse_SRCS})
target_link_libraries(kiofuse ${KDE4_KIO_LIBS})
add_definitions(-g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE)
set (CMAKE_EXE_LINKER_FLAGS -lfuse)