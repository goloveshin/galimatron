# installs galimatron and the Saxon libraries it loads, then builds a package.
# Windows: NSIS setup when makensis is on PATH, and a zip.
# Linux: tar.gz, plus deb and rpm when those tools are on PATH.
# the Saxon libraries sit next to the executable. Windows finds a DLL there.
# Linux finds a shared library there through INSTALL_RPATH $ORIGIN.
# share/galimatron holds gal-functions.xsl. catalog.xml maps the GitHub URI to that file.
# the executable loads ../share/galimatron/catalog.xml.

include(GNUInstallDirs)
include(InstallRequiredSystemLibraries)

install(
    TARGETS
        galimatron_executable
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    TARGETS
        saxonc-he
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# the core library ships only a Release binary, and a Debug build uses it.
# on Linux the loader asks for the SONAME, which is a symlink to that binary.
get_target_property(galimatron_saxon_core SaxonCCoreHE::saxonc-core-he IMPORTED_LOCATION_RELEASE)
if(NOT galimatron_saxon_core)
    message(FATAL_ERROR "SaxonC core runtime library was not found")
endif()
install(
    FILES
        ${galimatron_saxon_core}
    DESTINATION
        ${CMAKE_INSTALL_BINDIR}
)
if(UNIX)
    get_target_property(galimatron_saxon_core_soname SaxonCCoreHE::saxonc-core-he IMPORTED_SONAME_RELEASE)
    cmake_path(GET galimatron_saxon_core FILENAME galimatron_saxon_core_name)
    install(CODE "
        file(CREATE_LINK
            \"${galimatron_saxon_core_name}\"
            \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${galimatron_saxon_core_soname}\"
            SYMBOLIC
        )
    ")
    set_target_properties(
        galimatron_executable
        saxonc-he
        PROPERTIES
            INSTALL_RPATH "$ORIGIN"
    )
endif()

install(
    FILES
        ${CMAKE_SOURCE_DIR}/galimatron/translator_library/xslt/gal-functions.xsl
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/galimatron/xslt
)

install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/catalog.xml
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/galimatron
)

# the license of Galimatron and the license texts of the libraries it carries.
# MIT, BSD and Apache require that these travel with a distributed copy.
install(
    FILES
        ${CMAKE_SOURCE_DIR}/LICENSE
        ${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/doc/galimatron
)

install(
    DIRECTORY
        ${CMAKE_SOURCE_DIR}/third_party_licenses
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/doc/galimatron
)

if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    install(
        PROGRAMS
            ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    )
endif()

set(CPACK_PACKAGE_NAME "galimatron")
set(CPACK_PACKAGE_VENDOR "Galimatron")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Galimatron language translator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Galimatron")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Galimatron")
set(CPACK_VERBATIM_VARIABLES ON)
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)

if(WIN32)
    set(CPACK_GENERATOR ZIP)
    find_program(GALIMATRON_MAKENSIS makensis)
    if(GALIMATRON_MAKENSIS)
        list(APPEND CPACK_GENERATOR NSIS)
    else()
        message(STATUS "NSIS not found. The Windows package is a zip until makensis is on PATH.")
    endif()
    # no optional parts, so NSIS does not show the component page
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_NSIS_DISPLAY_NAME "Galimatron")
    set(CPACK_NSIS_PACKAGE_NAME "Galimatron")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    # art/galimatron.ico is the logo. NSIS uses it for the installer and the uninstaller.
    # the same file is installed as bin/galimatron.ico and registered as the Apps icon.
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/art/galimatron.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/art/galimatron.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\galimatron.ico")
    install(
        FILES
            ${CMAKE_SOURCE_DIR}/art/galimatron.ico
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    )
    # this directory's NSIS.InstallOptions.ini.in is the CPack page without the desktop shortcut
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/nsis")
else()
    set(CPACK_GENERATOR TGZ)
    find_program(GALIMATRON_DPKG dpkg-deb)
    if(GALIMATRON_DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    else()
        message(STATUS "dpkg-deb not found. The Linux package has no .deb until it is.")
    endif()
    find_program(GALIMATRON_RPM rpmbuild)
    if(GALIMATRON_RPM)
        list(APPEND CPACK_GENERATOR RPM)
    else()
        message(STATUS "rpmbuild not found. The Linux package has no .rpm until it is.")
    endif()
    set(CPACK_DEBIAN_PACKAGE_NAME "galimatron")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Galimatron")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
    set(CPACK_RPM_PACKAGE_LICENSE "BSL-1.0")
endif()

include(CPack)
