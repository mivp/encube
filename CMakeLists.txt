cmake_minimum_required(VERSION 2.8.1)
project(encube_om)
find_package(Omegalib)

set(S2VOLSURF $ENV{S2VOLSURF})
set(S2PATH $ENV{S2PATH})
set(S2ARCH $ENV{S2ARCH})

#######################################################################################################################
# MAC - system build
include_directories(${OMEGA_INCLUDE_DIRS} ${S2PATH}/src ${S2VOLSURF} /usr/X11/include /opt/local/include ../vr.s2omega)

# Setup compile info
add_executable(encube_om encube_om.cpp hdsupport.cpp utility.cpp routines.cpp json.cpp)
add_definitions(-DENCUBE_OMEGA)

target_link_libraries(encube_om ${OMEGA_LIB} ${OMEGA_TOOLKIT_LIB} ${CYCLOPS_LIB} ${S2VOLSURF}/${S2ARCH}/libxrw.dylib /opt/local/lib/libcfitsio.dylib /usr/lib/libz.dylib /opt/local/lib/libGLEW.dylib /System/Library/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib)


