project(antavore)

set(antavore_SOURCES
      antavore.c
      source.c)

add_library(antavore MODULE
      ${antavore_SOURCES})
target_link_libraries(antavore
      libobs)

install_obs_plugin_with_data(antavore data)