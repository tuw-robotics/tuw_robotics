add_executable(tuw_debug_demo demo.cpp)
target_link_libraries(tuw_debug_demo tuw_debug)


install(TARGETS tuw_debug_demo
  # IMPORTANT: Add the tuw_debug_demo executable to the "export-set"
  EXPORT TUWDebugTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)
  
