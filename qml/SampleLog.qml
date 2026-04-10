import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "." as Local

Dialog {
    id: sampleLogWindow
    title: "Sample Log"
    width: 1200
    height: 800
    visible: false

    function open() {
        visible = true
    }

    function close() {
        visible = false
    }

    Component.onCompleted: {
        filterModel("")
    }

    ColumnLayout {
        anchors.fill: parent
        Layout.fillWidth: true
        anchors.top: parent.top
        spacing: 10
        anchors.margins: 10
        Layout.margins: 10

        TextField {
            Layout.fillWidth: true
            id: searchField
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignTop
            placeholderText: "type filter text"
            onTextChanged: {
                filterModel(text)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TableView {
                id: tableView
                anchors.fill: parent
                model: filteredModel

                TableViewColumn {
                    role: "index"
                    title: "Index"
                    width: 80
                    delegate: sampleDelegate
                }
                TableViewColumn {
                    role: "topic"
                    title: "Topic"
                    width: 200
                    delegate: sampleDelegate
                }
                TableViewColumn {
                    role: "instance"
                    title: "Instance"
                    width: 120
                    delegate: sampleDelegate
                }
                TableViewColumn {
                    role: "sourceTime"
                    title: "Source Time"
                    width: 310
                    delegate: sampleDelegate
                }
                TableViewColumn {
                    role: "selectedFields"
                    title: "Value of selected fields"
                    width: 460
                    delegate: sampleDelegate
                }
            }

            Text {
                anchors.centerIn: parent
                text: "Incompatible QOS"
                font.pixelSize: 20
                color: "gray"
                visible: filteredModel.count === 0
            }
        }
    }

    Component {
        id: sampleDelegate
        Rectangle {
            color: "transparent"
            Text {
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: 13
                text: styleData.value
                color: {
                    if (styleData.role === "topic") return "Red"
                    if (styleData.role === "instance") return "#DA3450"
                    return "black"
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (styleData.role === "topic" || styleData.role === "instance") {
                        //showRowDetails(styleData.row)
                    }
                    // Open Sample Inspector for any column click
                    sampleDialog.showSample(filteredModel.get(styleData.row).index)
                }
            }
        }
    }

    ListModel {
        id: sourceModel
    }

    ListModel {
        id: filteredModel
    }

    function filterModel(query) {
        filteredModel.clear()
        for (var i = 0; i < sourceModel.count; i++) {
            var item = sourceModel.get(i)
            if (query === "" || itemMatchesFilter(item, query)) {
                filteredModel.append(item)
            }
        }
    }

    function itemMatchesFilter(item, query) {
        query = query.toLowerCase()
        return item.index.toString().toLowerCase().indexOf(query) !== -1 ||
               item.topic.toLowerCase().indexOf(query) !== -1 ||
               item.instance.toLowerCase().indexOf(query) !== -1 ||
               item.sourceTime.toLowerCase().indexOf(query) !== -1 ||
               item.selectedFields.toLowerCase().indexOf(query) !== -1
    }

    SampleInspector {
        id : sampleDialog
    }

    function addSampleData(sampleData) {
        sourceModel.append({
            index: sourceModel.count,
            topic: topicIDLModel.topicname,
            instance: "NO Key",
            sourceTime: topicIDLModel.getSampleTimestamp(sourceModel.count),
            selectedFields: sampleData.selectedFields,
            sampleInfo: sampleData.sampleInfo,
            dynamicData: sampleData.dynamicData
        })
        filterModel(searchField.text)
        console.log("Selected fields are: " + sampleData.selectedFields)
    }

    Connections {
        target: topicIDLModel
        function onSampleAdded(sampleIndex, sampleData) {
            addSampleData({
                selectedFields: sampleData.selectedFields,
                sampleInfo: sampleData.sampleInfo,
                dynamicData: sampleData.dynamicData
            })
        }
    }
}