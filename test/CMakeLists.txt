add_executable(test_workload
  test_workload.c
)

target_link_libraries(test_workload
  PUBLIC violet
  PUBLIC cJSON
)

