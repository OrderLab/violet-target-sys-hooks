add_library(violet STATIC
  violet_sys.h
  violet_sys.c
)

set(LIBVIOLET "violet")

install(TARGETS violet
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES violet_sys.h
  DESTINATION include
)

