project(geom)

cmake_minimum_required(VERSION 2.8.3)

# Set Python version
set( APP_PYTHON_VERSION 3.7 )

#
# Find Python libraries, interpreter and site-packages directory
#

# Find Python libs
find_package( PythonLibs ${APP_PYTHON_VERSION} REQUIRED )
if( PythonLibs_FOUND )
  include_directories( ${PYTHON_INCLUDE_DIRS} )
else()
  message( FATAL_ERROR "Python libraries not found" )
endif()

# Find Python interpreter
find_package( PythonInterp ${PYTHONLIBS_VERSION_STRING} REQUIRED )

# Find Python's site-packages directory and set PYTHON_SITE_PACKAGES variable
if( PYTHONINTERP_FOUND )
  execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
      OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  #message(STATUS "site-packages directory: " ${PYTHON_SITE_PACKAGES})
else()
  message( FATAL_ERROR "A valid Python interpreter is required for finding Python's site-packages directory!" )
endif()

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")

ENABLE_TESTING()
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!




# Turn compiler warnings up to 11, at least with gcc.  
if (CMAKE_BUILD_TOOL MATCHES "make")
    MESSAGE(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    # NON-optimized build:
    # add_definitions( -Wall  -Wno-deprecated -Werror -pedantic-errors)
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

option(BUILD_TYPE
  "Build type: Release=ON/Debug=OFF  " ON)

add_definitions(-DUNICODE)

if (BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Release")
    set(CMAKE_BUILD_TYPE Release)
endif(BUILD_TYPE)

if (NOT BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Debug")
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT BUILD_TYPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# this defines the source-files for library
set(GEOM_SRC
    Arc.cpp
    Area.cpp
    AreaClipper.cpp
    AreaDxf.cpp
    AreaOrderer.cpp
    AreaPocket.cpp
    Box.cpp
    Circle.cpp
    clipper.cpp
    Construction.cpp
    Curve.cpp
    Curve3D.cpp
    dxf.cpp
    Finite.cpp
    Geom.cpp
    HeeksGeomDxf.cpp
    Matrix.cpp
    Mesh.cpp
    OctTree.cpp
    Offset.cpp
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
