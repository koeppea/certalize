set(SOURCE_FILES
  main.c
  ui.c
)

add_executable(certalize ${SOURCE_FILES})
install(TARGETS certalize DESTINATION ${INSTALL_BINDIR})
