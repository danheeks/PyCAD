project( step )
cmake_minimum_required( VERSION 2.8.3 )

FIND_PACKAGE(Boost 1.73.0 COMPONENTS python3)
FIND_PACKAGE(PythonInterp 3)
FIND_PACKAGE(PythonLibs 3 REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!

#find OCE or OpenCASCADE
set( CASCADE_LIBS TKernel )
set( OCE_DIR "/usr/local/share/cmake/" )
find_package ( OCE )
if( ${OCE_FOUND} )
  add_definitions ( -DHAVE_CONFIG_H )
  set( OpenCASCADE_LIBRARIES ${CASCADE_LIBS} )
  set( OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS} )
  message("OCE found")
else() #look for OpenCASCADE
  set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/" )
  find_package ( OpenCASCADE REQUIRED COMPONENTS ${CASCADE_LIBS} )
endif()

message("OpenCASCADE_INCLUDE_DIR = ")
message(${OpenCASCADE_INCLUDE_DIR})
message("OpenCASCADE_LIBRARIES = ")
message(${OpenCASCADE_LIBRARIES})

#LINK_LIBRARIES("TKBRep" "TKIGES" "TKShHealing" "TKSTEP" "TKSTEP209" "TKSTEPAttr" "TKSTEPBase" "TKXSBase" "TKShapeSchema" "FWOSPlugin" "PTKernel" "TKBool" "TKCAF" "TKCDF" "TKernel" "TKFeat" "TKFillet" "TKG2d" "TKG3d" "TKGeomAlgo" "TKGeomBase" "TKHLR" "TKMath" "TKOffset" "TKPCAF" "TKPrim" "TKPShape" "TKService" "TKTopAlgo" "TKV3d" "TKMesh" "TKBO" "TKXDESTEP" "TKXCAF" "TKXCAFSchema" "TKLCAF" "TKPLCAF")

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

find_package( OpenGL REQUIRED )

if (NOT WIN32)
    message("opengl directory is")
    message(${OPENGL_INCLUDE_DIR})

    include_directories(OPENGL_INCLUDE_DIR)
endif (NOT WIN32)

include_directories(${OpenCASCADE_INCLUDE_DIR})
include_directories("../Geom")
include_directories("../CAD")

# this defines the source-files for library
set(STEP_SRC
    ../Geom/Arc.cpp
    ../Geom/Area.cpp
    ../Geom/AreaClipper.cpp
    ../Geom/AreaOrderer.cpp
    ../Geom/AreaPocket.cpp
    ../Geom/Circle.cpp
    ../Geom/clipper.cpp
    ../Geom/Construction.cpp
    ../Geom/Curve.cpp
    ../Geom/Finite.cpp
    ../Geom/Matrix.cpp
    ../Geom/Offset.cpp
    ../CAD/DigitizedPoint.cpp
    ../CAD/EndedObject.cpp
    ../CAD/HeeksColor.cpp
    ../CAD/HeeksObj.cpp
    ../CAD/IdNamedObjList.cpp
    ../CAD/LeftAndRight.cpp
    ../CAD/ObjList.cpp
    ../CAD/Picking.cpp
    ../CAD/Property.cpp
    ../CAD/strconv.cpp
    ../CAD/tinystr.cpp
    ../CAD/tinyxml.cpp
    ../CAD/tinyxmlerror.cpp
    ../CAD/tinyxmlparser.cpp
    Cone.cpp
    ConversionTools.cpp
    CoordinateSystem.cpp
    Cuboid.cpp
    Cylinder.cpp
    Edge.cpp
    EllipseDrawing.cpp
    Face.cpp
    FaceTools.cpp
    HEllipse.cpp
    HSpline.cpp
    Loop.cpp
    OCCGeom.cpp
    PropertySolid.cpp
    PythonStuff.cpp
    RuledSurface.cpp
    Settings.cpp
    Shape.cpp
    ShapeData.cpp
    ShapeTools.cpp
    Solid.cpp
    SolidTools.cpp
    Sphere.cpp
    StepFileObject.cpp
    Vertex.cpp
    Wire.cpp
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
      step
      MODULE
      ${STEP_SRC}
  )

target_link_libraries(step ${Boost_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
set_target_properties(step PROPERTIES PREFIX "") 


# this figures out where to install the Python modules
execute_process(
    COMMAND python3 -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
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
    TARGETS step
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${Python_site_packages}
)


