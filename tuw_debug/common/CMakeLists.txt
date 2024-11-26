include_directories(include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

add_subdirectory(src)
add_subdirectory(examples)

INSTALL(DIRECTORY include/  DESTINATION include FILES_MATCHING PATTERN "*.hpp")
