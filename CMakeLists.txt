cmake_minimum_required(VERSION 3.16)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "26")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump MARKDOWNPART_BASE_VERSION once new features are added
set(MARKDOWNPART_BASE_VERSION "0.1")
set(MARKDOWNPART_VERSION "${MARKDOWNPART_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(markdownpart VERSION ${MARKDOWNPART_VERSION})

set(QT_MIN_VERSION "6.7.0")
set(KF_MIN_VERSION "6.3.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMDeprecationSettings)
include(FeatureSummary)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    Parts
)

ecm_set_disabled_deprecation_versions(
    QT 6.10.0
    KF 6.17.0
)
include(FetchContent)
FetchContent_Declare(
    md4qt
    GIT_REPOSITORY https://github.com/igormironchik/md4qt.git
    GIT_TAG        main
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(md4qt)

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT -DMD4QT_QT_SUPPORT)
add_subdirectory(src)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
