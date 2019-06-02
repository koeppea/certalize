set(UI_FILES
  main.ui
)

foreach(f IN LISTS UI_FILES)
  configure_file(${f} ${f} COPYONLY)
endforeach()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${UI_FILES}
  DESTINATION ${INSTALL_UIDIR})
