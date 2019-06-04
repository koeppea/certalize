set(SOURCE_FILES
  main.c
  ui.c
)

add_executable(certalize ${SOURCE_FILES})
target_link_libraries(certalize ${LIBS})
install(TARGETS certalize DESTINATION ${INSTALL_BINDIR})
