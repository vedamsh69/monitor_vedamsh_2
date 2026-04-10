// Copyright 2025 CDAC - RTI Admin Console Style Publication UI
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import indedds.monitor 1.0


Rectangle {
    id: publicationTab
    color: "#f0f0f0"


    // Properties to be set from parent
    property string topicName: ""
        property int domainId: 0
            property var controllerInstance: typeof controller !== 'undefined' ? controller : null
                // Expose publicationManager
                property alias publicationManager: publicationManager
                    // Publication Manager instance
                    PublicationManager {
                        id: publicationManager


        Component.onCompleted: {
            console.log("[PublicationTab] PublicationManager initialized")
            if (controllerInstance)
            {
                publicationManager.controller = controllerInstance
                console.log("[PublicationTab] ✓ Controller injected into PublicationManager")
            }
            else
            {
                console.log("[PublicationTab] Controller not available yet; waiting for binding update")
            }
        }
                        onLogMessage: function(message) {
                        pythonLogView.append(message)
                    }


                    onErrorOccurred: function(errorMsg) {
                    console.error("[PublicationTab] Error:", errorMsg)
                    pythonLogView.append("ERROR: " + errorMsg)
                }


                // ✅ ADD THIS: Forward periodic publish requests to Controller
                onPublishSampleRequest: function(topicName, domainId, sampleData) {
                console.log("[Periodic Publish] Topic:", topicName, "Domain:", domainId)
                console.log("[Periodic Publish] Data:", JSON.stringify(sampleData))
                // Actual DDS publish is handled in PublicationManager::onPeriodicPublishTimeout().
                // Keep this signal path only for QML logging/visibility.
                pythonLogView.append("⏱ Periodic publish request dispatched")
}

}


// Monitor topic name changes
onTopicNameChanged: {
    console.log("[PublicationTab] Topic name changed to:", topicName)
    publicationManager.topicName = topicName
    publicationManager.domainId = domainId
}

onDomainIdChanged: {
    console.log("[PublicationTab] Domain ID changed to:", domainId)
    publicationManager.domainId = domainId
}


// Main layout
RowLayout {
    anchors.fill: parent
    anchors.margins: 10
    spacing: 10


    // ==================== LEFT PANEL: Code Editor ====================
    Rectangle {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: parent.width * 0.5
        color: "white"
        border.color: "#cccccc"
        border.width: 1


        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10


            // Title bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                color: "#2c3e50"


                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10


                    Label {
                        text: "Publication - " + publicationTab.topicName
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                    }


                    Item { Layout.fillWidth: true }


                    Button {
                        text: publicationManager.isPublishing ? "⏸ Stop Publishing" : "▶ Start Publishing"
                        Layout.preferredWidth: 150
                        onClicked: {
                            console.log("[PublicationTab::StartStop] ==================================================")
                            console.log("[PublicationTab::StartStop] Topic=" + publicationTab.topicName +
                                        ", Domain=" + publicationTab.domainId +
                                        ", QueueSize=" + publicationManager.sampleQueueSize)
                            if (publicationManager.isPublishing)
                            {
                                console.log("[PublicationTab::StartStop] STOP button clicked")
                                publicationManager.stopPeriodicPublishing()
                            } else {
                            console.log("[PublicationTab::StartStop] START button clicked")
                            console.log("[PublicationTab::StartStop] Opening period dialog...")
                            periodDialog.open()
                        }
                        console.log("[PublicationTab::StartStop] ==================================================")
                    }
                }
            }
        }


        // Toolbar buttons - RTI SIMPLE STYLE
        RowLayout {
            Layout.fillWidth: true
            spacing: 5


            ToolButton {
                text: "Load"
                ToolTip.text: "Load script from file"
                onClicked: {
                    console.log("[PublicationTab::Load] Load button clicked")
                    loadScriptDialog.open()
                }
            }


            ToolButton {
                text: "Save"
                ToolTip.text: "Save script to file"
                onClicked: {
                    console.log("[PublicationTab::Save] Save button clicked")
                    saveScriptDialog.open()
                }
            }


            ToolSeparator {}


            ToolButton {
                text: "🔄"
                ToolTip.text: "Reset to template"
                onClicked: {
                    console.log("[PublicationTab::Reset] ========================================")
                    console.log("[PublicationTab::Reset] Reset button clicked")
                    console.log("[PublicationTab::Reset] ✓ Resetting code editor to template")
                    console.log("[PublicationTab::Reset] Original template length: " + publicationManager.sampleCode.length)
                    codeEditor.text = publicationManager.sampleCode
                    console.log("[PublicationTab::Reset] ✓ Code editor reset successfully")
                    console.log("[PublicationTab::Reset] ========================================")
                }
            }


            ToolButton {
                text: "?"
                ToolTip.text: "Help"
                onClicked: {
                    console.log("[PublicationTab::Help] Help button clicked")
                    helpDialog.open()
                }
            }


            Item { Layout.fillWidth: true }
        }


        // Label above code editor
        Label {
            text: "code editing area"
            color: "#e74c3c"
            font.italic: true
            font.pixelSize: 11
        }


        // ========== CODE EDITOR - RTI STYLE (TextEdit for reliability) ==========
        Rectangle {
            id: codeEditorContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "#d0d0d0"
            border.width: 1


            ScrollView {
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                ScrollBar.vertical.policy: ScrollBar.AsNeeded


                TextEdit {
                    id: codeEditor
                    width: codeEditorContainer.width - 16
                    height: Math.max(codeEditorContainer.height - 16, contentHeight)


                    leftPadding: 8
                    rightPadding: 8
                    topPadding: 8
                    bottomPadding: 8


                    font.family: "Courier New"
                    font.pixelSize: 13
                    color: "#2c3e50"


                    selectByMouse: true
                    selectByKeyboard: true
                    selectionColor: "#3498db"
                    selectedTextColor: "white"


                    wrapMode: TextEdit.NoWrap
                    textFormat: TextEdit.PlainText


                    // ========== CRITICAL: Initialize empty ==========
                    text: ""


                    // ========== Initialize on component load ==========
                    Component.onCompleted: {
                        console.log("[CodeEditor] ✓ Component completed")
                        console.log("[CodeEditor] Initial sampleCode:", publicationManager.sampleCode)
                        console.log("[CodeEditor] Length:", publicationManager.sampleCode.length)


                        // Force initial load
                        if (publicationManager.sampleCode.length > 0)
                        {
                            text = publicationManager.sampleCode
                            console.log("[CodeEditor] ✓ Initial text SET")
                        } else {
                        console.log("[CodeEditor] ⚠ sampleCode is empty, waiting...")
                    }
                }


                // ========== React to code changes ==========
                Connections {
                    target: publicationManager


                    function onSampleCodeChanged()
                    {
                        console.log("[CodeEditor] ========================================")
                        console.log("[CodeEditor] ✓ sampleCodeChanged() signal received")
                        console.log("[CodeEditor] New code length:", publicationManager.sampleCode.length)
                        console.log("[CodeEditor] New code:")
                        console.log(publicationManager.sampleCode)

                        // FORCE clear and reset (fixes Qt binding issues)
                        codeEditor.text = ""
                        codeEditor.text = publicationManager.sampleCode


                        console.log("[CodeEditor] ✓ TextEdit updated")
                        console.log("[CodeEditor] Current TextEdit.text length:", codeEditor.text.length)
                        console.log("[CodeEditor] ========================================")
                    }
                }
            }
        }
    }


    // Status bar - RTI STYLE
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        color: "#ecf0f1"
        border.color: "#bdc3c7"
        border.width: 1


        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8


            // Status indicator
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: {
                    if (publicationManager.statusMessage.includes("✓") ||
                    publicationManager.statusMessage.includes("Ready")) {
                    return "#27ae60"  // Green
                } else if (publicationManager.statusMessage.includes("⚠")) {
                return "#f39c12"  // Orange
            } else {
            return "#95a5a6"  // Gray
        }
    }
}


