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
)
if (QT_MAJOR_VERSION STREQUAL "6")
    # after full switch to KF6, drop 6 suffix again
    target_sources(markdownpart PRIVATE markdownpart6.qrc)
else()
    target_sources(markdownpart PRIVATE markdownpart.qrc)
endif()

ki18n_wrap_ui(markdownpart searchtoolbar.ui)

target_link_libraries(markdownpart
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Parts
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS markdownpart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)

install(
    FILES org.kde.markdownpart.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)
