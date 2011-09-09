add_library(linenoise MODULE linenoise.c linenoise.h)

set_target_properties(linenoise PROPERTIES
  PREFIX ""
)

install(TARGETS linenoise LIBRARY DESTINATION "modules/linenoise")
install(FILES linenoise.st DESTINATION "modules/linenoise")
