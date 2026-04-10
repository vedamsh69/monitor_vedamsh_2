import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4 as QC1

Item {
    id: messagetab
    anchors.fill: parent

    property var filteredLogs: []
    property var displayedLogs: []
    property int maxDisplayRows: 1000
    
    // Store search state
    property string currentSearchText: ""
    property bool currentCaseSensitive: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: greyRectangle
            color: "#EBECF0"
            Layout.fillWidth: true
            Layout.preferredHeight: rowforbutton.height

            RowLayout {
                id: rowforbutton
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                spacing: 15
                anchors.margins: 5

                Button {
                    id: findbutton
                    text: "Find"
                    implicitHeight: 30
                    implicitWidth: 65
                    onClicked: findDlg.open()

                    background: Rectangle {
                        color: "white"
                        border.color: findbutton.down ? "#81c784" : "#c2c2c2"
                        border.width: 1
                        radius: 2
                    }

                    contentItem: Text {
                        text: findbutton.text
                        font: findbutton.font
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                FindDialog {
                    id: findDlg
                    
                    onDoSearch: {
                        console.log("Searching for:", text)
                        currentSearchText = text
                        currentCaseSensitive = caseSensitive
                        filterBySearch(text, caseSensitive)
                    }
                    
                    onClearSearch: {
                        clearSearchFilter()
                    }
                }

                Label {
                    text: "Row Count: "
                    verticalAlignment: Text.AlignVCenter
                }

                SpinBox {
                    id: rowCountSpinBox
                    editable: true
                    value: 1000
                    from: 100
                    to: 10000
                    stepSize: 100

                    onValueChanged: {
                        maxDisplayRows = value
                        updateFilteredLogs()
                    }

                    background: Rectangle {
                        implicitWidth: 85
                        implicitHeight: 23
                        border.color: rowCountSpinBox.focus ? "#b5e2ff" : "gray"
                        radius: 2
                    }

                    up.indicator: null
                    down.indicator: null
                    Layout.preferredWidth: 80

                    Rectangle {
                        anchors {
                            left: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                        width: 20
                        height: parent.height
                        color: "transparent"

                        Column {
                            anchors.fill: parent
                            spacing: 1

                            Button {
                                width: parent.width
                                height: parent.height / 2 - 0.5
                                text: "▲"
                                onClicked: rowCountSpinBox.increase()
                            }

                            Button {
                                width: parent.width
                                height: parent.height / 2 - 0.5
                                text: "▼"
                                onClicked: rowCountSpinBox.decrease()
                            }
                        }
                    }

                    implicitHeight: 23
                }

                Item { width: 10 }

                Label {
                    text: "View Filter: "
                }

                ComboBox {
                    id: logLevelComboBox
                    model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                    currentIndex: 0

                    onCurrentTextChanged: {
                        updateFilteredLogs()
                    }

                    implicitWidth: 100
                    implicitHeight: 25

                    background: Rectangle {
                        color: "white"
                        border.color: logLevelComboBox.pressed ? "#81c784" : "#c2c2c2"
                        border.width: 1
                        radius: 2
                    }

                    contentItem: Text {
                        leftPadding: 10
                        text: logLevelComboBox.displayText
                        font: logLevelComboBox.font
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

        Rectangle {
            id: tableContainer
            Layout.fillWidth: true
            Layout.preferredHeight: 500
            color: "white"
            border.color: "#EBECF0"
            border.width: 1

            QC1.TableView {
                id: tableView
                anchors.fill: parent
                anchors.margins: 1
                model: displayedLogs.length

                QC1.TableViewColumn { 
                    role: "id"; title: "Msg ID"; width: 60
                    delegate: Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        text: styleData.row < displayedLogs.length ? displayedLogs[styleData.row].id : ""
                        font.pixelSize: 13
                    }
                }
                
                QC1.TableViewColumn { 
                    role: "timestamp"; title: "Time"; width: 280
                    delegate: Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        text: styleData.row < displayedLogs.length ? displayedLogs[styleData.row].timestamp : ""
                        font.pixelSize: 13
                    }
                }
                
                QC1.TableViewColumn { 
                    role: "kind"; title: "Level"; width: 100
                    delegate: Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        text: styleData.row < displayedLogs.length ? displayedLogs[styleData.row].kind : ""
                        font.pixelSize: 13
                    }
                }
                
                QC1.TableViewColumn { 
                    role: "category"; title: "Category"; width: 150
                    delegate: Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        text: styleData.row < displayedLogs.length ? displayedLogs[styleData.row].category : ""
                        font.pixelSize: 13
                    }
                }
                
                QC1.TableViewColumn { 
                    role: "message"; title: "Message"; width: 300
                    delegate: Text {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: styleData.row < displayedLogs.length ? displayedLogs[styleData.row].message : ""
                        font.pixelSize: 13
                    }
                }

                headerDelegate: Rectangle {
                    color: "white"
                    height: 25
                    border.color: "#EBECF0"
                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: styleData.value
                    }
                }

                rowDelegate: Rectangle {
                    height: 30
                    color: {
                        if (styleData.row >= displayedLogs.length) return "white"
                        
                        var kind = displayedLogs[styleData.row].kind
                        if (kind === "DL_Error") return "#FFCDD2"
                        if (kind === "DL_Warning") return "#FFF9C4"
                        if (kind === "DL_Info") return "#BBDEFB"
                        if (kind === "DL_Notice") return "#C8E6C9"
                        if (kind === "DL_Debug") return "#D1C4E9"
                        return "white"
                    }
                }
            }
        }

        Rectangle {
            id: messageDetailsRectangle
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            border.color: "blue"
            color: "#F2F2F2"

            Label {
                id: messageDetailsLabel
                text: "Message Details"
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.topMargin: -10
                color: "blue"
                z: 1
                background: Rectangle {
                    color: messageDetailsRectangle.color
                    anchors.fill: parent
                    anchors.leftMargin: -5
                    anchors.rightMargin: -5
                }
            }

            Text {
                anchors.fill: parent
                anchors.margins: 10
                anchors.topMargin: 15
                wrapMode: Text.WordWrap
                text: {
                    if (tableView.currentRow !== -1 && tableView.currentRow < displayedLogs.length) {
                        var item = displayedLogs[tableView.currentRow]
                        return "Message ID: " + item.id +
                               "\nTimestamp: " + item.timestamp +
                               "\nLevel: " + item.kind +
                               "\nCategory: " + item.category +
                               "\nMessage: " + item.message
                    }
                    return "Select a log entry to view details"
                }
            }
        }
    }

    // Filter table by search text
    function filterBySearch(searchText, caseSensitive) {
        if (searchText.length === 0) {
            clearSearchFilter()
            return
        }
        
        var pattern = caseSensitive ? searchText : searchText.toLowerCase()
        var results = []
        
        console.log("Filtering", filteredLogs.length, "logs by:", pattern)
        
        for (var i = 0; i < filteredLogs.length; i++) {
            var log = filteredLogs[i]
            if (!log) continue
            
            var msg = log.message ? String(log.message) : ""
            var cat = log.category ? String(log.category) : ""
            var lvl = log.kind ? String(log.kind) : ""
            
            if (!caseSensitive) {
                msg = msg.toLowerCase()
                cat = cat.toLowerCase()
                lvl = lvl.toLowerCase()
            }
            
            if (msg.indexOf(pattern) !== -1 || 
                cat.indexOf(pattern) !== -1 || 
                lvl.indexOf(pattern) !== -1) {
                results.push(log)
            }
        }
        
        console.log("Found", results.length, "matching logs")
        displayedLogs = results
        findDlg.showResults(results.length)
    }

    // Clear search and show all logs
    function clearSearchFilter() {
        currentSearchText = ""
        currentCaseSensitive = false
        displayedLogs = filteredLogs
        console.log("Cleared search, showing all", displayedLogs.length, "logs")
    }

    // Update logs based on severity filter
    function updateFilteredLogs() {
        var selectedLevel = logLevelComboBox.currentText
        var totalLogs = dllogmodel.rowCount()
        var result = []
        
        var severityLevels = {
            "Trace": 0, "Silent": 0, "Debug": 1, "Info": 2,
            "Notice": 3, "Warning": 4, "Error": 5, "Severe": 6, "Fatal": 7
        }
        
        var filterSeverity = severityLevels[selectedLevel] || 0
        
        for (var i = 0; i < totalLogs; i++) {
            var logEntry = dllogmodel.getLog(i)
            if (!logEntry) continue
            
            var kind = logEntry.kind || ""
            var logSeverity = kind.replace("DL_", "")
            var logSeverityValue = severityLevels[logSeverity] || 0
            
            if (selectedLevel === "Trace" || logSeverityValue >= filterSeverity) {
                result.push(logEntry)
                if (result.length >= maxDisplayRows) break
            }
        }
        
        filteredLogs = result
        
        // IMPORTANT: Reapply search filter if active
        if (currentSearchText.length > 0) {
            console.log("Reapplying search filter:", currentSearchText)
            filterBySearch(currentSearchText, currentCaseSensitive)
        } else {
            displayedLogs = result
        }
        
        console.log("Updated - Filtered logs:", filteredLogs.length, "Displayed:", displayedLogs.length)
    }

    Timer {
        interval: 2000
        running: true
        repeat: true
        onTriggered: {
            dllogmodel.refreshLogs()
            updateFilteredLogs()  // This will automatically reapply search if active
        }
    }

    Component.onCompleted: {
        updateFilteredLogs()
    }
}
