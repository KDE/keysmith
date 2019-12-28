add_subdirectory(base32)
add_subdirectory(validators)

set(keysmith_SRCS
    main.cpp
    accountmodel.cpp
    account.cpp
)

set(keysmith_internal_libs base32_lib validator_lib)

qt5_add_resources(RESOURCES resources.qrc)
add_executable(org.kde.keysmith ${keysmith_SRCS} ${RESOURCES})
target_link_libraries(org.kde.keysmith
    Qt5::Core Qt5::Qml Qt5::Quick Qt5::Svg
    KF5::I18n
    ${LIBOATH_LIBRARIES}
    ${keysmith_internal_libs}
)
install(TARGETS org.kde.keysmith ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