Label {
    text: publicationManager.statusMessage
    font.pixelSize: 12
    Layout.fillWidth: true
}
}
}


// Action buttons - RTI STYLE
Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 44
    color: "#f5f5f5"
    border.color: "#d0d0d0"
    border.width: 1


    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 10


        Label {
            text: "Publish sample, cache sample"
            color: "#7f8c8d"
            font.pixelSize: 11
        }


        Item { Layout.fillWidth: true }


        Button {
            text: "Execute"
            implicitWidth: 100
            highlighted: true
            onClicked: {
                console.log("========================================")
                console.log("[PublicationTab::Execute] ==================================================")
                console.log("[PublicationTab::Execute] EXECUTE BUTTON CLICKED!")
                console.log("[PublicationTab::Execute] Topic name: " + publicationTab.topicName)
                console.log("[PublicationTab::Execute] Domain ID: " + publicationTab.domainId)
                console.log("[PublicationTab::Execute] Controller available: " + (controllerInstance !== null))
                console.log("[PublicationTab::Execute] Code length: " + codeEditor.text.length)
                console.log("[PublicationTab::Execute] Topic type: " + publicationManager.topicType)
                console.log("[PublicationTab::Execute] ==================================================")

                if (publicationManager.topicType.length === 0)
                {
                    console.error("[PublicationTab::Execute] ✗ Type not discovered yet. Blocking execute.")
                    pythonLogView.append("❌ ERROR: Topic IDL not discovered yet. Please wait.")
                    return
                }

                // STEP 1: Parse the Python-like code into QVariantMap
                console.log("[PublicationTab::Execute] STEP 1: Parsing Python code...")
                var sampleData = publicationManager.executePythonCode(codeEditor.text)
                console.log("[PublicationTab::Execute] STEP 1 RESULT: Parsed " + Object.keys(sampleData).length + " fields")
                console.log("[PublicationTab::Execute] Sample data: " + JSON.stringify(sampleData))

                if (Object.keys(sampleData).length === 0)
                {
                    console.error("[PublicationTab::Execute] ✗ FAILED: No fields extracted from code!")
                    console.error("[PublicationTab::Execute] Possible issues:")
                    console.error("[PublicationTab::Execute]   - Code is empty")
                    console.error("[PublicationTab::Execute]   - Code syntax error (missing 'sample.' prefix)")
                    pythonLogView.append("❌ ERROR: No valid sample data extracted. Check code syntax.")
                    console.log("========================================")
                    return
                }

                console.log("[PublicationTab::Execute] ========================================")

                // STEP 2: Add to queue for display
                console.log("[PublicationTab::Execute] STEP 2: Adding sample to queue...")
                var index = publicationManager.addSampleToQueue(sampleData)
                console.log("[PublicationTab::Execute] ✓ Sample #" + index + " added to queue (total: " + publicationManager.sampleQueueSize + ")")
                console.log("[PublicationTab::Execute] ========================================")

                // STEP 3: ✅ ACTUALLY PUBLISH TO DDS via Controller
                if (controllerInstance)
                {
                    console.log("[PublicationTab::Execute] STEP 3: Publishing to DDS via Controller...")
                    console.log("[PublicationTab::Execute] Calling controller.publishOneSample()...")

                    var success = controllerInstance.publishOneSample(
                        publicationTab.topicName,
                        publicationTab.domainId,
                        sampleData
                    )

                    console.log("[PublicationTab::Execute] publishOneSample() returned: " + success)

                    if (success)
                    {
                        console.log("[PublicationTab::Execute] ✓ ✓ ✓ SUCCESS: SAMPLE PUBLISHED TO DDS!")
                        pythonLogView.append("✅ Sample #" + index + " PUBLISHED to DDS ✓")
                        pythonLogView.append("   Fields: " + Object.keys(sampleData).length + " | Topic: " + publicationTab.topicName)
                    } else {
                    console.error("[PublicationTab::Execute] ✗ FAILED: DDS publish returned false!")
                    pythonLogView.append("❌ ERROR: Failed to publish sample to DDS")
                }
            } else {
            console.error("[PublicationTab::Execute] ✗ CRITICAL: Controller is NULL!")
            pythonLogView.append("❌ ERROR: Controller not available!")
        }

        console.log("[PublicationTab::Execute] ==================================================")
        console.log("========================================")
    }
}



