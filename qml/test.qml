import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 400
    height: 300

    Button {
        text: "Open Dialog"
        anchors.centerIn: parent
        onClicked: dialog.open()  // Show the dialog when clicked
    }

    Dialog {
        id: dialog
        title: "Received Data"
        modal: true
        standardButtons: Dialog.Close  // Close button

        contentItem: Column {
            spacing: 10

            TextArea {
                id: textArea
                width: parent.width
                height: 100
                text: topicIDLModel.textData  // Updates dynamically
                wrapMode: Text.Wrap
                readOnly: true  // Optional: Prevent user edits
            }
        }
    }
}





/*import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 400
    height: 300

    TextArea {
        id: textArea
        anchors.centerIn: parent
        width: 300
        height: 100
        text: topicData.textData  // Automatically updates when C++ data changes
        wrapMode: Text.Wrap
    }
}

*/