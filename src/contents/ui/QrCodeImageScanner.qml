/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Nicolas Badin <n1coc4cola@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls as QQC2

import QtCore
import QtMultimedia

import org.kde.kirigami as Kirigami

import Keysmith.Application as Application
import Keysmith.Models as Models
import Keysmith.Scanner
import Keysmith.Validator


Kirigami.PromptDialog {
    id: root
    title: "QR Code Area"
    width: parent.width * 0.9
    height: parent.height * 0.7
    iconName: "insert-image-symbolic"
    showCloseButton: true
    standardButtons: QQC2.DialogButtonBox.Close

    property alias value: ou

    OtpUri {
        id: ou
    }

    QRCodeImage {
        id: scanner
        source: image.source

        onDecodedTextChanged: {
            ou.uri = scanner.decodedText;
            if (ou.valid) {
                root.accept();
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: i18n("Select an Image")
        nameFilters: [scanner.supportedFileTypes]
        options: FileDialog.ReadOnly
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

        onAccepted: {
            image.source = fileDialog.selectedFile
        }

        onRejected: {
            root.close();
        }
    }

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

        Rectangle {
            color: "lightgray"
            Layout.fillWidth: true
            Layout.preferredWidth: root.width * 0.8
            Layout.preferredHeight: root.height * 0.8

            // The displayed image
            Image {
                id: image
                source: ""
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                smooth: true
                autoTransform: true

                property real xRatio: 0
                property real yRatio: 0
                property real wRatio: 0
                property real hRatio: 0

                property real renderedWidth: Math.min(image.width, image.height * (image.sourceSize.width / image.sourceSize.height))
                property real renderedHeight: Math.min(image.height, image.width / (image.sourceSize.width / image.sourceSize.height))

                // Mouse area for selection
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent

                    onPressed: function(mouse) {
                        selectionRect.x = mouse.x;
                        selectionRect.y = mouse.y;
                        selectionRect.width = 0;
                        selectionRect.height = 0;
                    }
                    onPositionChanged: function(mouse) {
                        const right = Math.max(mouse.x, selectionRect.x);
                        const bottom = Math.max(mouse.y, selectionRect.y);
                        const left = Math.min(mouse.x, selectionRect.x);
                        const top = Math.min(mouse.y, selectionRect.y);

                        selectionRect.x = left;
                        selectionRect.y = top;
                        selectionRect.width = right - left;
                        selectionRect.height = bottom - top;
                    }
                    onReleased: {
                        // Update ratios based on rendered size
                        const xDelta = (image.width - image.renderedWidth) / 2;
                        const yDelta = (image.height - image.renderedHeight) / 2;

                        image.xRatio = Math.max(selectionRect.x - xDelta, 0) / image.renderedWidth;
                        image.yRatio = Math.max(selectionRect.y - yDelta, 0) / image.renderedHeight;
                        image.wRatio = selectionRect.width / image.renderedWidth;
                        image.hRatio = selectionRect.height / image.renderedHeight;

                        // Update scanner imageRect
                        scanner.imageRect = Qt.rect(
                            (selectionRect.x - xDelta) * (image.width / image.renderedWidth),
                            (selectionRect.y - yDelta) * (image.height / image.renderedHeight),
                            selectionRect.width * (image.width / image.renderedWidth),
                            selectionRect.height * (image.height / image.renderedHeight)
                        );
                    }
                }

                onWidthChanged: {
                    const xDelta = (image.width - image.renderedWidth) / 2;
                    selectionRect.x = image.xRatio * image.renderedWidth + xDelta;
                    selectionRect.width = image.wRatio * image.renderedWidth;
                }
                onHeightChanged: {
                    const yDelta = (image.height - image.renderedHeight) / 2;
                    selectionRect.y = image.yRatio * image.renderedHeight + yDelta;
                    selectionRect.height = image.hRatio * image.renderedHeight;
                }

                // Rectangle to represent the selected zone
                Rectangle {
                    id: selectionRect
                    color: Qt.rgba(Kirigami.Theme.highlightColor.r,
                                   Kirigami.Theme.highlightColor.g,
                                   Kirigami.Theme.highlightColor.b,
                                   0.5)
                    border.color: Kirigami.Theme.highlightColor
                    border.width: 2
                }
            }
        }
    }

    onOpened: {
        fileDialog.open();
    }
    onClosed: {
        fileDialog.close();
    }
}
