/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtMultimedia

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import org.kde.prison.scanner as Prison

import Keysmith.Application as Application

Kirigami.Page {
    id: root

    title: i18nc("@title", "Scan a QR code")

    required property Application.ScanQRViewModel vm

    padding: 0

    onBackRequested: event => {
        event.accepted = true;
        vm.cancelled();
    }

    actions: Kirigami.Action {
        text: i18nc("@action:intoolbar", "Select Camera")
        visible: devices.videoInputs.length > 1
        icon.name: "camera-video-symbolic"
        onTriggered: cameraSelectorSheet.open()
    }

    Components.MessageDialog {
        id: cameraSelectorSheet

        padding: 0
        bottomPadding: 1 // HACK: prevent the scrollview to go out of the dialog
        title: i18nc("@title:dialog", "Select Camera")

        header: ColumnLayout {
            width: parent.width
            spacing: 0

            RowLayout {
                spacing: 0

                Layout.margins: Kirigami.Units.smallSpacing
                Kirigami.Heading {
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true
                    text: cameraSelectorSheet.title
                }

                QQC2.ToolButton {
                    icon.name: "dialog-close"
                    text: i18nc("@action:button", "Close")
                    display: QQC2.Button.IconOnly
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }
        }

        contentItem: QQC2.ScrollView {
            ListView {
                id: listView

                Kirigami.Theme.colorSet: Kirigami.Theme.View

                clip: true
                model: devices.videoInputs
                implicitWidth: Kirigami.Units.gridUnit * 20

                delegate: Delegates.RoundedItemDelegate {
                    required property int index
                    required property var modelData

                    text: modelData.description
                    onClicked: {
                        camera.cameraDevice = modelData;
                        camera.start();
                        cameraSelectorSheet.close();
                        Application.StateConfig.lastCameraId = modelData.id;
                        Application.StateConfig.save();
                    }
                }
            }
        }
    }

    MediaDevices {
        id: devices

        Component.onCompleted: setCamera()
        onVideoInputsChanged: setCamera()

        function setCamera(): void {
            if (Application.StateConfig.lastCameraId.length === 0) {
                return;
            }

            for (let cameraDevice of videoInputs) {
                // note strict checking doesn't work
                if (cameraDevice.id == Application.StateConfig.lastCameraId) {
                    camera.cameraDevice = cameraDevice;
                    camera.start();
                }
            }
        }
    }

    CaptureSession {
        videoOutput: viewFinder
        camera: Camera {
            id: camera
            active: vm.active
            cameraDevice: devices.defaultVideoInput
        }
    }

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
    }

    Prison.VideoScanner {
        videoSink: viewFinder.videoSink
        formats: Prison.Format.QRCode

        onResultChanged: {
            if (result.hasText) {
                vm.active = false;
                vm.scanCompleteText(result.text);
            } else if (result.hasBinaryData) {
                vm.active = false;
                vm.scanComplete(result.binaryData);
            }
        }
    }

    CameraPermission {
        id: permission
        onStatusChanged: {
            if (status == Qt.PermissionStatus.Granted) {
                camera.start();
            }
        }
    }
    Component.onCompleted: {
        if (permission.status == Qt.PermissionStatus.Undetermined)
            permission.request();
    }
}
