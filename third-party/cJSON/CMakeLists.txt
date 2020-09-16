add_library(cJSON STATIC
  cJSON.h
  cJSON.c
)

install(TARGETS cJSON
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES cJSON.h
  DESTINATION include
)
