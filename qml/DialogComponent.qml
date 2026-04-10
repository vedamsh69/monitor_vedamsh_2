import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: dialogforidl
    title: "Received Data"
    modal: true
    standardButtons: Dialog.Close

    contentItem: Column {
        spacing: 10

        Text {
            text: "Topic Name: " + topicIDLModel.topicname
            font.bold: true
        }

        TextArea {
            id: textArea
            width: parent.width
            height: 300
            text: topicIDLModel.textData
            wrapMode: Text.Wrap
            readOnly: true
        }
    }

    Component.onCompleted: {
        console.log("Topic Name from QML: " + topicIDLModel.topicname)
    }
}
    