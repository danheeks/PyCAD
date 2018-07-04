project(geom)

cmake_minimum_required(VERSION 2.4)

SET(ENV{BOOST_ROOT} "C:\Users\Dan Heeks\Downloads\boost_1_57_0\boost_1_57_0")

# Turn compiler warnings up to 11, at least with gcc.  
if (CMAKE_BUILD_TOOL MATCHES "make")
    MESSAGE(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    # NON-optimized build:
    # add_definitions( -Wall  -Wno-deprecated -Werror -pedantic-errors)
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

option(BUILD_TYPE
  "Build type: Release=ON/Debug=OFF  " ON)

if (BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Release")
    set(CMAKE_BUILD_TYPE Release)
endif(BUILD_TYPE)

if (NOT BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Debug")
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT BUILD_TYPE)

# this figures out the Python include directories and adds them to the
# header file search path
execute_process(
    COMMAND python-config --includes
    COMMAND sed -r "s/-I//g; s/ +/;/g"
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE Python_Includes
)
message(STATUS "Python include dir:" ${Python_Includes})

include_directories(${Python_Includes})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package( Boost COMPONENTS python REQUIRED )  # find BOOST and boost-python
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "boost-python lib is: " ${Boost_PYTHON_LIBRARY})
    MESSAGE(STATUS "boost_LIBRARY_DIRS is: " ${Boost_LIBRARY_DIRS})
    MESSAGE(STATUS "Boost_LIBRARIES is: " ${Boost_LIBRARIES})    
endif()

find_package( OpenGL REQUIRED )

#message("opengl directory is")
#message(${OPENGL_INCLUDE_DIR})


include_directories(OPENGL_INCLUDE_DIR)

# this defines the source-files for library
set(GEOM_SRC
    Arc.cpp
    Area.cpp
    AreaClipper.cpp
    AreaDxf.cpp
    AreaOrderer.cpp
    AreaPocket.cpp
    Circle.cpp
    clipper.cpp
    Construction.cpp
    Curve.cpp
    Curve3D.cpp
    dxf.cpp
    Finite.cpp
    HeeksGeomDxf.cpp
    kurve.cpp
    Matrix.cpp
    Mesh.cpp
    OctTree.cpp
    offset.cpp
    PythonStuff.cpp
    Solid.cpp
    Tris.cpp
)


# this defines the headers
if(DEFINED INCLUDE_INSTALL_DIR)
    set(includedir ${INCLUDE_INSTALL_DIR})
else(DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR include)
    set(includedir ${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR})
endif(DEFINED INCLUDE_INSTALL_DIR)

# include directories

# this part allow to support multi-arch
# ie. Debian builder sets correctly the target path according to architecture
# e.g. /usr/lib/i386-linux-gnu, /usr/lib/x86_64-linux-gnu
# TODO: Support this feature
#if(DEFINED CMAKE_INSTALL_LIBDIR)
#    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
#else(DEFINED CMAKE_INSTALL_LIBDIR)
#    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
#endif(DEFINED CMAKE_INSTALL_LIBDIR)


# this makes the Python module
  add_library(
      geom
      MODULE
      ${GEOM_SRC}
  )

target_link_libraries(geom ${Boost_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
set_target_properties(geom PROPERTIES PREFIX "") 

# this figures out where to install the Python modules
execute_process(
    COMMAND python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_VARIABLE Python_site_packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# strip away /usr/local/ because that is what CMAKE_INSTALL_PREFIX is set to
# also, since there is no leading "/", it makes ${Python_site_packages} a relative path.
STRING(REGEX REPLACE "/usr/local/(.*)$" "\\1" Python_site_packages "${Python_site_packages}" )
STRING(REGEX REPLACE "/usr/(.*)$" "\\1" Python_site_packages "${Python_site_packages}" )

message(STATUS "Python module will be installed to: " ${CMAKE_INSTALL_PREFIX}/${Python_site_packages})

# this installs the python library
install(
    TARGETS geom
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${Python_site_packages}
)