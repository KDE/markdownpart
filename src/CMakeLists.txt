add_definitions(-DTRANSLATION_DOMAIN=\"markdownpart\")

configure_file(markdownpart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop @ONLY)

set(markdownpart_SRCS
    searchtoolbar.cpp
    markdownview.cpp
    markdownpartfactory.cpp
    markdownpart.cpp
    markdownbrowserextension.cpp
)

ki18n_wrap_ui(markdownpart_SRCS searchtoolbar.ui)

qt5_add_resources(markdownpart_SRCS
    markdownpart.qrc
)

add_library(markdownpart  MODULE ${markdownpart_SRCS})
kcoreaddons_desktop_to_json(markdownpart ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop)

target_link_libraries(markdownpart
    KF5::I18n
    KF5::Parts
    Qt5::Widgets
)

install(TARGETS markdownpart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/parts)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(
    FILES org.kde.markdownpart.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)
