/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Nicolas Badin <n1coc4cola@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import QtMultimedia

import org.kde.kirigami as Kirigami

import Keysmith.Application as Application
import Keysmith.Models as Models
import Keysmith.Scanner
import Keysmith.Validator


Kirigami.PromptDialog {
    id: root
    title: "QR Code Scanner"
    width: parent.width * 0.9
    height: parent.height * 0.7
    iconName: "view-barcode-qr-symbolic"
    showCloseButton: true
    standardButtons: QQC2.DialogButtonBox.Close

    property alias value: ou

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        Layout.margins: Kirigami.Units.smallSpacing

        RowLayout {
            id: mainLayout
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Kirigami.Icon {
                id: icon
                source: root.iconName
                visible: root.iconName.length > 0

                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                Layout.alignment: Qt.AlignTop
            }

            Kirigami.Heading {
                id: heading
                text: root.title
                visible: root.title.length > 0
                elide: QQC2.Label.ElideRight
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        VideoOutput {
            id: videoOutput
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: root.height - heading.height - root.footer.height
            Layout.preferredWidth: root.width - 2*icon.width
            fillMode: VideoOutput.PreserveAspectFit
       }
    }

    OtpUri {
        id: ou
    }

    QRCodeVideo {
        id: scanner

        onDecodedTextChanged: {
            ou.uri = scanner.decodedText;
            if (ou.valid) {
                root.accept();
            }
        }
    }

    CaptureSession {
        id: captureSession
        camera: Camera {
            id: camera
        }

        videoOutput: videoOutput
    }

    onOpened: {
        //scanner.setCapture(videoOutput.videoSink);
        camera.start();
    }
    onClosed: {
        camera.stop();
    }
    Component.onCompleted: {
        scanner.setCapture(videoOutput.videoSink);
    }
}
