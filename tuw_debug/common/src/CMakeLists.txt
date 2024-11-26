configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

add_library(tuw_debug 
  timing.cpp
  debug.cpp
)

install(TARGETS tuw_debug
  # IMPORTANT: Add the tuw_debug library to the "export-set"
  EXPORT TUWDebugTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/tuw"
    COMPONENT dev)
