// Copyright 2025 CDAC
import QtQuick 2.6
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: pythonLogView
    color: "white"
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Toolbar
        RowLayout {
            Layout.fillWidth: true
            
            Label {
                text: "Python Execution Log"
                font.bold: true
                font.pixelSize: 14
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: "Clear"
                onClicked: clearLogs()
            }
            
            Button {
                text: "Export"
                onClicked: exportLogs()
            }
        }
        
        // Tab Bar for Console vs Errors
        TabBar {
            id: logTabBar
            Layout.fillWidth: true
            
            TabButton {
                text: "Console Output"
            }
            
            TabButton {
                text: "Errors (" + errorLogModel.count + ")"
            }
        }
        
        // Log Content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: logTabBar.currentIndex
            
            // Console Output Tab
            Rectangle {
                color: "#F5F5F5"
                border.color: "#CCCCCC"
                border.width: 1
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    clip: true
                    
                    ListView {
                        id: consoleLogListView
                        model: consoleLogModel
                        spacing: 2
                        
                        delegate: Rectangle {
                            width: consoleLogListView.width
                            height: logText.contentHeight + 10
                            color: index % 2 === 0 ? "white" : "#FAFAFA"
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 5
                                spacing: 10
                                
                                Label {
                                    text: model.timestamp
                                    font.pixelSize: 10
                                    color: "#666666"
                                    Layout.preferredWidth: 80
                                }
                                
                                Label {
                                    id: logText
                                    text: model.message
                                    font.family: "Courier"
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }
            }
            
            // Error Tab
            Rectangle {
                color: "#FFEBEE"
                border.color: "#F44336"
                border.width: 1
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    clip: true
                    
                    ListView {
                        id: errorLogListView
                        model: errorLogModel
                        spacing: 2
                        
                        delegate: Rectangle {
                            width: errorLogListView.width
                            height: errorText.contentHeight + 20
                            color: "#FFCDD2"
                            border.color: "#E57373"
                            border.width: 1
                            
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 5
                                spacing: 5
                                
                                RowLayout {
                                    Label {
                                        text: model.timestamp
                                        font.pixelSize: 10
                                        color: "#D32F2F"
                                        font.bold: true
                                    }
                                    
                                    Label {
                                        text: "[ERROR]"
                                        font.pixelSize: 10
                                        color: "#D32F2F"
                                        font.bold: true
                                    }
                                    
                                    Item { Layout.fillWidth: true }
                                }
                                
                                Label {
                                    id: errorText
                                    text: model.message
                                    font.family: "Courier"
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    color: "#B71C1C"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Data Models
    ListModel {
        id: consoleLogModel
    }
    
    ListModel {
        id: errorLogModel
    }
    
    // Functions
    function addLog(message, isError) {
        var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss")
        var logEntry = {
            "timestamp": timestamp,
            "message": message
        }
        
        if (isError === true) {
            errorLogModel.append(logEntry)
        } else {
            consoleLogModel.append(logEntry)
        }
        
        // Auto-scroll to bottom
        if (isError) {
            errorLogListView.positionViewAtEnd()
        } else {
            consoleLogListView.positionViewAtEnd()
        }
    }
    
    function clearLogs() {
        consoleLogModel.clear()
        errorLogModel.clear()
    }
    
    function exportLogs() {
        console.log("Exporting logs to file")
    }
    
    Component.onCompleted: {
        addLog("Python execution environment initialized")
        addLog("Ready to execute code")
    }
}
