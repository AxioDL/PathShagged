add_executable(mkqticon mkqticon.c)
target_link_libraries(mkqticon ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
target_include_directories(mkqticon PRIVATE ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

macro(declare_qticon_target)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/metaforce-gui/platforms/freedesktop/mainicon_qt.bin
                   COMMAND $<TARGET_FILE:mkqticon>
                   ARGS ${CMAKE_BINARY_DIR}/metaforce-gui/platforms/freedesktop/mainicon_qt.bin
                   DEPENDS
                   ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop/128x128/apps/metaforce-gui.png
                   ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop/64x64/apps/metaforce-gui.png
                   ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop/48x48/apps/metaforce-gui.png
                   ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop/32x32/apps/metaforce-gui.png
                   ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop/16x16/apps/metaforce-gui.png
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/metaforce-gui/platforms/freedesktop
                   COMMENT "Generating mainicon_qt.bin")
bintoc(mainicon_qt.cpp ${CMAKE_BINARY_DIR}/metaforce-gui/platforms/freedesktop/mainicon_qt.bin MAINICON_QT)
endmacro()
