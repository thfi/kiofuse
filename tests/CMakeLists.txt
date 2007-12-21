project (testFileJob)
 
find_package(KDE4 REQUIRED)
    include_directories( ${KDE4_INCLUDES} )
 
set(testFileJob_SRCS main.cpp)
 
kde4_add_executable(testFileJob ${testFileJob_SRCS})
target_link_libraries(testFileJob ${KDE4_KIO_LIBS})
