import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: topicListDialog
    title: "Topics List"
    width: 400
    height: 600
    modal: true
    
    // Remove the standard close button and add custom buttons
    standardButtons: Dialog.NoButton

    property string selectedTopicName: ""

    Rectangle {
        anchors.fill: parent
        color: "white"

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            ComboBox {
                id: domainSelector
                model: domainModel
                textRole: "name"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    var domainId = domainModel.data(domainModel.index(currentIndex, 0), Qt.UserRole + 1)
                    topicListView.model = domainModel.subModelFromEntityId(domainId)
                }
            }

            ListView {
                id: topicListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                delegate: ItemDelegate {
                    width: parent.width
                    height: 40

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10

                        Text {
                            text: name  // This is the topic name
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "ID: " + id
                            color: "gray"
                        }
                    }

                    onClicked: {
                        console.log("Selected topic: " + name)
                        topicListDialog.selectedTopicName = name
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                Button {
                    text: "Cancel"
                    onClicked: topicListDialog.reject()
                }

                Button {
                    text: "OK"
                    enabled: topicListDialog.selectedTopicName !== ""
                    onClicked: {
                        console.log("Starting subscriber for topic: " + topicListDialog.selectedTopicName)
                        controller.startDynamicSubscriber(topicListDialog.selectedTopicName)
                        topicListDialog.accept()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (domainSelector.count > 0) {
            domainSelector.currentIndex = 0
        }
    }
}