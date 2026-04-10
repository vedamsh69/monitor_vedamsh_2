import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import Theme 1.0

Dialog {
    id: networkInfoDialog
    title: "System OverView Panel Table"
    standardButtons: Dialog.Close

    width: 800
    height: 500

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: "DDS Network Information"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        TopicsTable {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: "Refresh"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                console.log("Refreshed data")
            }
        }
    }

    function open() {
        visible = true
    }
}