project( heeksintf )
cmake_minimum_required( VERSION 2.6 )

add_definitions ( -Wall )

include_directories ( ${CMAKE_SOURCE_DIR}
                      ${CMAKE_SOURCE_DIR}/src
                    )
set ( LIBRARY_OUTPUT_PATH ../bin )

set( heeksintf_SRCS
    test.cpp
   )
   
set( heeksintf_HDRS
    test.h
   )

add_library( heeksintf SHARED ${heeksintf_SRCS} )

set_target_properties( heeksintf PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heeksintf DESTINATION ${LIB_INSTALL_DIR} )

file( GLOB sources ${heeksintf_SRCS} ${heeksintf_HDRS} )
install( FILES ${sources} DESTINATION "share/cad/interface" COMPONENT heeksintf )
