import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4 as QC1

Item {
    id: tableviewid
    anchors.fill: parent

    property bool isPlaying: false

    // Create a ListModel to act as an intermediary
    ListModel {
        id: intermediateModel
    }

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            id: tableContainer
            Layout.fillWidth: true
            //Layout.preferredHeight: 500
            Layout.fillHeight: true
            color: "white"
            border.color: "#EBECF0"
            border.width: 1

            QC1.TableView {
                id: tableView
                anchors.fill: parent
                anchors.margins: 1
                model: intermediateModel

                QC1.TableViewColumn { role: "hostId"; title: "Host"; width: 150 }
                QC1.TableViewColumn { role: "process"; title: "Process"; width: 150 }
                QC1.TableViewColumn { role: "timestamp"; title: "Timestamp"; width: 280 }
                QC1.TableViewColumn { role: "kind"; title: "Level"; width: 100 }
                QC1.TableViewColumn { role: "category"; title: "Category"; width: 150 }
                QC1.TableViewColumn { role: "message"; title: "Message"; width: 300 }

                headerDelegate: Rectangle {
                    color: "white"
                    height: textItem.implicitHeight * 1.2
                    width: textItem.implicitWidth
                    border.color: "#EBECF0"
                    Text {
                        id: textItem
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: styleData.value
                        elide: Text.ElideRight
                        color: "black"
                        renderType: Text.NativeRendering
                    }
                }

                rowDelegate: Rectangle {
                    height: 30
                    color: {
                        var rowData = intermediateModel.get(styleData.row);
                        var kindValue = rowData ? rowData.kind : "";
                        if (kindValue === "DL_Error")   return "#FFCDD2"  // soft red
                        if (kindValue === "DL_Warning") return "#FFF9C4"  // soft yellow
                        if (kindValue === "DL_Info")    return "#BBDEFB"  // soft blue
                        if (kindValue === "DL_Notice")  return "#C8E6C9"  // soft green
                        if (kindValue === "DL_Debug")   return "#D1C4E9"  // soft purple
                        return "white";
                    }
                }

                itemDelegate: Item {
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                        text: styleData.value
                        color: "black"
                        font.pixelSize: 13
                    }
                }
            }
        }
    }

    // Timer to update the intermediateModel
    Timer {
        interval: 1000 // 1 second
        running: true
        repeat: true
        onTriggered: {
            if (isPlaying) {
                updateIntermediateModel()
            }
        }
    }

    // Function to update the intermediateModel from dllogmodel
    function updateIntermediateModel() {
        // Clear the current intermediateModel
        intermediateModel.clear()

        // Populate intermediateModel with data from dllogmodel
        for (var i = 0; i < dllogmodel.rowCount(); i++) {
            var logEntry = dllogmodel.getLog(i)
            intermediateModel.append({
                "hostId": logEntry.hostId,
                "process": logEntry.process,
                "timestamp": logEntry.timestamp,
                "kind": logEntry.kind,
                "category": logEntry.category,
                "message": logEntry.message
            })
        }
    }

    // Initial population of the intermediateModel
    Component.onCompleted: {
        updateIntermediateModel()
    }
}



