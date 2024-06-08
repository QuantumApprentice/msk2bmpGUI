enable_testing()

add_executable(
  QFO2ToolLib_spec
  hello_spec.cpp
)

target_link_libraries(
  QFO2ToolLib_spec PUBLIC
  QFO2ToolLib
  GTest::gtest_main
)

target_include_directories(QFO2ToolLib_spec PUBLIC
  ../src/QFO2ToolLib
)

include(GoogleTest)
gtest_discover_tests(QFO2ToolLib_spec)
