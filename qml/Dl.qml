import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: dlarea
    title: "Distributed Logger"
    modal: true
    standardButtons: Dialog.Close
    width: 800
    height: 600

    onOpened: {
        dllogmodel.autoRefresh = true
    }

    onClosed: {
        dllogmodel.autoRefresh = false
    }

    contentItem: Rectangle {
        color: "#f0f0f0"
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Label {
                Layout.fillWidth: true
                text: "Distributed Logger Subscriber"
                font.bold: true
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Start Subscriber"
                highlighted: true
                onClicked: {
                    controller.startDynamicDLSubscriber()
                }
            }

ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: dllogmodel
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 250
                    color: index % 2 === 0 ? "#e0e0e0" : "#f0f0f0"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        Text {
                            text: "<b>ID:</b> " + model.id
                            font.pixelSize: 12
                        }

                        Text {
                            text: "<b>Host ID:</b> " + model.hostId
                            font.pixelSize: 12
                        }

                        Text {
                            text: "<b>Process:</b> " + model.process // Updated to match new column
                            font.pixelSize: 12
                        }

                        Text {
                            text: "<b>Message:</b> " + model.message
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "<b>Timestamp:</b> " + model.timestamp // Updated to use the formatted timestamp directly
                            font.pixelSize: 12
                        }

                        Text {
                            text: "<b>File:</b> " + model.filename + " | <b>Line:</b> " + model.line + " | <b>Function:</b> " + model.function;
                            font.pixelSize: 12
                        }

                        Text {
                            text: "<b>Category:</b> " + model.category + " | <b>Kind:</b> " + model.kind
                            font.pixelSize: 12
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Text {
                    text: "Auto-refreshing every second"
                    font.pixelSize: 14
                    font.italic: true
                }

                Text {
                    text: "Total Logs: " + dllogmodel.logCount
                    font.pixelSize: 14
                    font.bold: true
                }
            }
        }
    }
}

