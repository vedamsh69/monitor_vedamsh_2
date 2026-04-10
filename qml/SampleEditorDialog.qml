// Copyright 2025 CDAC
import QtQuick 2.6
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Dialog {
    id: sampleEditorDialog
    title: "Edit Sample"
    visible: false
    
    width: 600
    height: 400
    
    property var sampleData: ({})
    
    signal sampleEdited(var sampleData)
    
    contentItem: Rectangle {
        color: "white"
        
        ScrollView {
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            
            ColumnLayout {
                id: fieldsLayout
                width: parent.width - 20
                spacing: 10
                
                Repeater {
                    id: fieldRepeater
                    model: Object.keys(sampleEditorDialog.sampleData)
                    
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: modelData + ":"
                            Layout.preferredWidth: 150
                            font.bold: true
                        }
                        
                        TextField {
                            id: fieldInput
                            Layout.fillWidth: true
                            text: sampleEditorDialog.sampleData[modelData] || ""
                            placeholderText: "Enter " + modelData
                            
                            property string fieldName: modelData
                        }
                    }
                }
                
                // Add field button
                Button {
                    text: "Add Field"
                    Layout.alignment: Qt.AlignRight
                    onClicked: addFieldDialog.visible = true
                }
            }
        }
    }
    
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    
    onAccepted: {
        var editedData = {}
        
        // Collect all field values
        for (var i = 0; i < fieldRepeater.count; i++) {
            var item = fieldRepeater.itemAt(i)
            if (item && item.children[1]) {
                var textField = item.children[1]
                var fieldName = textField.fieldName
                editedData[fieldName] = textField.text
            }
        }
        
        sampleEdited(editedData)
    }
    
    Dialog {
        id: addFieldDialog
        title: "Add New Field"
        visible: false
        
        width: 400
        height: 200
        
        contentItem: Rectangle {
            color: "white"
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                
                Label {
                    text: "Field Name:"
                }
                TextField {
                    id: newFieldName
                    Layout.fillWidth: true
                    placeholderText: "Enter field name"
                }
                Label {
                    text: "Field Value:"
                }
                TextField {
                    id: newFieldValue
                    Layout.fillWidth: true
                    placeholderText: "Enter field value"
                }
            }
        }
        
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        
        onAccepted: {
            if (newFieldName.text !== "") {
                sampleEditorDialog.sampleData[newFieldName.text] = newFieldValue.text
                newFieldName.text = ""
                newFieldValue.text = ""
                // Refresh the repeater
                fieldRepeater.model = Object.keys(sampleEditorDialog.sampleData)
            }
        }
    }
    
    function loadSample(data) {
        sampleData = data
        fieldRepeater.model = Object.keys(data)
    }
}
