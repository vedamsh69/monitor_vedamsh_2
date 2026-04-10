import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4 as QC1
import QtQuick.Controls.Styles 1.4

Item {
    id: equivalentidl

    // ========== EXPORT TO FILE FUNCTION ==========
    function saveToFile(filePath, content) {
        console.log("[EquivalentIDlView] Saving to file:", filePath)
        
        // Method 1: Try using XMLHttpRequest (works in some Qt versions)
        try {
            var request = new XMLHttpRequest()
            request.open("PUT", "file://" + filePath, false)
            request.send(content)
            
            if (request.status === 0 || request.status === 200) {
                console.log("[EquivalentIDlView] File saved successfully using XMLHttpRequest")
                return true
            }
        } catch (e) {
            console.log("[EquivalentIDlView] XMLHttpRequest failed:", e)
        }
        
        // Method 2: Fallback - print content for manual save
        console.log("[EquivalentIDlView] Content to save:")
        console.log(content)
        
        return false
    }

    // ========== MAIN LAYOUT ==========
    Rectangle {
        anchors.fill: parent
        color: "white"
        border.color: "#e0e0e0"
        border.width: 1

        ScrollView {
            id: scrollView
            anchors.fill: parent
            anchors.margins: 2
            
            // ScrollBar styling
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            TextArea {
                id: equivalentidltextbox
                text: topicIDLModel.textData
                readOnly: true
                selectByMouse: true
                selectByKeyboard: true
                wrapMode: TextArea.NoWrap
                
                // Font styling matching RTI
                font.family: "Courier New, Consolas, Monaco, monospace"
                font.pixelSize: 13
                color: "#000000"
                
                // Padding
                leftPadding: 10
                rightPadding: 10
                topPadding: 10
                bottomPadding: 10
                
                // Background
                background: Rectangle {
                    color: "white"
                    border.color: "#d0d0d0"
                    border.width: 0
                }
                
                // Text selection color
                selectedTextColor: "white"
                selectionColor: "#0078d7"
                
                // Enable keyboard shortcuts
                Keys.onPressed: {
                    if (event.modifiers & Qt.ControlModifier) {
                        if (event.key === Qt.Key_A) {
                            selectAll()
                            event.accepted = true
                        } else if (event.key === Qt.Key_C) {
                            copy()
                            event.accepted = true
                        }
                    }
                }
                
                // Right-click context menu
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    propagateComposedEvents: true
                    
                    onClicked: {
                        if (mouse.button === Qt.RightButton) {
                            contextMenu.popup()
                        }
                    }
                    
                    // Allow text selection
                    onPressed: {
                        mouse.accepted = false
                    }
                }
            }
        }

        // ========== CONTEXT MENU ==========
        Menu {
            id: contextMenu
            
            MenuItem {
                text: "Copy"
                enabled: equivalentidltextbox.selectedText.length > 0
                
                onTriggered: {
                    equivalentidltextbox.copy()
                    console.log("[EquivalentIDlView] Text copied to clipboard")
                }
            }
            
            MenuSeparator {}
            
            MenuItem {
                text: "Select All"
                
                onTriggered: {
                    equivalentidltextbox.selectAll()
                    console.log("[EquivalentIDlView] All text selected")
                }
            }
            
            MenuSeparator {}
            
            MenuItem {
                text: "Copy All"
                
                onTriggered: {
                    equivalentidltextbox.selectAll()
                    equivalentidltextbox.copy()
                    equivalentidltextbox.deselect()
                    console.log("[EquivalentIDlView] All text copied to clipboard")
                }
            }
        }

        // ========== EMPTY STATE MESSAGE ==========
        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: 150
            color: "#f9f9f9"
            border.color: "#e0e0e0"
            border.width: 1
            radius: 5
            visible: topicIDLModel.textData.length === 0
            
            Column {
                anchors.centerIn: parent
                spacing: 15
                
                Text {
                    text: "📄"
                    font.pixelSize: 48
                    anchors.horizontalCenter: parent.horizontalCenter
                    opacity: 0.5
                }
                
                Text {
                    text: "No IDL Data Available"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#666666"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Subscribe to a topic to view its IDL structure"
                    font.pixelSize: 12
                    color: "#999999"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
        
        // ========== STATUS BAR (Optional - shows line count) ==========
        Rectangle {
            id: statusBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 25
            color: "#f5f5f5"
            border.color: "#d0d0d0"
            border.width: 1
            visible: topicIDLModel.textData.length > 0
            
            Row {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10
                
                Text {
                    text: "Lines: " + equivalentidltextbox.lineCount
                    font.pixelSize: 10
                    color: "#666666"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Rectangle {
                    width: 1
                    height: 15
                    color: "#d0d0d0"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    text: "Characters: " + equivalentidltextbox.text.length
                    font.pixelSize: 10
                    color: "#666666"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Item {
                    width: parent.width - 250
                    height: parent.height
                }
                
                Text {
                    text: equivalentidltextbox.selectedText.length > 0 ? 
                          ("Selected: " + equivalentidltextbox.selectedText.length + " chars") : ""
                    font.pixelSize: 10
                    color: "#0078d7"
                    visible: equivalentidltextbox.selectedText.length > 0
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // ========== CONNECTIONS TO MODEL ==========
    Connections {
        target: topicIDLModel
        
        function onTextDataChanged() {
            console.log("[EquivalentIDlView] IDL text data changed, updating display")
            equivalentidltextbox.text = topicIDLModel.textData
            
            // Auto-scroll to top when new data arrives
            equivalentidltextbox.cursorPosition = 0
        }
    }
    
    // ========== COMPONENT INITIALIZATION ==========
    Component.onCompleted: {
        console.log("[EquivalentIDlView] Component loaded")
        equivalentidltextbox.text = topicIDLModel.textData
    }
}
