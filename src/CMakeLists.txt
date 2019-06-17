set(SOURCE_FILES
  main.c
  ui.c
  buf.c
  debug.c
  base64.c
)

add_executable(certalize ${SOURCE_FILES})
target_link_libraries(certalize ${LIBS})
install(TARGETS certalize DESTINATION ${INSTALL_BINDIR})