Button {
    text: "Cache"
    implicitWidth: 100

    onClicked: {
        console.log("========================================")
        console.log("[PublicationTab::Cache] CACHE BUTTON CLICKED!")
        console.log("[PublicationTab::Cache] Code length: " + codeEditor.text.length)

        var sampleData = publicationManager.executePythonCode(codeEditor.text)
        console.log("[PublicationTab::Cache] Parsed " + Object.keys(sampleData).length + " fields")

        if (Object.keys(sampleData).length > 0)
        {
            var index = publicationManager.addSampleToQueue(sampleData)
            console.log("[PublicationTab::Cache] ✓ Sample #" + index + " cached (queue size: " + publicationManager.sampleQueueSize + ")")
            pythonLogView.append("📌 Sample #" + index + " cached (" + Object.keys(sampleData).length + " fields)")
        } else {
        console.error("[PublicationTab::Cache] ✗ No valid data to cache")
        pythonLogView.append("❌ ERROR: No valid sample data to cache")
    }
    console.log("========================================")
}
}
}
}
}
}


// ==================== RIGHT PANEL: Samples Queue and Info ====================
Rectangle {
    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.preferredWidth: parent.width * 0.5
    color: "white"
    border.color: "#cccccc"
    border.width: 1


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10


        // Tab bar
        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: "Samples Queue (" + publicationManager.sampleQueueSize + ")"
            }
            TabButton { text: "Publisher State" }
            TabButton { text: "Python Log" }
        }


        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex


            // ========== TAB 1: Samples Queue ==========
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10


                    Label {
                        text: "information area"
                        color: "#e74c3c"
                        font.italic: true
                        font.pixelSize: 11
                    }


                    // Sample queue toolbar
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5


                        ToolButton {
                            text: "Edit"
                            enabled: false
                        }


                        ToolButton {
                            text: "Remove"
                            enabled: publicationManager.sampleQueueSize > 0
                        }


                        ToolButton {
                            text: "Clear All"
                            enabled: publicationManager.sampleQueueSize > 0
                            onClicked: {
                                console.log("[PublicationTab::ClearAll] ==================================================")
                                console.log("[PublicationTab::ClearAll] Clear All button clicked")
                                console.log("[PublicationTab::ClearAll] Clearing " + publicationManager.sampleQueueSize + " samples from queue...")
                                publicationManager.clearSampleQueue()
                                console.log("[PublicationTab::ClearAll] ✓ Queue cleared successfully")
                                console.log("[PublicationTab::ClearAll] ==================================================")
                            }
                        }


                        ToolSeparator {}


                        ToolButton {
                            text: "Load"
                            ToolTip.text: "Load samples from file"
                        }


                        ToolButton {
                            text: "Save"
                            ToolTip.text: "Save samples to file"
                            enabled: publicationManager.sampleQueueSize > 0
                        }
                    }


                    // Sample table/list
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        border.color: "#cccccc"
                        border.width: 1


                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0


                            // Header
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                color: "#ecf0f1"


                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 0


                                    Label {
                                        text: "Sample"
                                        font.bold: true
                                        Layout.preferredWidth: 80
                                        horizontalAlignment: Text.AlignHCenter
                                    }


                                    Rectangle { width: 1; Layout.fillHeight: true; color: "#bdc3c7" }


                                    Label {
                                        text: "Actions"
                                        font.bold: true
                                        Layout.preferredWidth: 100
                                        horizontalAlignment: Text.AlignHCenter
                                    }


                                    Rectangle { width: 1; Layout.fillHeight: true; color: "#bdc3c7" }


                                    Label {
                                        text: "Sample content"
                                        font.bold: true
                                        Layout.fillWidth: true
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                            }


                            // Sample list
                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true


                                ListView {
                                    id: sampleListView
                                    model: publicationManager.sampleQueueSize


                                    delegate: Rectangle {
                                        width: sampleListView.width
                                        height: 40
                                        color: index % 2 === 0 ? "white" : "#f9f9f9"


                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 5
                                            spacing: 5


                                            Label {
                                                text: index.toString()
                                                Layout.preferredWidth: 70
                                                horizontalAlignment: Text.AlignHCenter
                                            }


                                            Button {
                                                text: "Publish"
                                                Layout.preferredWidth: 90
                                                onClicked: {
                                                    console.log("[PublicationTab::PublishOne] ==================================================")
                                                    console.log("[PublicationTab::PublishOne] Publish button clicked for sample index: " + index)
                                                    console.log("[PublicationTab::PublishOne] Topic=" + publicationTab.topicName +
                                                                ", Domain=" + publicationTab.domainId)
                                                    if (!controllerInstance)
                                                    {
                                                        console.error("[PublicationTab::PublishOne] ✗ Controller is NULL")
                                                        pythonLogView.append("ERROR: Controller not available")
                                                        console.log("[PublicationTab::PublishOne] ==================================================")
                                                        return
                                                    }

                                                    // Get the sample from queue
                                                    var samples = publicationManager.getAllSamples()
                                                    if (index < samples.length)
                                                    {
                                                        var sampleData = samples[index]

                                                        // Publish via Controller
                                                        var success = controllerInstance.publishOneSample(  // Changed from 'controller'
                                                        publicationTab.topicName,
                                                        publicationTab.domainId,
                                                        sampleData
                                                    )

                                                    if (success)
                                                    {
                                                        console.log("[PublicationTab::PublishOne] ✓ SUCCESS for sample index: " + index)
                                                        pythonLogView.append("✓ Sample #" + index + " published")
                                                    } else {
                                                    console.error("[PublicationTab::PublishOne] ✗ FAILED for sample index: " + index)
                                                    pythonLogView.append("✗ Sample #" + index + " publish failed")
                                                }
                                            }
                                            console.log("[PublicationTab::PublishOne] ==================================================")
                                        }
                                    }



                                    Label {
                                        text: "Sample data #" + index
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }


                        // Empty message
                        Label {
                            anchors.centerIn: parent
                            text: "No samples available for publishing"
                            color: "#7f8c8d"
                            font.italic: true
                            visible: publicationManager.sampleQueueSize === 0
                        }
                    }
                }
            }


            // Publishing controls - RTI STYLE
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: "#ecf0f1"
                border.color: "#bdc3c7"
                border.width: 1


                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10


                    Label {
                        text: "Publication period (ms):"
                        font.pixelSize: 12
                    }


                    Button {
                        text: "<"
                        implicitWidth: 30
                        onClicked: {
                            periodSpinBox.value = Math.max(100, periodSpinBox.value - 100)
                        }
                    }


                    SpinBox {
                        id: periodSpinBox
                        from: 100
                        to: 10000
                        value: 700
                        stepSize: 100
                        editable: true
                        implicitWidth: 120
                    }

                    Button {
                        text: ">"
                        implicitWidth: 30
                        onClicked: {
                            periodSpinBox.value = Math.min(10000, periodSpinBox.value + 100)
                        }
                    }


                    Item { Layout.fillWidth: true }


                    Button {
                        text: "Publish All"
                        implicitWidth: 100
                        enabled: publicationManager.sampleQueueSize > 0
                        onClicked: {
                            console.log("========================================")
                            console.log("[PublicationTab::PublishAll] ==================================================")
                            console.log("[PublicationTab::PublishAll] PUBLISH ALL button clicked!")
                            console.log("[PublicationTab::PublishAll] Samples in queue: " + publicationManager.sampleQueueSize)

                            if (!controllerInstance)
                            {
                                console.error("[PublicationTab::PublishAll] ✗ CRITICAL: Controller is NULL!")
                                pythonLogView.append("❌ ERROR: Controller not available!")
                                console.log("========================================")
                                return
                            }

                            console.log("[PublicationTab::PublishAll] ✓ Controller available")

                            // Get all samples from queue
                            console.log("[PublicationTab::PublishAll] Fetching all samples from queue...")
                            var samples = publicationManager.getAllSamples()
                            console.log("[PublicationTab::PublishAll] ✓ Retrieved " + samples.length + " samples")

                            var successCount = 0
                            var failCount = 0

                            for (var i = 0; i < samples.length; i++) {
                                var sampleData = samples[i]
                                console.log("[PublicationTab::PublishAll] ========== Sample " + i + " ==========")
                                console.log("[PublicationTab::PublishAll] Data: " + JSON.stringify(sampleData))

                                // Publish each sample to DDS
                                console.log("[PublicationTab::PublishAll] Calling publishOneSample()...")
                                var success = controllerInstance.publishOneSample(
                                    publicationTab.topicName,
                                    publicationTab.domainId,
                                    sampleData
                                )

                                if (success)
                                {
                                    console.log("[PublicationTab::PublishAll] ✓ SUCCESS: Sample " + i + " published to DDS")
                                    pythonLogView.append("✅ Sample #" + i + " published successfully")
                                    successCount++
                                } else {
                                console.error("[PublicationTab::PublishAll] ✗ FAILED: Sample " + i + " publish FAILED")
                                pythonLogView.append("❌ Sample #" + i + " publish FAILED")
                                failCount++
                            }
                        }

                        console.log("[PublicationTab::PublishAll] ========================================")
                        console.log("[PublicationTab::PublishAll] SUMMARY: " + successCount + " succeeded, " + failCount + " failed")
                        pythonLogView.append("--- Published " + successCount + "/" + samples.length + " samples successfully ---")
                        console.log("[PublicationTab::PublishAll] ==================================================")
                        console.log("========================================")
                    }
                }



            }
        }
    }
}


