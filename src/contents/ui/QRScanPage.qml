/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 */

import QtQuick
import QtMultimedia

import org.kde.kirigami as Kirigami
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

    CaptureSession {
        videoOutput: viewFinder
        camera: Camera {
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
