import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4 as QC1
import QtQuick.Controls.Styles 1.4

Dialog {
    id: sampleDialog
    title: "Sample Inspector"
    width: 1100
    height: 900 
    standardButtons: Dialog.Close

    onRejected: {
        sampleInspectorLoader.active = false
    }

    property var currentSample: null

    Rectangle {
        id: outerRectangle
        anchors.fill: parent
        color: "white"
        border.color: "gray"
        border.width: 1
        radius: 5

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            // Search Bar Section
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Layout.margins: 10

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 25
                    placeholderText: "Type filter text on field names"
                    onTextChanged: {
                        if (text.length > 0) {
                            searchTree(text)
                        } else {
                            resetTree()
                        }
                    }
                }
            }

            // Table View
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                color: "white"
                border.color: "lightgray"
                border.width: 1

                QC1.TableView {
                    id: tableView
                    anchors.fill: parent
                    model: filteredModel

                    QC1.TableViewColumn {
                        role: "field"
                        title: "Field"
                        width: sampleDialog.width * 0.320
                        delegate: Item {
                            height: 30

                            Row {
                                spacing: 5
                                x: 10 + model.depth * 20

                                Text {
                                    text: model.hasChildren ? (model.expanded ? "▼" : "▶") : ""
                                    font.pointSize: 10
                                    font.family: "Arial"
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: styleData.value || ""
                                    font.pointSize: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (model.hasChildren) {
                                        setExpanded(filteredModel, styleData.row, !model.expanded)
                                    }
                                }
                            }
                        }
                    }

                    QC1.TableViewColumn {
                        role: "value"
                        title: "Value"
                        width: sampleDialog.width * 0.320
                        delegate: Item {
                            height: 30

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 10
                                text: styleData.value || ""
                                font.pointSize: 12
                            }
                        }
                    }

                    QC1.TableViewColumn {
                        role: "type"
                        title: "Type"
                        width: sampleDialog.width * 0.320
                        delegate: Item {
                            height: 30

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 10
                                text: styleData.value || ""
                                font.pointSize: 12
                            }
                        }
                    }

                    style: TableViewStyle {
                        backgroundColor: "transparent"
                        alternateBackgroundColor: "transparent"
                        textColor: "black"
                        headerDelegate: Rectangle {
                            height: 30
                            color: "white"
                            border.color: "lightgray"
                            border.width: 1
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                text: styleData.value
                                font.bold: true
                            }
                        }
                        rowDelegate: Rectangle {
                            height: 30
                            color: "transparent"
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 1
                                color: "lightgray"
                            }
                        }
                    }
                }

                // Vertical grid lines
                Repeater {
                    model: 2 // Number of vertical lines (3 columns - 1)
                    Rectangle {
                        x: (index + 1) * (parent.width / 3)
                        width: 1
                        height: parent.height
                        color: "lightgray"
                    }
                }
            }
        }
    }

    ListModel {
        id: treeModel
    }

    ListModel {
        id: filteredModel
    }

        // Infer RTI-like type for SampleInfo fields
    function sampleInfoTypeFor(fieldName, value) {
        fieldName = fieldName.trim()
        value = value.trim()

        // 1) Timestamps
        if (fieldName.indexOf("timestamp") !== -1)
            return "Timestamp"

        // 2) GUIDs
        if (fieldName.indexOf("guid") !== -1 || fieldName.indexOf("GUID") !== -1)
            return "GUID"

        // 3) Sequence numbers
        if (fieldName.indexOf("sequence_number") !== -1)
            return "SequenceNumber_t"

        // 4) Flags
        if (fieldName === "flag")
            return "int"

        // 5) State / rank / counts – treat as int
        if (fieldName === "sample_state" ||
            fieldName === "view_state" ||
            fieldName === "instance_state" ||
            fieldName === "sample_rank" ||
            fieldName === "generation_rank" ||
            fieldName === "absolute_generation_rank" ||
            fieldName === "disposed_generation_count" ||
            fieldName === "no_writers_generation_count")
            return "int"

        // 6) Booleans
        if (value === "true" || value === "false")
            return "boolean"

        // 7) Fallback: numeric vs string
        if (!isNaN(Number(value)))
            return value.indexOf('.') !== -1 ? "double" : "int"

        return "string"
    }


    function populateTreeModel() {
        treeModel.clear()

// 1) Sample Data (to match RTI screenshot grouping)
var sampleDataElement = {
    field: "SampleData",
    value: "",
    type: "",
    expanded: false,
    hasChildren: true,
    depth: 0,
    children: []
}

var dynamicDataStr = currentSample.dynamicData
var dynamicDataLines = dynamicDataStr.split('\n')
for (var j = 0; j < dynamicDataLines.length; j++) {
    var dataLine = dynamicDataLines[j].trim()
    if (dataLine) {
        var dataParts = dataLine.split(':')
        if (dataParts.length === 2) {
            sampleDataElement.children.push({
                field: dataParts[0].trim(),
                value: dataParts[1].trim(),
                type: inferType(dataParts[1].trim()),
                depth: 1
            })
        }
    }
}
treeModel.append(sampleDataElement)

// 2) Sample Info
var sampleInfoElement = {
    field: "SampleInfo",
    value: "",
    type: "",
    expanded: false,
    hasChildren: true,
    depth: 0,
    children: []
}

var sampleInfoStr = currentSample.sampleInfo
var sampleInfoLines = sampleInfoStr.split('\n')
for (var i = 0; i < sampleInfoLines.length; i++) {
    var line = sampleInfoLines[i].trim()
    if (line) {
        var parts = line.split(':')
                        if (parts.length === 2) {
                    var fName = parts[0].trim()
                    var fValue = parts[1].trim()
                    sampleInfoElement.children.push({
                        field: fName,
                        value: fValue,
                        type: sampleInfoTypeFor(fName, fValue),
                        depth: 1
                    })
                }

    }
}
treeModel.append(sampleInfoElement)

resetTree()

    }

    function inferType(value) {
        if (!isNaN(Number(value))) {
            return value.indexOf('.') !== -1 ? "double" : "int"
        }
        return "string"
    }

    function searchTree(query) {
        filteredModel.clear()
        query = query.toLowerCase()

        for (var i = 0; i < treeModel.count; i++) {
            var item = treeModel.get(i)
            var matchFound = false

            if (item.field.toLowerCase().includes(query) ||
                item.value.toLowerCase().includes(query) ||
                item.type.toLowerCase().includes(query)) {
                matchFound = true
            }

            var children = []
            for (var j = 0; j < item.children.count; j++) {
                var child = item.children.get(j)
                if (child.field.toLowerCase().includes(query) ||
                    child.value.toLowerCase().includes(query) ||
                    child.type.toLowerCase().includes(query)) {
                    matchFound = true
                    children.push(child)
                }
            }

            if (matchFound) {
                filteredModel.append({
                    field: item.field,
                    value: item.value,
                    type: item.type,
                    expanded: true,
                    hasChildren: children.length > 0,
                    children: children,
                    depth: item.depth
                })

                if (children.length > 0) {
                    for (var k = 0; k < children.length; k++) {
                        filteredModel.append({
                            field: children[k].field,
                            value: children[k].value,
                            type: children[k].type,
                            expanded: false,
                            hasChildren: false,
                            depth: children[k].depth
                        })
                    }
                }
            }
        }
    }

    function resetTree() {
        filteredModel.clear()
        for (var i = 0; i < treeModel.count; i++) {
            var item = treeModel.get(i)
            filteredModel.append({
                field: item.field,
                value: item.value,
                type: item.type,
                expanded: item.expanded,
                hasChildren: item.hasChildren,
                children: item.children,
                depth: item.depth
            })
        }
    }

    function setExpanded(model, index, expanded) {
        var item = model.get(index)
        if (item.children && item.children.count > 0) {
            item.expanded = expanded
            if (expanded) {
                for (var i = 0; i < item.children.count; i++) {
                    var childIndex = index + i + 1
                    if (model.count <= childIndex || model.get(childIndex).field !== item.children.get(i).field) {
                        model.insert(childIndex, item.children.get(i))
                    }
                    if (item.children.get(i).expanded) {
                        setExpanded(model, childIndex, true)
                    }
                }
            } else {
                var removedCount = removeExpandedChildren(model, index + 1, item.depth)
                model.remove(index + 1, removedCount)
            }
        }
    }

    function removeExpandedChildren(model, startIndex, parentDepth) {
        var count = 0
        while (startIndex + count < model.count && model.get(startIndex + count).depth > parentDepth) {
            count++
        }
        return count
    }

    function showSample(sampleIndex) {
        currentSample = topicIDLModel.getSample(sampleIndex)
        populateTreeModel()
        open()
    }

    Component.onCompleted: {
        if (currentSample) {
            populateTreeModel()
        }
    }
}