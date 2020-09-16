add_library(cJSON STATIC
  cJSON.h
  cJSON.c
)

target_include_directories(cJSON PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS cJSON
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES cJSON.h
  DESTINATION include
)
