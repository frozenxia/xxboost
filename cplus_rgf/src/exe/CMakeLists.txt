add_executable(test_dataset test_dataset.cpp)
target_link_libraries(test_dataset base)

install(TARGETS  test_dataset DESTINATION ${CMAKE_SOURCE_DIR}/bin)