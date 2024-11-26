cmake_minimum_required(VERSION 3.0)
project(TUWDebug)
set(TUWDEBUG_MAJOR_VERSION 0)
set(TUWDEBUG_MINOR_VERSION 1)
set(TUWDEBUG_PATCH_VERSION 0)
set(TUWDEBUG_VERSION
  ${TUWDEBUG_MAJOR_VERSION}.${TUWDEBUG_MINOR_VERSION}.${TUWDEBUG_PATCH_VERSION})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/TUWDebug)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/common/include")
include_directories(${PROJECT_INCLUDE_DIR})  # to find foo/config.h

add_subdirectory(common)


# The interesting stuff goes here
# ===============================

# Add all targets to the build-tree export set
export(TARGETS tuw_debug tuw_debug_demo
  FILE "${PROJECT_BINARY_DIR}/TUWDebugTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE TUWDebug)

# Create the TUWDebugConfig.cmake and TUWDebugConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_INCLUDE_DIR}")  # local config
configure_file(TUWDebugConfig.cmake.in
  "${PROJECT_BINARY_DIR}/TUWDebugConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "${INSTALL_INCLUDE_DIR}")  # installed config
configure_file(TUWDebugConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TUWDebugConfig.cmake" @ONLY)
# ... for both
configure_file(TUWDebugConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/TUWDebugConfigVersion.cmake" @ONLY)

# Install the TUWDebugConfig.cmake and TUWDebugConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TUWDebugConfig.cmake"
  "${PROJECT_BINARY_DIR}/TUWDebugConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
  
# Install the export set for use with the install-tree
install(EXPORT TUWDebugTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)


# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
