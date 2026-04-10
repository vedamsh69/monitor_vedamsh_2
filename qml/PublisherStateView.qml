// Copyright 2025 CDAC
import QtQuick 2.6
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: publisherStateView
    color: "white"
    
    property string topicName: ""
    property int domainId: 0
    
    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        
        ColumnLayout {
            width: parent.width - 20
            spacing: 15
            
            // Publisher Information
            GroupBox {
                title: "Publisher Information"
                Layout.fillWidth: true
                
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    rowSpacing: 5
                    columnSpacing: 10
                    
                    Label { text: "Topic Name:"; font.bold: true }
                    Label { text: topicName }
                    
                    Label { text: "Domain ID:"; font.bold: true }
                    Label { text: domainId }
                    
                    Label { text: "Data Type:"; font.bold: true }
                    Label { text: "DynamicType" }
                    
                    Label { text: "Status:"; font.bold: true }
                    Label { 
                        text: "Active"
                        color: "green"
                        font.bold: true
                    }
                }
            }
            
            // QoS Settings
            GroupBox {
                title: "Quality of Service (QoS)"
                Layout.fillWidth: true
                
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    rowSpacing: 5
                    columnSpacing: 10
                    
                    Label { text: "Reliability:"; font.bold: true }
                    Label { text: "RELIABLE" }
                    
                    Label { text: "Durability:"; font.bold: true }
                    Label { text: "TRANSIENT_LOCAL" }
                    
                    Label { text: "History:"; font.bold: true }
                    Label { text: "KEEP_LAST (depth: 10)" }
                    
                    Label { text: "Ownership:"; font.bold: true }
                    Label { text: "SHARED" }
                    
                    Label { text: "Liveliness:"; font.bold: true }
                    Label { text: "AUTOMATIC" }
                }
            }
            
            // Statistics
            GroupBox {
                title: "Publisher Statistics"
                Layout.fillWidth: true
                
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    rowSpacing: 5
                    columnSpacing: 10
                    
                    Label { text: "Samples Published:"; font.bold: true }
                    Label { 
                        id: samplesPublishedLabel
                        text: "0"
                    }
                    
                    Label { text: "Publication Rate:"; font.bold: true }
                    Label { 
                        id: publicationRateLabel
                        text: "0 samples/sec"
                    }
                    
                    Label { text: "Last Publication:"; font.bold: true }
                    Label { 
                        id: lastPublicationLabel
                        text: "Never"
                    }
                    
                    Label { text: "Matched Subscribers:"; font.bold: true }
                    Label { 
                        id: matchedSubscribersLabel
                        text: "0"
                    }
                }
            }
            
            // Actions
            GroupBox {
                title: "Actions"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    Button {
                        text: "Refresh Statistics"
                        Layout.fillWidth: true
                        onClicked: refreshStatistics()
                    }
                    
                    Button {
                        text: "View QoS Details"
                        Layout.fillWidth: true
                        onClicked: showQosDetails()
                    }
                    
                    Button {
                        text: "Export Configuration"
                        Layout.fillWidth: true
                        onClicked: exportConfiguration()
                    }
                }
            }
        }
    }
    
    function refreshStatistics() {
        console.log("Refreshing publisher statistics")
    }
    
    function showQosDetails() {
        console.log("Showing detailed QoS information")
    }
    
    function exportConfiguration() {
        console.log("Exporting publisher configuration")
    }
}
