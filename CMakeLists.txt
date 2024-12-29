cmake_minimum_required(VERSION 3.8)
project(tictactoe)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#uncomment for debugging:
#set(CMAKE_BUILD_TYPE DEBUG)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS "-g")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(${PROJECT_NAME} src/tictactoe.cpp src/tttgame.cpp)
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

#target_include_directories(test PRIVATE ${HOME}/.local/include)
#target_link_libraries(test PRIVATE ${HOME}/.local/lib/libprotobuf.so)

install(TARGETS
		tictactoe
		DESTINATION lib/${PROJECT_NAME})
