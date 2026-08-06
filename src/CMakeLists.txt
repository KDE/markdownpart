add_definitions(-DTRANSLATION_DOMAIN=\"markdownpart\")

configure_file(markdownpart.json.in ${CMAKE_CURRENT_BINARY_DIR}/markdownpart.json @ONLY)

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

target_link_libraries(markdownpart
    KF6::I18n
    KF6::Parts
    Qt6::Widgets
    md4qt::md4qt
)

target_include_directories(markdownpart PRIVATE ${md4qt_SOURCE_DIR}/src)

install(TARGETS markdownpart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)

install(
    FILES org.kde.markdownpart.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)
