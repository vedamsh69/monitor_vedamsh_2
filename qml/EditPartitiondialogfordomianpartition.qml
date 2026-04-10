import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Dialog {
    id: editpartitionfordomainparticipant
    title: "Edit Partition"
    height: 320
    width: 250
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    // Center the dialog in the parent item (qosandcontentfilter)
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    // Ensure the dialog stays centered when the parent is resized
    Connections {
        target: parent
        function onWidthChanged() { updatePosition() }
        function onHeightChanged() { updatePosition() }
    }

    function updatePosition() {
        x = (parent.width - width) / 2
        y = (parent.height - height) / 2
    }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                border.color: "black"
                border.width: 1


                ListView {
                    id: contentDisplayBox2
                    anchors.fill: parent
                    anchors.margins: 5
                    model: ListModel {}
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 40
                        text: model.content
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            contentDisplayBox2.currentIndex = index
                        }
                    }
                    focus: true
                    clip: true
                }
            }


            ColumnLayout {
                spacing : 10

                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 4

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/resources/images/icons/plus.png"
                        width: 15
                        height: 15
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked:
                        {
                            addDialog.open()
                        }
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 4

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/resources/images/icons/pencil.png"
                        width: 24
                        height: 24
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: contentDisplayBox2.currentIndex !== -1
                        onClicked: {
                            editDialogfordomainparticipant.content = contentDisplayBox2.model.get(contentDisplayBox2.currentIndex).content
                            editDialogfordomainparticipant.open()
                        }
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 4

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/resources/images/icons/uparrow.png"
                        width: 15
                        height: 15
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: contentDisplayBox2.currentIndex > 0
                        onClicked: {
                            contentDisplayBox2.model.move(contentDisplayBox2.currentIndex, contentDisplayBox2.currentIndex - 1, 1)
                            contentDisplayBox2.currentIndex--
                        }
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 4

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/resources/images/icons/downarrow.png"
                        width: 15
                        height: 15
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: contentDisplayBox2.currentIndex !== -1 && contentDisplayBox2.currentIndex < contentDisplayBox2.count - 1
                        onClicked: {
                            contentDisplayBox2.model.move(contentDisplayBox2.currentIndex, contentDisplayBox2.currentIndex + 1, 1)
                            contentDisplayBox2.currentIndex++
                        }
                    }
                }
                Rectangle {
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 4

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/resources/images/icons/close.png"
                        width: 15
                        height: 15
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: contentDisplayBox2.currentIndex !== -1
                        onClicked: {
                            contentDisplayBox2.model.remove(contentDisplayBox2.currentIndex)
                        }
                    }
                }
            }
        }


        onAccepted: {
            var selectedItems = []
            for (var i = 0; i < contentDisplayBox2.model.count; i++) {
                selectedItems.push(contentDisplayBox2.model.get(i).content)
            }
            dppartitiontextfield.text = selectedItems.join(", ")
        }



        Dialog {
            id: addDialog
            height: 200
            width: 450
            title: "Add"
            standardButtons: Dialog.Ok | Dialog.Cancel

            // Center the dialog on the screen
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            // Make sure the dialog is a child of the application window
            parent: Overlay.overlay

            modal: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 5

                Label
                {
                    text : "Add Partition"
                }

                TextField {
                    id: addTextField
                    Layout.fillWidth: true
                    placeholderText: "Enter partition name"
                }
            }

            onAccepted: {
                if (addTextField.text.trim() !== "") {
                    contentDisplayBox2.model.append({content: addTextField.text.trim()})
                    addTextField.text = ""
                }
            }
        }





        Dialog {
            id: editDialogfordomainparticipant
            height: 200
            width: 450
            title: "Edit"
            standardButtons: Dialog.Ok | Dialog.Cancel

            property string content: ""


            // Center the dialog on the screen
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            // Make sure the dialog is a child of the application window
            parent: Overlay.overlay

            modal: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 5

                Label
                {
                    text : "Add Partition"
                }

                TextField {
                    id: editTextField
                    Layout.fillWidth: true
                    text: editDialogfordomainparticipant.content
                }
            }

            onAccepted: {
                if (editTextField.text.trim() !== "") {
                    contentDisplayBox2.model.set(contentDisplayBox2.currentIndex, {content: editTextField.text.trim()})
                }
            }
        }
}







