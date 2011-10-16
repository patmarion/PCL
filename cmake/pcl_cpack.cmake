# Package creation using CPack

###############################################################################
#find available package generators

# RPM (disabled until RedHat/Fedora users/developers need this)
#find_program(RPM_PROGRAM rpm)
#if(EXISTS ${RPM_PROGRAM})
#  list(APPEND CPACK_GENERATOR "RPM")
#endif(EXISTS ${RPM_PROGRAM})

set(CPACK_PACKAGE_VERSION "${PCL_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PCL_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${PCL_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PCL_REVISION_VERSION}")
set(CPACK_PACKAGE_CONFIG_INSTALL_DIR ${PCLCONFIG_INSTALL_DIR})

# DEB
if("${CMAKE_SYSTEM}" MATCHES "Linux")
  find_program(DPKG_PROGRAM dpkg)
  if(EXISTS ${DPKG_PROGRAM})
    list(APPEND CPACK_GENERATOR "DEB")
  endif(EXISTS ${DPKG_PROGRAM})
endif()

# NSIS
if(WIN32)
  find_program(NSIS_PROGRAM makensis MakeNSIS)
  if(EXISTS ${NSIS_PROGRAM})
    list(APPEND CPACK_GENERATOR "NSIS")
  endif(EXISTS ${NSIS_PROGRAM})
  if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(win_system_name win64)
  else(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES32")
    set(win_system_name win32)
  endif(CMAKE_CL_64)
  set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME}-${PCL_VERSION}-${win_system_name}")
  # force CPACK_PACKAGE_INSTALL_REGISTRY_KEY because of a known limitation in cmake/cpack to be fixed in next releases
  # http://public.kitware.com/Bug/view.php?id=9094
  # This is to allow a 32bit and a 64bit of PCL to get installed on one system
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME} ${PCL_VERSION} ${win_system_name}" )
endif()

# dpkg
if(APPLE)
  find_program(PACKAGE_MAKER_PROGRAM PackageMaker
               HINTS /Developer/Applications/Utilities)
  if(EXISTS ${PACKAGE_MAKER_PROGRAM})
    list(APPEND CPACK_GENERATOR "PackageMaker")
  endif(EXISTS ${PACKAGE_MAKER_PROGRAM})
endif()

include(InstallRequiredSystemLibraries)

set(PCL_CPACK_CFG_FILE "${PCL_BINARY_DIR}/cpack_options.cmake")

###############################################################################
# Make the CPack input file.
macro(PCL_MAKE_CPACK_INPUT)
    set(_cpack_cfg_in "${PCL_SOURCE_DIR}/cmake/cpack_options.cmake.in")

    # Prepare the components list
    set(PCL_CPACK_COMPONENTS)
    PCL_CPACK_MAKE_COMPS_OPTS(PCL_CPACK_COMPONENTS "${_comps}")

    configure_file(${_cpack_cfg_in} ${PCL_CPACK_CFG_FILE} @ONLY)
endmacro(PCL_MAKE_CPACK_INPUT)


macro(PCL_CPACK_MAKE_COMPS_OPTS _var _current)
    set(_comps_list)
    foreach(_ss ${PCL_SUBSYSTEMS})
        if("${_ss}" STREQUAL "global_tests")
            # we don't install global_tests
        else("${_ss}" STREQUAL "global_tests")
            PCL_GET_SUBSYS_STATUS(_status ${_ss})
            if(_status)
                set(_comps_list "${_comps_list} ${_ss}")
                PCL_CPACK_ADD_COMP_INFO(${_var} ${_ss})
            endif(_status)
        endif("${_ss}" STREQUAL "global_tests")
    endforeach(_ss)
    set(${_var} "${${_var}}\nset(CPACK_COMPONENTS_ALL${_comps_list})\n")
endmacro(PCL_CPACK_MAKE_COMPS_OPTS)


macro(PCL_CPACK_ADD_COMP_INFO _var _ss)
    string(TOUPPER "${_ss}" _comp_name)
    set(${_var}
        "${${_var}}set(CPACK_COMPONENT_${_comp_name}_DISPLAY_NAME \"${_ss}\")\n")
    GET_IN_MAP(_desc PCL_SUBSYS_DESC ${_ss})
    set(${_var}
        "${${_var}}set(CPACK_COMPONENT_${_comp_name}_DESCRIPTION \"${_desc}\")\n")
    set(_deps_str)
    GET_IN_MAP(_deps PCL_SUBSYS_DEPS ${_ss})
    foreach(_dep ${_deps})
        set(_deps_str "${_deps_str} ${_dep}")
    endforeach(_dep)
    set(${_var}
        "${${_var}}set(CPACK_COMPONENT_${_comp_name}_DEPENDS ${_deps_str})\n")
endmacro(PCL_CPACK_ADD_COMP_INFO)

