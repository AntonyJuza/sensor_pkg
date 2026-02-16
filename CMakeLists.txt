cmake_minimum_required(VERSION 3.8)
project(sensor_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

# Find pigpio library
find_library(PIGPIO_LIBRARY NAMES pigpio REQUIRED)
find_path(PIGPIO_INCLUDE_DIR NAMES pigpio.h REQUIRED)

# Include directories
include_directories(include ${PIGPIO_INCLUDE_DIR})

# DHT11 Sensor Node
add_executable(dht11_node src/dht11_node.cpp src/dht11_driver.cpp)
ament_target_dependencies(dht11_node rclcpp std_msgs sensor_msgs)
target_link_libraries(dht11_node ${PIGPIO_LIBRARY} pthread rt)

# MQ2 Sensor Node
add_executable(mq2_node src/mq2_node.cpp src/mq2_driver.cpp)
ament_target_dependencies(mq2_node rclcpp std_msgs sensor_msgs)
target_link_libraries(mq2_node ${PIGPIO_LIBRARY} pthread rt)

# Flame Sensor Node
add_executable(flame_node src/flame_node.cpp src/flame_driver.cpp)
ament_target_dependencies(flame_node rclcpp std_msgs)
target_link_libraries(flame_node ${PIGPIO_LIBRARY} pthread rt)

# Sensor Simulator (no hardware required)
add_executable(sensor_simulator src/sensor_simulator.cpp)
ament_target_dependencies(sensor_simulator rclcpp std_msgs sensor_msgs)

# Install targets
install(TARGETS
  dht11_node
  mq2_node
  flame_node
  sensor_simulator
  DESTINATION lib/${PROJECT_NAME})

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# Install header files
install(DIRECTORY
  include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()