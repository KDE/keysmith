/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 */

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

    title: "Scan a QR code"

    required property Application.ScanQRViewModel vm

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
                    }
                }
            }
        }
    }

    MediaDevices {
        id: devices
    }

    CaptureSession {
        videoOutput: viewFinder
        camera: Camera {
            id: camera
            active: vm.active
        }
    }

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
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
}
