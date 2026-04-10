import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4 as QC1
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.11

Item {
    id: typetreetable
    width: 1300

    ListModel {
        id: treeModel
    }
    
    Connections {
        target: topicIDLModel
        function onTreeDataChanged() {
            console.log("[TypeTreeViewTable] Tree data changed signal received")
            loadTreeData()
        }
    }
    
    Component.onCompleted: {
        console.log("[TypeTreeViewTable] Component completed")
        loadTreeData()
    }
    
    function loadTreeData() {
        console.log("[TypeTreeViewTable] Loading tree data...")
        treeModel.clear()
        
        var treeData = topicIDLModel.treeData
        console.log("[TypeTreeViewTable] Tree data items:", treeData.length)
        
        if (treeData && treeData.length > 0) {
            for (var i = 0; i < treeData.length; i++) {
                console.log("[TypeTreeViewTable] Adding root:", treeData[i].name)
                treeModel.append(treeData[i])
            }
        } else {
            console.log("[TypeTreeViewTable] No tree data available")
            treeModel.append({
                name: "No type data available",
                typeOrdinal: "Subscribe to a topic to view structure",
                extensibility: "",
                optional: "",
                minSample: "",
                maxSample: "",
                typeCode: "",
                typeObject: "",
                unionLabels: "",
                depth: 0,
                expanded: false,
                children: []
            })
        }
        
        if (tableView && tableView.forceLayout) {
            tableView.forceLayout()
        }
    }

    // ========== EXPAND ALL FUNCTION ==========
    function expandAll() {
        console.log("[TypeTreeViewTable] ========== EXPAND ALL ==========")
        var expandedCount = 0
        
        // Expand all nodes that have children
        for (var i = 0; i < treeModel.count; i++) {
            var item = treeModel.get(i)
            if (item.children && item.children.count > 0 && !item.expanded) {
                console.log("[TypeTreeViewTable] Expanding:", item.name, "at index", i)
                setExpanded(treeModel, i, true)
                expandedCount++
            }
        }
        
        console.log("[TypeTreeViewTable] ✓ Expanded", expandedCount, "nodes")
        if (tableView && tableView.forceLayout) {
            tableView.forceLayout()
        }
    }

    // ========== COLLAPSE ALL FUNCTION ==========
    function collapseAll() {
        console.log("[TypeTreeViewTable] ========== COLLAPSE ALL ==========")
        var collapsedCount = 0
        
        // Collapse from end to beginning to avoid index issues
        for (var i = treeModel.count - 1; i >= 0; i--) {
            var item = treeModel.get(i)
            if (item.depth === 0 && item.expanded) {
                console.log("[TypeTreeViewTable] Collapsing:", item.name, "at index", i)
                setExpanded(treeModel, i, false)
                collapsedCount++
            }
        }
        
        console.log("[TypeTreeViewTable] ✓ Collapsed", collapsedCount, "root nodes")
        if (tableView && tableView.forceLayout) {
            tableView.forceLayout()
        }
    }

    // ========== GENERATE CSV FUNCTION ==========
    function generateCSV() {
        console.log("[TypeTreeViewTable] ========== GENERATING CSV ==========")
        
        var csv = "Name,Type/Ordinal,Extensibility,Optional,Min Sample,Max Sample,Type Code,Type Object,Union Labels\n"
        var rowCount = 0
        
        for (var i = 0; i < treeModel.count; i++) {
            var item = treeModel.get(i)
            
            // Add indentation for hierarchy
            var indent = ""
            for (var d = 0; d < item.depth; d++) {
                indent += "  "
            }
            
            csv += "\"" + indent + item.name + "\","
            csv += "\"" + item.typeOrdinal + "\","
            csv += "\"" + item.extensibility + "\","
            csv += "\"" + item.optional + "\","
            csv += "\"" + item.minSample + "\","
            csv += "\"" + item.maxSample + "\","
            csv += "\"" + item.typeCode + "\","
            csv += "\"" + item.typeObject + "\","
            csv += "\"" + item.unionLabels + "\"\n"
            
            rowCount++
        }
        
        console.log("[TypeTreeViewTable] ✓ Generated CSV with", rowCount, "rows")
        return csv
    }

    // ========== FILE SAVE FUNCTION ==========
    function saveToFile(filePath, content) {
        console.log("[TypeTreeViewTable] ========== SAVING FILE ==========")
        console.log("[TypeTreeViewTable] File path:", filePath)
        console.log("[TypeTreeViewTable] Content length:", content.length, "bytes")
        console.log("[TypeTreeViewTable] ========== FILE CONTENT ==========")
        console.log(content)
        console.log("[TypeTreeViewTable] ========== END ==========")
        
        // Return true to indicate completion
        // Actual file writing would need C++ backend
        return true
    }

    QC1.TableView {
        id: tableView
        anchors.fill: parent
        model: treeModel
        alternatingRowColors: true
        
        style: TableViewStyle {
            alternateBackgroundColor: "#f9f9f9"
            backgroundColor: "white"
            
            headerDelegate: Rectangle {
                height: 30
                color: "#e8e8e8"
                border.color: "#cccccc"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: styleData.value
                    font.bold: true
                    font.pixelSize: 11
                    color: "#333333"
                }
            }
        }

        QC1.TableViewColumn {
            role: "name"
            title: "Name"
            width: typetreetable.width * 0.15
            delegate: Item {
                property int indent: 20
                width: parent.width
                height: 28

                Row {
                    spacing: 5
                    x: {
                        if (styleData.row >= 0 && styleData.row < treeModel.count) {
                            return treeModel.get(styleData.row).depth * indent
                        }
                        return 0
                    }

                    Text {
                        text: {
                            if (styleData.row >= 0 && styleData.row < treeModel.count) {
                                var item = treeModel.get(styleData.row)
                                if (item.children && item.children.count > 0) {
                                    return item.expanded ? "▼" : "▶"
                                }
                            }
                            return ""
                        }
                        font.pointSize: 9
                        font.family: "DejaVu Sans Mono"
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#2196F3"
                        font.bold: true
                    }

                    Text {
                        text: styleData.value || ""
                        font.pixelSize: 11
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#000000"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (styleData.row >= 0 && styleData.row < treeModel.count) {
                            var item = treeModel.get(styleData.row)
                            if (item.children && item.children.count > 0) {
                                console.log("[TypeTreeViewTable] Toggling:", item.name)
                                setExpanded(treeModel, styleData.row, !item.expanded)
                                if (tableView && tableView.forceLayout) {
                                    tableView.forceLayout()
                                }
                            }
                        }
                    }
                }
            }
        }

        QC1.TableViewColumn { role: "typeOrdinal"; title: "Type/Ordinal"; width: typetreetable.width * 0.12 }
        QC1.TableViewColumn { role: "extensibility"; title: "Extensibility"; width: typetreetable.width * 0.12 }
        QC1.TableViewColumn { role: "optional"; title: "Optional?"; width: typetreetable.width * 0.09 }
        QC1.TableViewColumn { role: "minSample"; title: "Min Sample"; width: typetreetable.width * 0.10 }
        QC1.TableViewColumn { role: "maxSample"; title: "Max Sample"; width: typetreetable.width * 0.10 }
        QC1.TableViewColumn { role: "typeCode"; title: "Type Code"; width: typetreetable.width * 0.09 }
        QC1.TableViewColumn { role: "typeObject"; title: "Type Object"; width: typetreetable.width * 0.11 }
        QC1.TableViewColumn { role: "unionLabels"; title: "Union Labels"; width: typetreetable.width * 0.12 }
    }

    function setExpanded(model, index, expanded) {
        var item = model.get(index)
        if (item.children && item.children.count > 0) {
            item.expanded = expanded
            if (expanded) {
                for (var i = 0; i < item.children.count; i++) {
                    var childIndex = index + i + 1
                    if (model.count <= childIndex || model.get(childIndex).name !== item.children.get(i).name) {
                        model.insert(childIndex, item.children.get(i))
                    }
                    if (item.children.get(i).expanded) {
                        setExpanded(model, childIndex, true)
                    }
                }
            } else {
                var removedCount = removeExpandedChildren(model, index + 1, item.depth)
                if (removedCount > 0) {
                    model.remove(index + 1, removedCount)
                }
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
}
