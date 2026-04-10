import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15

Dialog {
    id: myDialog
    height: 1025
    width: 800
    title: "Preferences"

    property string selectedButton: ""
        property string domainInput: ""

            contentItem: Item {
                id: rootitem
                implicitHeight: mainColumn.implicitHeight

                ColumnLayout {
                    id: mainColumn
                    anchors.fill: parent
                    spacing: 0

                    Row {
                        id: row
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            id: rectangle
                            width: parent.width * 0.25
                            height: parent.height
                            Column {
                                spacing: 10
                                anchors.fill: parent
                                anchors.margins: 10

                                TextField {
                                    id: searchField
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    //font.pointSize: 8
                                    leftPadding: 18
                                    placeholderText: "type filter text"
                                    height: 30
                                    background: Rectangle {
                                        border.color: "#CBCBCB"
                                        radius: 5
                                    }
                                }

                                Button {
                                    id: administrationButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton
                                        === "administration" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Administration"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton
                                        === "administration" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "administration"
                                        rightLoader.source = "AdministrationTable.qml"
                                    }
                                }
                                Button {
                                    id: dataVisualisationButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton
                                        === "dataVisualization" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Data visualization"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton
                                        === "dataVisualization" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "dataVisualization"
                                        rightLoader.source = "DataVisualisationTable.qml"
                                    }
                                }
                                Button {
                                    id: healthButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "health" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Health"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton === "health" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "health"
                                        rightLoader.source = "HealthTable.qml"
                                    }
                                }
                                Button {
                                    id: loggingButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "logging" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Logging"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton === "logging" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "logging"
                                        rightLoader.source = "LoggingTable.qml"
                                    }
                                }
                                Button {
                                    id: pythonButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "python" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Python"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton === "python" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "python"
                                        rightLoader.source = "PythonTable.qml"
                                    }
                                }
                                Button {
                                    id: notificationButton
                                    width: parent.width
                                    height: 30

                                    background: Rectangle {
                                        color: myDialog.selectedButton === "notification" ? "#DA3450" : "white"
                                    }

                                    contentItem: Text {
                                        text: "Notifications"
                                        color: myDialog.selectedButton === "notification" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    onClicked: {
                                        myDialog.selectedButton = "notification"
                                        rightLoader.source = "NotificationsTable.qml"
                                    }
                                }
                                Button {
                                    id: securityButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "security" ? "#DA3450" : "white"
                                    }
                                    contentItem: Text {
                                        text: "Security"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton === "security" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        myDialog.selectedButton = "security"
                                        rightLoader.source = "SecurityTable.qml"
                                    }
                                }
                                Button {
                                    id: certificatesButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "certificates" ? "#DA3450" : "white"
                                    }
                                    contentItem: Text {
                                        text: "Certificates"
                                        //font.pointSize: 8
                                        color: myDialog.selectedButton === "certificates" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        myDialog.selectedButton = "certificates"
                                        rightLoader.source = "CertificatesTable.qml"
                                    }
                                }
                                Button {
                                    id: validationButton
                                    width: parent.width
                                    height: 30
                                    background: Rectangle {
                                        color: myDialog.selectedButton === "validation" ? "#DA3450" : "white"
                                    }
                                    contentItem: Text {
                                        text: "Validation"
                                        color: myDialog.selectedButton === "validation" ? "white" : "black"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        myDialog.selectedButton = "validation"
                                        rightLoader.source = "ValidationTable.qml"
                                    }
                                }
                            }
                        }
                        Rectangle {
                            id: rightRectangle
                            width: parent.width * 0.75
                            height: parent.height
                            color: "#F2F2F2"

                            Loader {
                                id: rightLoader
                                anchors.fill: parent
                                source: ""
                            }
                        }
                    }

                    // New rectangle for buttons
                    Rectangle {
                        id: buttonRectangle
                        Layout.fillWidth: true
                        height: 70
                        color: "white"
                        border.color: "#E8E9EB"
                        border.width: 0.5

                        RowLayout {
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.margins: 18
                            spacing: 5

                            Button {
                                text: "Cancel"
                                implicitWidth: 130
                                implicitHeight: 35

                                background: Rectangle {
                                    color: "white"
                                    radius: 5
                                    border.width: 0.5
                                    border.color: "#E8E9EB"
                                }

                                contentItem: Text {
                                    text: parent.text
                                    //font.pixelSize: 10
                                    color: "gray"
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                onClicked: myDialog.reject()
                            }

                            Button {
                                text: "Apply and Close"
                                implicitWidth: 130
                                implicitHeight: 35

                                background: Rectangle {
                                    color: "#249225"
                                    radius: 5
                                }

                                contentItem: Text {
                                    text: parent.text
                                    //font.pixelSize: 10
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                onClicked: {
                                    if (myDialog.domainInput !== "")
                                    {
                                        controller.init_monitor(parseInt(myDialog.domainInput))
                                    }
                                    myDialog.accept()
                                }
                            }
                        }
                    }
                }
            }

            // Remove the standard buttons
            standardButtons: Dialog.NoButton
        }
