add_subdirectory(validators)

set(otpclient_SRCS
    main.cpp
    accountmodel.cpp
    account.cpp
    base32.cpp
    )

set(otpclient_internal_libs validator_lib)

qt5_add_resources(RESOURCES resources.qrc)
add_executable(org.kde.otpclient ${otpclient_SRCS} ${RESOURCES})
target_link_libraries(org.kde.otpclient
    Qt5::Core Qt5::Qml Qt5::Quick Qt5::Svg
    ${LIBOATH_LIBRARIES}
    ${otpclient_internal_libs}
)
install(TARGETS org.kde.otpclient ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
