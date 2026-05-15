add_library(KF6Codecs)
add_library(KF6::Codecs ALIAS KF6Codecs)

set_target_properties(KF6Codecs PROPERTIES
    VERSION     ${KCODECS_VERSION}
    SOVERSION   ${KCODECS_SOVERSION}
    EXPORT_NAME Codecs
)

ecm_create_qm_loader(KF6Codecs kcodecs6_qt)

target_sources(KF6Codecs PRIVATE
    kcharsets.cpp
    kcharsets.h
    kcodecsbase45.cpp
    kcodecsbase64.cpp
    kcodecsbase64.h
    kcodecs.cpp
    kcodecs.h
    kcodecs_p.h
    kcodecsqp.cpp
    kcodecsqp.h
    kcodecsuuencode.cpp
    kcodecsuuencode.h
    kemailaddress.cpp
    kemailaddress.h
    kencodingprober.cpp
    kencodingprober.h
    kencodingprober_p.h
    probers/CharDistribution.cpp
    probers/CharDistribution.h
    probers/JpCntx.cpp
    probers/JpCntx.h
    probers/LangBulgarianModel.cpp
    probers/LangCyrillicModel.cpp
    probers/LangGreekModel.cpp
    probers/LangHebrewModel.cpp
    probers/LangHungarianModel.cpp
    probers/LangThaiModel.cpp
    probers/nsBig5Prober.cpp
    probers/nsBig5Prober.h
    probers/nsCharSetProber.cpp
    probers/nsCharSetProber.h
    probers/nsCodingStateMachine.h
    probers/nsEscCharsetProber.cpp
    probers/nsEscCharsetProber.h
    probers/nsEscSM.h
    probers/nsEUCJPProber.cpp
    probers/nsEUCJPProber.h
    probers/nsEUCKRProber.cpp
    probers/nsEUCKRProber.h
    probers/nsGB2312Prober.cpp
    probers/nsGB2312Prober.h
    probers/nsHebrewProber.cpp
    probers/nsHebrewProber.h
    probers/nsLatin1Prober.cpp
    probers/nsLatin1Prober.h
    probers/nsMBCSGroupProber.cpp
    probers/nsMBCSGroupProber.h
    probers/nsMBCSSM.h
    probers/nsSBCharSetProber.cpp
    probers/nsSBCharSetProber.h
    probers/nsSBCSGroupProber.cpp
    probers/nsSBCSGroupProber.h
    probers/nsSJISProber.cpp
    probers/nsSJISProber.h
    probers/nsUniversalDetector.cpp
    probers/nsUniversalDetector.h
    probers/UnicodeGroupProber.cpp
    probers/UnicodeGroupProber.h
)

ecm_qt_declare_logging_category(KF6Codecs
    HEADER kcodecs_debug.h
    IDENTIFIER KCODECS_LOG
    CATEGORY_NAME kf.codecs
    OLD_CATEGORY_NAMES kf5.kcodecs
    DESCRIPTION "KCodecs"
    EXPORT KCODECS
)

ecm_generate_export_header(KF6Codecs
    BASE_NAME KCodecs
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Codecs INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCodecs>")
target_compile_features(KF6Codecs PUBLIC cxx_std_20)
target_link_libraries(KF6Codecs PUBLIC Qt6::Core)

ecm_generate_headers(KCodecs_HEADERS
  HEADER_NAMES
  KCharsets
  KCodecs
  KEncodingProber
  KEmailAddress
  REQUIRED_HEADERS KCodecs_HEADERS
)

install(TARGETS KF6Codecs EXPORT KF6CodecsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcodecs_export.h
  ${KCodecs_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCodecs COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KCODECS
    FILE kcodecs.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_generate_qdoc(KF6Codecs kcodecs.qdocconf)