// ========== TAB 2: Publisher State ==========
Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10


        GroupBox {
            title: "Publisher Information"
            Layout.fillWidth: true


            GridLayout {
                anchors.fill: parent
                columns: 2
                rowSpacing: 8
                columnSpacing: 15


                Label { text: "Topic Name:"; font.bold: true }
                Label { text: publicationManager.topicName }


                Label { text: "Topic Type:"; font.bold: true }
                Label { text: publicationManager.topicType }


                Label { text: "Domain ID:"; font.bold: true }
                Label { text: publicationManager.domainId.toString() }


                Label { text: "Queue Size:"; font.bold: true }
                Label { text: publicationManager.sampleQueueSize.toString() }


                Label { text: "Publishing:"; font.bold: true }
                Label {
                    text: publicationManager.isPublishing ? "Active" : "Stopped"
                    color: publicationManager.isPublishing ? "#2ecc71" : "#e74c3c"
                    font.bold: true
                }
            }
        }


        Item { Layout.fillHeight: true }
    }
}


// ========== TAB 3: Python Log ==========
Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 10


        RowLayout {
            Layout.fillWidth: true


            Label {
                text: "Execution Log"
                font.bold: true
                font.pixelSize: 13
            }


            Item { Layout.fillWidth: true }


            Button {
                text: "Clear Log"
                onClicked: {
                    pythonLogView.text = "Python execution log:\n"
                }
            }
        }


        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true


            TextArea {
                id: pythonLogView
                readOnly: true
                wrapMode: TextArea.Wrap
                font.family: "Courier New"
                font.pixelSize: 11
                text: "Python execution log:\n"


                function append(message)
                {
                    text += "[" + Qt.formatDateTime(new Date(), "hh:mm:ss") + "] " + message + "\n"
                }
            }
        }
    }
}
}
}
}
}


