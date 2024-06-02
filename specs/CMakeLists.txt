enable_testing()

add_executable(
  msk2bmpGUI_spec
  hello_spec.cpp
)

target_link_libraries(
  msk2bmpGUI_spec PUBLIC
  msk2bmp
  GTest::gtest_main
)

target_include_directories(msk2bmpGUI_spec PUBLIC
  ../src/msk2bmp
)

include(GoogleTest)
gtest_discover_tests(msk2bmpGUI_spec)
