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

# Find pigpio daemon interface library
find_library(PIGPIOD_IF2_LIBRARY NAMES pigpiod_if2 REQUIRED)
find_path(PIGPIO_INCLUDE_DIR NAMES pigpiod_if2.h REQUIRED)

# Include directories
include_directories(include ${PIGPIO_INCLUDE_DIR})

# DHT11 Sensor Node
add_executable(dht11_node src/dht11_node.cpp src/dht11_driver.cpp)
ament_target_dependencies(dht11_node rclcpp std_msgs sensor_msgs)
target_link_libraries(dht11_node ${PIGPIOD_IF2_LIBRARY} pthread rt)

# DHT22 Sensor Node
add_executable(dht22_node src/dht22_node.cpp src/dht22_driver.cpp)
ament_target_dependencies(dht22_node rclcpp std_msgs sensor_msgs)
target_link_libraries(dht22_node ${PIGPIOD_IF2_LIBRARY} pthread rt)
target_compile_definitions(dht22_node PRIVATE "READ_DHT22_PY=\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/read_dht22.py\"")
# MQ2 Sensor Node
add_executable(mq2_node src/mq2_node.cpp src/mq2_driver.cpp)
ament_target_dependencies(mq2_node rclcpp std_msgs sensor_msgs)
target_link_libraries(mq2_node ${PIGPIOD_IF2_LIBRARY} pthread rt)

# Flame Sensor Node
add_executable(flame_node src/flame_node.cpp src/flame_driver.cpp)
ament_target_dependencies(flame_node rclcpp std_msgs)
target_link_libraries(flame_node ${PIGPIOD_IF2_LIBRARY} pthread rt)

# Sensor Simulator (no hardware required)
add_executable(sensor_simulator src/sensor_simulator.cpp)
ament_target_dependencies(sensor_simulator rclcpp std_msgs sensor_msgs)

# Install targets
install(TARGETS
  dht11_node
  dht22_node
  mq2_node
  flame_node
  sensor_simulator
  DESTINATION lib/${PROJECT_NAME})

# Install helper python script used by the DHT22 driver
install(PROGRAMS
  scripts/dht22_node_python.py
  scripts/DHT22.py
  scripts/read_dht22.py
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