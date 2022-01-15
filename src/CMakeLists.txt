add_definitions(-DTRANSLATION_DOMAIN=\"markdownpart\")

configure_file(markdownpart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop @ONLY)

add_library(markdownpart MODULE)

target_sources(markdownpart PRIVATE
    searchtoolbar.cpp
    markdownview.cpp
    markdownpartfactory.cpp
    markdownpart.cpp
    markdownbrowserextension.cpp
    markdownview.hpp
    markdownbrowserextension.hpp
    markdownpart.hpp
    searchtoolbar.hpp
    markdownpartfactory.hpp

    markdownpart.qrc
)

ki18n_wrap_ui(markdownpart searchtoolbar.ui)

kcoreaddons_desktop_to_json(markdownpart ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop)

target_link_libraries(markdownpart
    KF5::I18n
    KF5::Parts
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS markdownpart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICESDIR})

install(
    FILES org.kde.markdownpart.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)
