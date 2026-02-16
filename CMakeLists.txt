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

# Include directories - Add include path for local headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# DHT11 Sensor Node
add_executable(dht11_node src/dht11_node.cpp src/dht11_driver.cpp)
target_include_directories(dht11_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(dht11_node rclcpp std_msgs sensor_msgs)
target_link_libraries(dht11_node pigpiod_if2 pthread)

# MQ2 Sensor Node
add_executable(mq2_node src/mq2_node.cpp src/mq2_driver.cpp)
target_include_directories(mq2_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(mq2_node rclcpp std_msgs sensor_msgs)
target_link_libraries(mq2_node pigpiod_if2 pthread)

# Flame Sensor Node
add_executable(flame_node src/flame_node.cpp src/flame_driver.cpp)
target_include_directories(flame_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(flame_node rclcpp std_msgs)
target_link_libraries(flame_node pigpiod_if2 pthread)

# Install targets
install(TARGETS
  dht11_node
  mq2_node
  flame_node
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