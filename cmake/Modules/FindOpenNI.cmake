###############################################################################
# Find OpenNI
#
# This sets the following variables:
# OPENNI_FOUND - True if OPENNI was found.
# OPENNI_INCLUDE_DIRS - Directories containing the OPENNI include files.
# OPENNI_LIBRARIES - Libraries needed to use OPENNI.
# OPENNI_DEFINITIONS - Compiler flags for OPENNI.

find_package(PkgConfig)
if(${CMAKE_VERSION} VERSION_LESS 2.8.2)
  pkg_check_modules(PC_OPENNI openni-dev)
else()
  pkg_check_modules(PC_OPENNI QUIET openni-dev)
endif()

set(OPENNI_DEFINITIONS ${PC_OPENNI_CFLAGS_OTHER})

#add a hint so that it can find it without the pkg-config
find_path(OPENNI_INCLUDE_DIR XnStatus.h
    HINTS ${PC_OPENNI_INCLUDEDIR} ${PC_OPENNI_INCLUDE_DIRS} /usr/include/openni
    $ENV{PROGRAMFILES}/OpenNI/include
    PATH_SUFFIXES openni)
#add a hint so that it can find it without the pkg-config
find_library(OPENNI_LIBRARY OpenNI
    HINTS ${PC_OPENNI_LIBDIR} ${PC_OPENNI_LIBRARY_DIRS} /usr/lib $ENV{PROGRAMFILES}/OpenNI/Lib)

set(OPENNI_INCLUDE_DIRS ${OPENNI_INCLUDE_DIR})
set(OPENNI_LIBRARIES ${OPENNI_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenNI DEFAULT_MSG
    OPENNI_LIBRARY OPENNI_INCLUDE_DIR)
    
mark_as_advanced(OPENNI_LIBRARY OPENNI_INCLUDE_DIR)
if(OPENNI_FOUND)
  include_directories(${OPENNI_INCLUDE_DIRS})
  message(STATUS "OpenNI found (include: ${OPENNI_INCLUDE_DIR}, lib: ${OPENNI_LIBRARY})")
endif(OPENNI_FOUND)

