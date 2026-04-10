import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4 as QC1

Dialog {
    id: typetreedialog
    title: "Data Type Details"
    width: 1300
    height: 500
    standardButtons: Dialog.NoButton

    property string serializationMode: "XCDR"

    contentItem: Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 5

            // ========== TOOLBAR ==========
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 45
                color: "#f5f5f5"
                border.color: "#cccccc"
                border.width: 1
                radius: 3

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 8

                    // XCDR/XCDR2 buttons
                    Label {
                        text: "Serialization:"
                        visible: tabView.currentIndex === 0
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Button {
                        id: xcdrButton
                        text: "XCDR"
                        Layout.preferredWidth: 70
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 0
                        checkable: true
                        checked: serializationMode === "XCDR"
                        
                        background: Rectangle {
                            color: xcdrButton.checked ? "#4CAF50" : "#e0e0e0"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }
                        
                        contentItem: Text {
                            text: xcdrButton.text
                            color: xcdrButton.checked ? "white" : "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 11
                            font.bold: xcdrButton.checked
                        }

                        onClicked: {
                            serializationMode = "XCDR"
                            xcdr2Button.checked = false
                            console.log("[TypeTreeView] ✓ Mode: XCDR")
                        }
                    }

                    Button {
                        id: xcdr2Button
                        text: "XCDR2"
                        Layout.preferredWidth: 70
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 0
                        checkable: true
                        checked: false
                        
                        background: Rectangle {
                            color: xcdr2Button.checked ? "#4CAF50" : "#e0e0e0"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }
                        
                        contentItem: Text {
                            text: xcdr2Button.text
                            color: xcdr2Button.checked ? "white" : "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 11
                            font.bold: xcdr2Button.checked
                        }

                        onClicked: {
                            serializationMode = "XCDR2"
                            xcdrButton.checked = false
                            console.log("[TypeTreeView] ✓ Mode: XCDR2")
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 2
                        Layout.preferredHeight: 30
                        color: "#cccccc"
                        visible: tabView.currentIndex === 0
                    }

                    // Export CSV
                    Button {
                        text: "📊 Export CSV"
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 0
                        
                        background: Rectangle {
                            color: parent.pressed ? "#c0c0c0" : "#e8e8e8"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            console.log("[TypeTreeView] ✓ Export CSV clicked")
                            csvFileDialog.open()
                        }
                    }

                    // Expand All
                    Button {
                        text: "⊞ Expand All"
                        Layout.preferredWidth: 110
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 0
                        
                        background: Rectangle {
                            color: parent.pressed ? "#c0c0c0" : "#e8e8e8"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            console.log("[TypeTreeView] ✓ Expand All clicked")
                            typetreetable.expandAll()
                        }
                    }

                    // Collapse All
                    Button {
                        text: "⊟ Collapse All"
                        Layout.preferredWidth: 115
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 0
                        
                        background: Rectangle {
                            color: parent.pressed ? "#c0c0c0" : "#e8e8e8"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            console.log("[TypeTreeView] ✓ Collapse All clicked")
                            typetreetable.collapseAll()
                        }
                    }

                    // Export IDL
                    Button {
                        text: "📄 Export IDL"
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        visible: tabView.currentIndex === 1
                        
                        background: Rectangle {
                            color: parent.pressed ? "#c0c0c0" : "#e8e8e8"
                            border.color: "#999999"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            console.log("[TypeTreeView] ✓ Export IDL clicked")
                            idlFileDialog.open()
                        }
                    }

                    Item { Layout.fillWidth: true }

                    CheckBox {
                        text: "🔗 Link with selection"
                        checked: true
                        Layout.preferredHeight: 32
                        
                        onCheckedChanged: {
                            console.log("[TypeTreeView] Link:", checked)
                        }
                    }
                }
            }

            // ========== TAB VIEW ==========
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                border.width: 1
                border.color: "#cccccc"
                radius: 5

                QC1.TabView {
                    id: tabView
                    anchors.fill: parent
                    anchors.margins: 1

                    QC1.Tab {
                        title: "Type Tree"
                        Rectangle {
                            anchors.fill: parent
                            color: "white"
                            TypeTreeViewTable {
                                id: typetreetable
                                anchors.fill: parent
                            }
                        }
                    }

                    QC1.Tab {
                        title: "Equivalent IDL"
                        Rectangle {
                            anchors.fill: parent
                            color: "white"
                            EquivalentIDlView {
                                id: equivalentidl
                                anchors.fill: parent
                            }
                        }
                    }
                }
            }

            // OK Button
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                Button {
                    text: "OK"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: parent.pressed ? "#00DD00" : "#00FF00"
                        radius: 5
                        border.color: "#00AA00"
                        border.width: 2
                    }

                    onClicked: typetreedialog.accept()
                }
            }
        }
    }

    // ========== CSV EXPORT DIALOG ==========
    FileDialog {
        id: csvFileDialog
        title: "Export Type Tree to CSV"
        selectExisting: false
        nameFilters: ["CSV files (*.csv)"]
        defaultSuffix: "csv"
        
        onAccepted: {
            var path = csvFileDialog.fileUrl.toString()
            path = path.replace(/^file:\/\//, "")
            
            console.log("[TypeTreeView] Saving CSV to:", path)
            var csvContent = topicIDLModel.generateCSVFromTree()
            
            if (topicIDLModel.saveTextToFile(path, csvContent)) {
                console.log("[TypeTreeView] ✓✓✓ CSV EXPORTED SUCCESSFULLY! ✓✓✓")
            } else {
                console.log("[TypeTreeView] ✗ CSV export failed")
            }
        }
    }

    // ========== IDL EXPORT DIALOG ==========
    FileDialog {
        id: idlFileDialog
        title: "Export IDL to File"
        selectExisting: false
        nameFilters: ["IDL files (*.idl)", "Text files (*.txt)"]
        defaultSuffix: "idl"
        
        onAccepted: {
            var path = idlFileDialog.fileUrl.toString()
            path = path.replace(/^file:\/\//, "")
            
            console.log("[TypeTreeView] Saving IDL to:", path)
            var idlText = topicIDLModel.textData
            
            if (topicIDLModel.saveTextToFile(path, idlText)) {
                console.log("[TypeTreeView] ✓✓✓ IDL EXPORTED SUCCESSFULLY! ✓✓✓")
            } else {
                console.log("[TypeTreeView] ✗ IDL export failed")
            }
        }
    }
}
