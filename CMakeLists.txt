project (kiofuse)

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} )
 
set(kiofuse_SRCS
        main.cpp
        kiofuseops.cpp
        kiofuseapp.cpp
        basejobhelper.cpp
        jobhelpers.cpp)

kde4_add_executable(kiofuse ${kiofuse_SRCS})
target_link_libraries(kiofuse ${KDE4_KIO_LIBS})
set (CMAKE_CXX_FLAGS " -D_FILE_OFFSET_BITS=64 -lfuse")