// ==================== DIALOGS ====================


FileDialog {
    id: loadScriptDialog
    title: "Load Python Script"
    nameFilters: ["Python files (*.py)", "All files (*)"]
}


FileDialog {
    id: saveScriptDialog
    title: "Save Python Script"
    selectExisting: false
    nameFilters: ["Python files (*.py)", "All files (*)"]
}


Dialog {
    id: periodDialog
    title: "Set Publication Period"
    standardButtons: Dialog.Ok | Dialog.Cancel


    ColumnLayout {
        spacing: 10


        Label {
            text: "Enter publication period in milliseconds:"
        }


        SpinBox {
            id: periodDialogSpinBox
            from: 100
            to: 10000
            value: 700
            stepSize: 100
            Layout.fillWidth: true
        }
    }

    onAccepted: {
        console.log("[PublicationTab::PeriodDialog] ========================================")
        console.log("[PublicationTab::PeriodDialog] START PUBLISHING confirmed")
        console.log("[PublicationTab::PeriodDialog] Period(ms): " + periodDialogSpinBox.value)
        console.log("[PublicationTab::PeriodDialog] Queue size: " + publicationManager.sampleQueueSize)
        if (publicationManager.topicType.length === 0)
        {
            console.error("[PublicationTab::PeriodDialog] ✗ Type not discovered yet. Blocking start.")
            pythonLogView.append("❌ ERROR: Cannot start publishing before IDL discovery.")
            console.log("[PublicationTab::PeriodDialog] ========================================")
            return
        }
        publicationManager.startPeriodicPublishing(periodDialogSpinBox.value)
        console.log("[PublicationTab::PeriodDialog] startPeriodicPublishing() invoked")
        console.log("[PublicationTab::PeriodDialog] ========================================")
    }
}


Dialog {
    id: helpDialog
    title: "Publication Help"
    standardButtons: Dialog.Ok
    width: 500


    ScrollView {
        anchors.fill: parent


        Label {
            width: helpDialog.width - 40
            wrapMode: Text.WordWrap
            text: "<h3>RTI-style Publication Interface</h3>" +
            "<p><b>Usage:</b></p>" +
            "<ol>" +
            "<li>Select a topic from the topic list (will auto-load structure)</li>" +
            "<li>Edit sample fields in the code editor</li>" +
            "<li>Click '▶ Execute' to add sample to queue</li>" +
            "<li>Use 'Publish All' to send all queued samples</li>" +
            "<li>Or start periodic publishing with custom interval</li>" +
            "</ol>" +
            "<p><b>Features:</b></p>" +
            "<ul>" +
            "<li>Dynamic topic structure loading from IDL</li>" +
            "<li>Python-like syntax for sample editing</li>" +
            "<li>Sample queue management</li>" +
            "<li>Periodic and on-demand publishing</li>" +
            "<li>Load/Save scripts and samples</li>" +
            "</ul>"
        }
    }
}
}
