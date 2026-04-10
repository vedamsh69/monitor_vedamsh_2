// Copyright 2025 CDAC
import QtQuick 2.6
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Rectangle {
    id: sampleQueueView
    color: "white"
    
    property var publicationManager: null
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Toolbar
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: "Edit"
                enabled: sampleListView.currentIndex >= 0
                onClicked: editSample()
            }
            
            Button {
                text: "Remove"
                enabled: sampleListView.currentIndex >= 0
                onClicked: removeSample()
            }
            
            Button {
                text: "Clear All"
                onClicked: clearAllSamples()
            }
            
            Button {
                text: "Load"
                onClicked: loadSamplesDialog.visible = true
            }
            
            Button {
                text: "Save"
                onClicked: saveSamplesDialog.visible = true
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // Sample List
        ListView {
            id: sampleListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            model: ListModel {
                id: sampleListModel
            }
            
            delegate: Rectangle {
                width: sampleListView.width
                height: 60
                color: sampleListView.currentIndex === index ? "#E3F2FD" : "white"
                border.color: "#CCCCCC"
                border.width: 1
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: sampleListView.currentIndex = index
                }
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    Label {
                        text: "Sample " + model.index
                        font.bold: true
                    }
                    
                    Label {
                        text: "Timestamp: " + (model.timestamp || "N/A")
                        font.pixelSize: 10
                    }
                    
                    Label {
                        text: "State: " + getStateText(model.state || 0)
                        font.pixelSize: 10
                    }
                }
                
                Button {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 5
                    text: "Publish"
                    onClicked: {
                        if (publicationManager) {
                            publicationManager.publishSample(model.index)
                        }
                    }
                }
            }
        }
        
        // Publish Controls
        RowLayout {
            Layout.fillWidth: true
            
            Label {
                text: "Publication period (ms):"
            }
            
            SpinBox {
                id: publishPeriodSpinbox
                from: 100
                to: 10000
                value: 500
                stepSize: 100
            }
            
            Button {
                text: "Publish All"
                highlighted: true
                onClicked: {
                    if (publicationManager) {
                        publicationManager.publishAllSamples()
                    }
                }
            }
        }
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
            if (publicationManager && sampleListView.currentIndex >= 0) {
                var samples = publicationManager.getAllSamples()
                if (sampleListView.currentIndex < samples.length) {
                    var currentSample = samples[sampleListView.currentIndex]
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
    function getStateText(state) {
        switch(state) {
            case 0: return "Pending"
            case 1: return "Published"
            case 2: return "Cached Only"
            default: return "Unknown"
        }
    }
    
    function updateSampleList() {
        if (!publicationManager) return
        
        sampleListModel.clear()
        var samples = publicationManager.getAllSamples()
        for (var i = 0; i < samples.length; i++) {
            sampleListModel.append(samples[i])
        }
    }
    
    function editSample() {
        if (sampleListView.currentIndex >= 0 && publicationManager) {
            var samples = publicationManager.getAllSamples()
            if (sampleListView.currentIndex < samples.length) {
                var currentSample = samples[sampleListView.currentIndex]
                sampleEditorDialog.loadSample(currentSample.data || {})
                sampleEditorDialog.visible = true
            }
        }
    }
    
    function removeSample() {
        if (sampleListView.currentIndex >= 0 && publicationManager) {
            var samples = publicationManager.getAllSamples()
            if (sampleListView.currentIndex < samples.length) {
                var currentSample = samples[sampleListView.currentIndex]
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
