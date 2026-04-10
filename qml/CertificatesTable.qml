import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        // ── Header Bar ─────────────────────────────────────────────────
        Rectangle {
            id: upperRectangle
            width: parent.width
            height: 60
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            border.color: "lightgray"
            color: "#F2F2F2"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text { text: "Certificates"; color: "darkgray"; bottomPadding: 15 }
                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Image { source: "qrc:/resources/images/icons/refresh_arrow.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/left_arrow.png";    width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/down_arrow.png";    width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/dots.png";          width: 20; height: 20; MouseArea { anchors.fill: parent } }
                }
            }
        }

        // ── Bottom Bar (declared before ScrollView so anchors resolve) ─
        Rectangle {
            id: bottomBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "white"
            border.color: "#E8E9EB"; border.width: 1

            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16; spacing: 8

                Button {
                    id: applyBtn; text: "Apply"
                    implicitWidth: 80; implicitHeight: 35
                    background: Rectangle { radius: 5; color: "white"; border.color: "darkgray"; border.width: 1 }
                    contentItem: Text { text: applyBtn.text; color: "black"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {}
                }
            }
        }

        // ── ScrollView with Item wrapper (prevents blank screen) ───────
        ScrollView {
            id: scrollView
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            Item {
                id: scrollContent
                width: scrollView.width
                height: mainCol.implicitHeight + 40

                ColumnLayout {
                    id: mainCol
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 16
                    anchors.topMargin: 14
                    spacing: 16

                    // ── Allowed Certificates ───────────────────────────
                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: "Allowed certificates"
                            color: "black"
                            font.pixelSize: 12
                        }

                        RowLayout {
                            width: parent.width
                            spacing: 6

                            // List area
                            Rectangle {
                                Layout.fillWidth: true
                                height: 130
                                border.color: "darkgray"; border.width: 1
                                color: "white"; radius: 2
                                clip: true

                                ListView {
                                    id: allowedCertsList
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    model: allowedCertsModel
                                    clip: true
                                    ScrollBar.vertical: ScrollBar {
                                        active: allowedCertsList.contentHeight > allowedCertsList.height
                                    }
                                    delegate: Rectangle {
                                        width: allowedCertsList.width
                                        height: 26
                                        color: allowedCertsList.currentIndex === index ? "#DA3450" : "transparent"
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.left: parent.left; anchors.leftMargin: 6
                                            text: modelData
                                            color: allowedCertsList.currentIndex === index ? "white" : "black"
                                            font.pixelSize: 11
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: allowedCertsList.currentIndex = index
                                        }
                                    }
                                }
                            }

                            // Red X button
                            Rectangle {
                                width: 28; height: 28
                                color: "white"; radius: 3
                                border.color: "#DA3450"; border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"; color: "#DA3450"
                                    font.pixelSize: 13; font.bold: true
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (allowedCertsList.currentIndex >= 0) {
                                            allowedCertsModel.remove(allowedCertsList.currentIndex)
                                            allowedCertsList.currentIndex = -1
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ── Rejected Certificates ──────────────────────────
                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: "Rejected certificates"
                            color: "black"
                            font.pixelSize: 12
                        }

                        RowLayout {
                            width: parent.width
                            spacing: 6

                            Rectangle {
                                Layout.fillWidth: true
                                height: 130
                                border.color: "darkgray"; border.width: 1
                                color: "white"; radius: 2
                                clip: true

                                ListView {
                                    id: rejectedCertsList
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    model: rejectedCertsModel
                                    clip: true
                                    ScrollBar.vertical: ScrollBar {
                                        active: rejectedCertsList.contentHeight > rejectedCertsList.height
                                    }
                                    delegate: Rectangle {
                                        width: rejectedCertsList.width
                                        height: 26
                                        color: rejectedCertsList.currentIndex === index ? "#DA3450" : "transparent"
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.left: parent.left; anchors.leftMargin: 6
                                            text: modelData
                                            color: rejectedCertsList.currentIndex === index ? "white" : "black"
                                            font.pixelSize: 11
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: rejectedCertsList.currentIndex = index
                                        }
                                    }
                                }
                            }

                            // Red X button
                            Rectangle {
                                width: 28; height: 28
                                color: "white"; radius: 3
                                border.color: "#DA3450"; border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"; color: "#DA3450"
                                    font.pixelSize: 13; font.bold: true
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (rejectedCertsList.currentIndex >= 0) {
                                            rejectedCertsModel.remove(rejectedCertsList.currentIndex)
                                            rejectedCertsList.currentIndex = -1
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ── Selected Certificate Panel ─────────────────────
                    Rectangle {
                        Layout.fillWidth: true
                        height: selectedCertCol.implicitHeight + 20
                        border.color: "gray"; border.width: 1
                        color: "#F2F2F2"; radius: 2

                        // Floating label on border
                        Label {
                            text: "Selected certificate"
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.topMargin: -10
                            z: 1
                            font.pixelSize: 12
                            background: Rectangle {
                                color: "#F2F2F2"
                                anchors.fill: parent
                                anchors.leftMargin: -5
                                anchors.rightMargin: -5
                            }
                        }

                        ColumnLayout {
                            id: selectedCertCol
                            anchors.fill: parent
                            anchors.margins: 12
                            anchors.topMargin: 16
                            spacing: 8

                            // ── Issued To ──────────────────────────────
                            Rectangle {
                                Layout.fillWidth: true
                                height: issuedToCol.implicitHeight + 16
                                border.color: "lightgray"; border.width: 1
                                color: "transparent"; radius: 2

                                Label {
                                    text: "Issued To"
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.topMargin: -9
                                    z: 1
                                    font.pixelSize: 11
                                    background: Rectangle { color: "#F2F2F2"; anchors.fill: parent; anchors.leftMargin: -4; anchors.rightMargin: -4 }
                                }

                                ColumnLayout {
                                    id: issuedToCol
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    anchors.topMargin: 12
                                    spacing: 4

                                    CertInfoRow { label: "Country";     value: "--" }
                                    CertInfoRow { label: "Organization"; value: "--" }
                                    CertInfoRow { label: "Common Name"; value: "--" }
                                }
                            }

                            // ── Issued By ──────────────────────────────
                            Rectangle {
                                Layout.fillWidth: true
                                height: issuedByCol.implicitHeight + 16
                                border.color: "lightgray"; border.width: 1
                                color: "transparent"; radius: 2

                                Label {
                                    text: "Issued By"
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.topMargin: -9
                                    z: 1
                                    font.pixelSize: 11
                                    background: Rectangle { color: "#F2F2F2"; anchors.fill: parent; anchors.leftMargin: -4; anchors.rightMargin: -4 }
                                }

                                ColumnLayout {
                                    id: issuedByCol
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    anchors.topMargin: 12
                                    spacing: 4

                                    CertInfoRow { label: "Country";     value: "--" }
                                    CertInfoRow { label: "Organization"; value: "--" }
                                    CertInfoRow { label: "Common Name"; value: "--" }
                                }
                            }

                            // ── Validity Period ────────────────────────
                            Rectangle {
                                Layout.fillWidth: true
                                height: validityCol.implicitHeight + 16
                                border.color: "lightgray"; border.width: 1
                                color: "transparent"; radius: 2

                                Label {
                                    text: "Validity Period"
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.topMargin: -9
                                    z: 1
                                    font.pixelSize: 11
                                    background: Rectangle { color: "#F2F2F2"; anchors.fill: parent; anchors.leftMargin: -4; anchors.rightMargin: -4 }
                                }

                                ColumnLayout {
                                    id: validityCol
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    anchors.topMargin: 12
                                    spacing: 4

                                    CertInfoRow { label: "Issued On";  value: "--" }
                                    CertInfoRow { label: "Expires On"; value: "--" }
                                }
                            }

                            // ── Fingerprints (SHA256) ──────────────────
                            Rectangle {
                                Layout.fillWidth: true
                                height: fingerprintsCol.implicitHeight + 16
                                border.color: "lightgray"; border.width: 1
                                color: "transparent"; radius: 2

                                Label {
                                    text: "Fingerprints (SHA256)"
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.topMargin: -9
                                    z: 1
                                    font.pixelSize: 11
                                    background: Rectangle { color: "#F2F2F2"; anchors.fill: parent; anchors.leftMargin: -4; anchors.rightMargin: -4 }
                                }

                                ColumnLayout {
                                    id: fingerprintsCol
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    anchors.topMargin: 12
                                    spacing: 4

                                    // Certificate fingerprint
                                    RowLayout {
                                        Layout.fillWidth: true; spacing: 8
                                        Text { text: "--"; color: "gray"; font.pixelSize: 11; Layout.preferredWidth: 120 }
                                        Text { text: "Certificate"; color: "black"; font.pixelSize: 11 }
                                    }

                                    // Public key fingerprint
                                    RowLayout {
                                        Layout.fillWidth: true; spacing: 8
                                        Text { text: "--"; color: "gray"; font.pixelSize: 11; Layout.preferredWidth: 120 }
                                        Text { text: "Public key"; color: "black"; font.pixelSize: 11 }
                                    }
                                }
                            }
                        }
                    }

                    Item { height: 10 }
                }
            }
        }
    }

    // ── Data models ────────────────────────────────────────────────────
    ListModel { id: allowedCertsModel }
    ListModel { id: rejectedCertsModel }

    // ── Reusable cert info row (label + value) ─────────────────────────
    component CertInfoRow: RowLayout {
        property string label: ""
        property string value: "--"
        Layout.fillWidth: true
        spacing: 8

        Text {
            text: label
            color: "black"
            font.pixelSize: 11
            Layout.preferredWidth: 120
        }
        Text {
            text: value
            color: "gray"
            font.pixelSize: 11
        }
    }
}