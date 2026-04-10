import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0

Dialog {
    id: createsubscriptiondialogid
    modal: true
    visible: false
    property int domainnumber: 0
    property string topicname: ""
    property string selectedTopic: ""
    
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 720

    property int defaultSpacing: 10
  
    Column {
        id: col
        anchors.fill: parent
        anchors.margins: defaultSpacing
        spacing: defaultSpacing
        property real cellwidth: col.width / 3 - spacing

        Item {
            width: parent.width
            height: 80

            Label {
                id: subscriptionlabel
                text: "Create Subscription"
                anchors.left: parent.left
            }

            Image {
                source: "qrc:/resources/images/icons/create_subscription_icon.png"
                width: 50
                height: 50
                anchors.right: parent.right
                anchors.top: parent.top
            }

            Label {
                text: "Subscribe to topic Domain " + createsubscriptiondialogid.domainnumber + ": " + createsubscriptiondialogid.topicname
                visible: true
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: 60
                width: parent.width - 60 // Adjust to prevent overlap with the image
                wrapMode: Text.WordWrap
            }
        }

        Rectangle {
            id: rec
            width: parent.width
            height: 1  // Height of the line
            color: "#D4D4D4"  // Color of the line
        }

        Column {
            id: col2
            spacing: defaultSpacing

            Label {
                text: "Data Type"
            }

            Grid {
                id: grid
                columns: 2
                spacing: defaultSpacing
                width: parent.width

                TextField {
                    id: textbox
                    height: 25
                    width: 480
                    placeholderText: "Filter Data Types"
                }

                CheckBox {
                    height: 25
                    text: "Hide irrelevant types"
                }
            }

            Grid {
                id: grid2
                columns: 2
                spacing: defaultSpacing
                width: parent.width

                ComboBox {
                    id: combobox
                    model: ["", createsubscriptiondialogid.topicname]
                    width: 500
                    height: 30
                    onCurrentIndexChanged: {
                        if (currentIndex !== -1) {
                            createsubscriptiondialogid.selectedTopic = model[currentIndex]
                        } else {
                            createsubscriptiondialogid.selectedTopic = ""
                        }
                    }
                }
            }

            Text {
                id: warningText
                text: "Please select a topic from the DropDown"
                color: "red"
                visible: createsubscriptiondialogid.selectedTopic === ""
            }

            Text {
                id: loaddatatypedialog
                color: "pink"
                anchors.left: parent.left
                text: "Load Data Types from XML files"

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: dialogwindow.open()
                }
            }

            Loaddatatypes {
                id: dialogwindow
            }
        }

        Column {
            AdvancedSettings {
                id: advancedsettingssection
            }
        }
    }


    footer: DialogButtonBox {
        Button {
            text: qsTr("OK")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: createsubscriptiondialogid.selectedTopic !== ""
        }
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    onAccepted: {
        console.log("Starting subscriber for topic: " + createsubscriptiondialogid.selectedTopic)
        controller.startDynamicSubscriber(createsubscriptiondialogid.selectedTopic)
    }

    onRejected: {
        console.log("Dialog cancelled")
    }
}