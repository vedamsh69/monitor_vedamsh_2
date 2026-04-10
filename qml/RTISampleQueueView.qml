// Copyright 2025 CDAC - RTI Sample Queue View
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

Rectangle {
    id: rtiSampleQueueView
    color: "#FFFFFF"
    
    property var publicationManager: null
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5
        
        // Toolbar
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#F5F5F5"
            border.color: "#D0D0D0"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                spacing: 5
                
                Button {
                    text: "Edit"
                    enabled: sampleTable.currentRow >= 0
                    onClicked: editSample()
                    icon.source: "qrc:/resources/images/icons/preferences/pencil.png"
                }
                
                Button {
                    text: "Remove"
                    enabled: sampleTable.currentRow >= 0
                    onClicked: removeSample()
                    icon.source: "qrc:/resources/images/icons/cross/cross_red.svg"
                }
                
                Button {
                    text: "Clear All"
                    onClicked: clearAllSamples()
                    icon.source: "qrc:/resources/images/icons/preferences/broom.png"
                }
                
                Rectangle {
                    width: 1
                    height: 25
                    color: "#C0C0C0"
                }
                
                Button {
                    text: "Load"
                    onClicked: loadSamplesDialog.open()
                    icon.source: "qrc:/resources/images/icons/plus/plus_black.svg"
                }
                
                Button {
                    text: "Save"
                    onClicked: saveSamplesDialog.open()
                    icon.source: "qrc:/resources/images/icons/tick/tick_black.svg"
                }
                
                Item { Layout.fillWidth: true }
            }
        }
        
        // Sample table (RTI Style)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FFFFFF"
            border.color: "#C0C0C0"
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Table header
                Rectangle {
                    Layout.fillWidth: true
                    height: 30
                    color: "#E8E8E8"
                    border.color: "#C0C0C0"
                    border.width: 1
                    
                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        
                        Label {
                            text: "Sample"
                            font.bold: true
                            font.pixelSize: 11
                            Layout.preferredWidth: 80
                            Layout.leftMargin: 10
                        }
                        
                        Rectangle {
                            width: 1
                            height: 30
                            color: "#C0C0C0"
                        }
                        
                        Label {
                            text: "Actions"
                            font.bold: true
                            font.pixelSize: 11
                            Layout.preferredWidth: 100
                            Layout.leftMargin: 10
                        }
                        
                        Rectangle {
                            width: 1
                            height: 30
                            color: "#C0C0C0"
                        }
                        
                        Label {
                            text: "Sample content"
                            font.bold: true
                            font.pixelSize: 11
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                        }
                    }
                }
                
                // Table content
                TableView {
                    id: sampleTable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    
                    property int currentRow: -1
                    
                    model: sampleListModel
                    
                    delegate: Rectangle {
                        implicitWidth: sampleTable.width
                        implicitHeight: 35
                        color: sampleTable.currentRow === row ? "#E3F2FD" : (row % 2 === 0 ? "#FFFFFF" : "#F9F9F9")
                        border.color: "#E0E0E0"
                        border.width: 0.5
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: sampleTable.currentRow = row
                        }
                        
                        RowLayout {
                            anchors.fill: parent
                            spacing: 0
                            
                            Label {
                                text: model.index
                                font.pixelSize: 10
                                Layout.preferredWidth: 80
                                Layout.leftMargin: 10
                            }
                            
                            Rectangle {
                                width: 1
                                height: 35
                                color: "#E0E0E0"
                            }
                            
                            RowLayout {
                                Layout.preferredWidth: 100
                                Layout.leftMargin: 5
                                spacing: 3
                                
                                Button {
                                    text: "▶"
                                    width: 25
                                    height: 25
                                    font.pixelSize: 10
                                    onClicked: {
                                        if (publicationManager) {
                                            publicationManager.publishSample(model.index)
                                        }
                                    }
                                }
                                
                                Button {
                                    text: "✓"
                                    width: 25
                                    height: 25
                                    font.pixelSize: 10
                                }
                            }
                            
                            Rectangle {
                                width: 1
                                height: 35
                                color: "#E0E0E0"
                            }
                            
                            Label {
                                text: JSON.stringify(model.data || {})
                                font.pixelSize: 10
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                            }
                        }
                    }
                }
                
                // Empty state message
                Label {
                    text: "No samples available for publishing"
                    font.italic: true
                    font.pixelSize: 11
                    color: "#808080"
                    visible: sampleListModel.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
        
        // Bottom control bar
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#F5F5F5"
            border.color: "#C0C0C0"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10
                
                Label {
                    text: "Publication period (ms):"
                    font.pixelSize: 11
                }
                
                Button {
                    text: "<"
                    width: 30
                    height: 30
                    onClicked: {
                        if (periodSpinBox.value > 100) {
                            periodSpinBox.value -= 100
                        }
                    }
                }
                
                SpinBox {
                    id: periodSpinBox
                    from: 100
                    to: 10000
                    value: 700
                    stepSize: 100
                    editable: true
                    font.pixelSize: 11
                }
                
                Button {
                    text: ">"
                    width: 30
                    height: 30
                    onClicked: {
                        if (periodSpinBox.value < 10000) {
                            periodSpinBox.value += 100
                        }
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Publish All"
                    highlighted: true
                    font.pixelSize: 11
                    onClicked: {
                        if (publicationManager) {
                            publicationManager.publishAllSamples()
                        }
                    }
                }
            }
        }
    }
    
    // Data model
    ListModel {
        id: sampleListModel
    }
    
    // Dialogs
    FileDialog {
        id: loadSamplesDialog
        title: "Load Samples"
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: {
            if (publicationManager) {
                publicationManager.loadSamplesFromFile(fileUrl)
            }
        }
    }
    
    FileDialog {
        id: saveSamplesDialog
        title: "Save Samples"
        selectExisting: false
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: {
            if (publicationManager) {
                publicationManager.saveSamplesToFile(fileUrl)
            }
        }
    }
    
    SampleEditorDialog {
        id: sampleEditorDialog
        onSampleEdited: function(sampleData) {
            if (publicationManager && sampleTable.currentRow >= 0) {
                var samples = publicationManager.getAllSamples()
                if (sampleTable.currentRow < samples.length) {
                    var currentSample = samples[sampleTable.currentRow]
                    publicationManager.updateSample(currentSample.index, sampleData)
                }
            }
        }
    }
    
    // Update list when publicationManager changes
    Connections {
        target: publicationManager
        function onSampleQueueSizeChanged() {
            updateSampleList()
        }
    }
    
    // Functions
    function updateSampleList() {
        if (!publicationManager) return
        
        sampleListModel.clear()
        var samples = publicationManager.getAllSamples()
        for (var i = 0; i < samples.length; i++) {
            sampleListModel.append(samples[i])
        }
    }
    
    function editSample() {
        if (sampleTable.currentRow >= 0 && publicationManager) {
            var samples = publicationManager.getAllSamples()
            if (sampleTable.currentRow < samples.length) {
                var currentSample = samples[sampleTable.currentRow]
                sampleEditorDialog.loadSample(currentSample.data || {})
                sampleEditorDialog.open()
            }
        }
    }
    
    function removeSample() {
        if (sampleTable.currentRow >= 0 && publicationManager) {
            var samples = publicationManager.getAllSamples()
            if (sampleTable.currentRow < samples.length) {
                var currentSample = samples[sampleTable.currentRow]
                publicationManager.removeSampleFromQueue(currentSample.index)
            }
        }
    }
    
    function clearAllSamples() {
        if (publicationManager) {
            publicationManager.clearSampleQueue()
        }
    }
    
    Component.onCompleted: {
        updateSampleList()
    }
}
