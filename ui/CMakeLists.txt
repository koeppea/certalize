set(UI_FILES
  widgets.ui
  menus.ui
  style.css
)

foreach(f IN LISTS UI_FILES)
  configure_file(${f} ${f} COPYONLY)
endforeach()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${UI_FILES}
  DESTINATION ${INSTALL_UIDIR})
