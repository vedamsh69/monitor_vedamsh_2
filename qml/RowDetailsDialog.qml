// RowDetailsDialog.qml
import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

Dialog {
    id: rowDetailsDialog
    title: "Row Details"
    width: 400
    height: 300
    standardButtons: StandardButton.Ok

    property var rowData: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text {
            text: "Index: " + (rowData ? rowData.index : "")
        }
        Text {
            text: "Topic: " + (rowData ? rowData.topic : "")
        }
        Text {
            text: "Instance: " + (rowData ? rowData.instance : "")
        }
        Text {
            text: "Source Time: " + (rowData ? rowData.sourceTime : "")
        }
        Text {
            text: "Selected Fields: " + (rowData ? rowData.selectedFields : "")
        }
        Text {
            text: "Host Name: " + (rowData
                                   && rowData.hostName ? rowData.hostName : "N/A")
        }
    }
}
