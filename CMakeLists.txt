project(kiofuse)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

find_package(FUSE 2.9 REQUIRED)
include_directories(${FUSE_INCLUDE_DIRS})

set(kiofuse_SRCS
    main.cpp
    kiofuseops.cpp
    kiofuseapp.cpp
    basejobhelper.cpp
    jobhelpers.cpp
    cache.cpp
    fusethread.cpp
)

kde4_add_executable(kiofuse ${kiofuse_SRCS})

add_definitions(-g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE)

target_link_libraries(kiofuse
    ${KDE4_KIO_LIBS}
    ${FUSE_LIBRARIES}
)

install(
    TARGETS
    kiofuse
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
