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

                Text {
                    text: "Preferences regarding notifications"
                    color: "darkgray"
                    bottomPadding: 15
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    Image {
                        source: "qrc:/resources/images/icons/refresh_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/left_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/dots.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                }
            }
        }

        // ── Content Area ───────────────────────────────────────────────
        Rectangle {
            id: lowerRectangle
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            color: "#F2F2F2"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 6

                // ── Checkbox 1 ─────────────────────────────────────────
                CheckBox {
                    id: showGuideNotificationsCheck
                    Layout.fillWidth: true
                    checked: true

                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        x: showGuideNotificationsCheck.leftPadding
                        y: parent.height / 2 - height / 2
                        border.color: showGuideNotificationsCheck.checked
                                      ? "#DA3450" : "#34495e"
                        border.width: 1
                        radius: 3
                        color: showGuideNotificationsCheck.checked
                               ? "#DA3450" : "white"

                        Text {
                            anchors.centerIn: parent
                            text: showGuideNotificationsCheck.checked ? "✓" : ""
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }

                    contentItem: Text {
                        text: "Show guide notifications"
                        leftPadding: showGuideNotificationsCheck.indicator.width
                                     + showGuideNotificationsCheck.spacing + 4
                        verticalAlignment: Text.AlignVCenter
                        color: "black"
                        wrapMode: Text.WordWrap
                    }
                }

                // ── Checkbox 2 ─────────────────────────────────────────
                CheckBox {
                    id: notifyServiceConfigCheck
                    Layout.fillWidth: true
                    checked: true

                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        x: notifyServiceConfigCheck.leftPadding
                        y: parent.height / 2 - height / 2
                        border.color: notifyServiceConfigCheck.checked
                                      ? "#DA3450" : "#34495e"
                        border.width: 1
                        radius: 3
                        color: notifyServiceConfigCheck.checked
                               ? "#DA3450" : "white"

                        Text {
                            anchors.centerIn: parent
                            text: notifyServiceConfigCheck.checked ? "✓" : ""
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }

                    contentItem: Text {
                        text: "Notify when service configuration was successful"
                        leftPadding: notifyServiceConfigCheck.indicator.width
                                     + notifyServiceConfigCheck.spacing + 4
                        verticalAlignment: Text.AlignVCenter
                        color: "black"
                        wrapMode: Text.WordWrap
                    }
                }

                // ── Checkbox 3 ─────────────────────────────────────────
                CheckBox {
                    id: showWarningDialogCheck
                    Layout.fillWidth: true
                    checked: true

                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        x: showWarningDialogCheck.leftPadding
                        y: parent.height / 2 - height / 2
                        border.color: showWarningDialogCheck.checked
                                      ? "#DA3450" : "#34495e"
                        border.width: 1
                        radius: 3
                        color: showWarningDialogCheck.checked
                               ? "#DA3450" : "white"

                        Text {
                            anchors.centerIn: parent
                            text: showWarningDialogCheck.checked ? "✓" : ""
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }

                    contentItem: Text {
                        text: "Show warning dialog when editing DomainParticipant can affect to other Topics."
                        leftPadding: showWarningDialogCheck.indicator.width
                                     + showWarningDialogCheck.spacing + 4
                        verticalAlignment: Text.AlignVCenter
                        color: "black"
                        wrapMode: Text.WordWrap
                    }
                }

                // ── Vertical spacer ────────────────────────────────────
                Item { Layout.fillHeight: true }
            }
        }

        // ── Bottom Bar: Restore Defaults + Apply ───────────────────────
        Rectangle {
            id: bottomBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "white"
            border.color: "#E8E9EB"
            border.width: 1

            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16
                spacing: 8

                Button {
                    id: restoreDefaultsBtn
                    text: "Restore Defaults"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: restoreDefaultsBtn.text
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        showGuideNotificationsCheck.checked = true
                        notifyServiceConfigCheck.checked    = true
                        showWarningDialogCheck.checked      = true
                    }
                }

                Button {
                    id: applyBtn
                    text: "Apply"
                    implicitWidth: 80
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: applyBtn.text
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {}
                }
            }
        }
    }
}