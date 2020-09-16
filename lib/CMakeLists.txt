add_library(violet STATIC
  violet_sys.h
  violet_sys.c
)

target_include_directories(violet PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(violet
  cJSON
)

set(LIBVIOLET "violet")

install(TARGETS violet
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES violet_sys.h
  DESTINATION include
)
