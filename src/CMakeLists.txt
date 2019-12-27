add_subdirectory(base32)
add_subdirectory(account)
add_subdirectory(model)
add_subdirectory(validators)
add_subdirectory(app)

set(keysmith_SRCS
    main.cpp
)

set(keysmith_internal_libs base32_lib validator_lib account_lib model_lib keysmith_lib)

qt5_add_resources(RESOURCES resources.qrc)
add_executable(keysmith ${keysmith_SRCS} ${RESOURCES})
target_link_libraries(keysmith
    Qt5::Core Qt5::Qml Qt5::Quick Qt5::Svg
    KF5::I18n
    ${LIBOATH_LIBRARIES}
    ${keysmith_internal_libs}
)
install(TARGETS keysmith ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
