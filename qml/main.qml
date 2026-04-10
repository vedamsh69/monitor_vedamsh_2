// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.
//
// eProsima Fast DDS Monitor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Monitor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Monitor. If not, see <https://www.gnu.org/licenses/>.

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Dialogs 1.2

import Theme 1.0

ApplicationWindow {
    id: mainApplicationView
    visible: true
    width: 1600
    height: 900
    title: qsTr("Admin & Monitor")

    signal startMonitoring

    // This property holds the current number of initialized monitors
    property int monitors: 0

        onStartMonitoring: {
            mainStartView.visible = false
            monitorMenuBar.visible = true
            toolBar.isVisible = true
            panels.visible = true
            dialogInitMonitor.open()
        }

        MainStartView {
            id: mainStartView
            visible: true
        }

        menuBar: MonitorMenuBar {
            id: monitorMenuBar
            visible: false
            onToolBarHidden: toolBar.isVisible = !toolBar.isVisible
            onDispDataButtonHidden: toolBar.isVisibleDispData = !toolBar.isVisibleDispData
            onDispDynDataButtonHidden: toolBar.isVisibleDispDynData = !toolBar.isVisibleDispDynData
            onRefreshButtonHidden: toolBar.isVisibleRefresh = !toolBar.isVisibleRefresh
            onClearLogButtonHidden: toolBar.isVisibleClearLog = !toolBar.isVisibleClearLog
            onDashboardLayoutButtonHidden: toolBar.isVisibleDashboardLayout = !toolBar.isVisibleDashboardLayout
            onClearIssuesButtonHidden: toolBar.isVisibleClearIssues = !toolBar.isVisibleClearIssues
            onLeftSidebarHidden: panels.openCloseLeftSideBar()
            onChangeChartboxLayout: {
                panels.changeChartboxLayout(chartsPerRow)
                toolBar.changeChartboxLayoutIcon(chartsPerRow)
            }
            onSaveAllCSV: {
                panels.saveAllCSV()
            }
            onExplorerDDSEntitiesChanged: panels.changeExplorerDDSEntities(status)
            onExplorerPhysicalChanged: panels.changeExplorerPhysical(status)
            onExplorerLogicalChanged: panels.changeExplorerLogical(status)
            onExplorerEntityInfoChanged: panels.changeExplorerEntityInfo(status)
        }

        header: MonitorToolBar {
            id: toolBar
            onChangeChartboxLayout: monitorMenuBar.changeChartboxLayoutViewMenu(chartsPerRow)
        }

        Panels {
            id: panels
            visible: false
            onExplorerDDSEntitiesChanged: monitorMenuBar.changeExplorerDDSEntities(status)
            onExplorerPhysicalChanged: monitorMenuBar.changeExplorerPhysical(status)
            onExplorerLogicalChanged: monitorMenuBar.changeExplorerLogical(status)
            onExplorerEntityInfoChanged: monitorMenuBar.changeExplorerEntityInfo(status)
        }

        InitMonitorDialog {
            id: dialogInitMonitor
        }

        TopicListView {
            id: topicListDialog
        }

        DialogComponent
    {
        id : dialogforidl
    }

    SampleInspector
{
    id : sampleDialog
}


SampleLog
{
id : sampleLogWindow
}


PreferenceTable {
    id: preferenceTableDialog
}

NetworkInfoDialog
{
id : networkInfoDialog
}


DistributedLogger {
    id: distributedlogger
}


Dl {
    id : dlarea
}

Distri
{
id : distributed
}

TypeTreeView
{
id : typetreedialog
}

InitDSMonitorDialog {
    id: dialogDSInitMonitor
}

DumpFileDialog {
    id: dumpDialog
    clear: false
}

DumpFileDialog {
    id: dumpDialogClear
    clear: true
}

CreateSubscriptionDialog
{
id : createsubscriptiondialogid
}

PublishDialog {
    id: publishDialogid

    // REMOVED onAccepted - PublishDialog.qml handles tab opening after discovery
}



HistoricDataKindDialog {
    id: dataKindDialog
    onCreateChart: panels.createHistoricChart(dataKind)
}

DynamicDataKindDialog {
    id: dynamicDataKindDialog
    onCreateChart: panels.createDynamicChart(dataKind, timeWindowSeconds, updatePeriod, maxPoints)
}

ScheduleClearDialog {
    id: scheduleClear
}

AboutDialog {
    id: aboutDialog
}

ErrorDialog {
    id: errorDialog
}

SeriesSetMaxPointsDialog {
    id: seriesSetMaxPointsDialog
}

MatchAnalysesDialog {
    id: matchAnalysesDialog
}

// ============================================
// Publication Tab as Full-Screen Popup
// ============================================
Popup {
    id: publicationTabPopup
    visible: false
    x: 0
    y: 0
    width: parent.width
    height: parent.height
    modal: true
    closePolicy: Popup.NoAutoClose
    padding: 0

    background: Rectangle {
        color: "#f5f5f5"
    }

    PublicationTab {
        id: publicationTab
        anchors.fill: parent


        // Close button overlay
        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 15
            width: 45
            height: 45
            color: "#e74c3c"
            radius: 23
            z: 2000

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onEntered: parent.color = "#c0392b"
                onExited: parent.color = "#e74c3c"

                onClicked: {
                    console.log("[Main] Closing Publication Tab")
                    publicationTabPopup.close()
                    if (publicationTab.publicationManager.isPublishing)
                    {
                        publicationTab.publicationManager.stopPeriodicPublishing()
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: "×"
                color: "white"
                font.pixelSize: 32
                font.bold: true
            }
        }

        Component.onCompleted: {
            console.log("[PublicationTab] Popup loaded")
            console.log("[PublicationTab] Checking topicIDLModel availability...")

            if (typeof topicIDLModel !== 'undefined' && topicIDLModel !== null)
            {
                console.log("[PublicationTab] topicIDLModel is AVAILABLE")
                // FIXED: Access textData as PROPERTY, not function
                console.log("[PublicationTab] topicIDLModel textData:", topicIDLModel.textData)
                publicationTab.publicationManager.setTopicIDLModel(topicIDLModel)
                console.log("[PublicationTab] topicIDLModel connected to PublicationManager")
            } else {
            console.error("[PublicationTab] topicIDLModel is NOT AVAILABLE!")
        }
    }
}
}
// ============================================


// Creates a connection between Controller::error(QString, int) signal and ErrorDialog
Connections {
    target: controller
    function onError(errorMsg, errorType)
    {
        errorDialog.text = errorMsg
        errorDialog.errorType = errorType
        errorDialog.open()
    }
}

// Creates a connection between Controller::monitorInitialized() signal and qml
Connections {
    target: controller
    function onMonitorInitialized()
    {
        monitors++
    }
}

// ============================================
// Publication Tab Management Function
// ============================================
function openPublicationTab(topicName, domainId)
{
    console.log("[Main] ========================================")
    console.log("[Main] 🚀 openPublicationTab called")
    console.log("[Main] Topic:", topicName)
    console.log("[Main] Domain:", domainId)
    console.log("[Main] topicIDLModel:", topicIDLModel)
    console.log("[Main] topicIDLModel.textData:", topicIDLModel ? topicIDLModel.textData : "NULL")
    console.log("[Main] ========================================")

    // ========== REMOVED VALIDATION - Allow opening even if IDL is empty ==========
    // The PublicationManager will use fallback structure if needed
    console.log("[Main] ✓ Proceeding to open Publication Tab...")

    // Set properties
    publicationTab.topicName = topicName
    publicationTab.domainId = domainId

    // Connect topicIDLModel
    publicationTab.publicationManager.setTopicIDLModel(topicIDLModel)
    console.log("[Main] ✓ topicIDLModel connected")
    publicationTab.publicationManager.controller = controller
    console.log("[Main] ✓ controller connected to PublicationManager")

    // Trigger code generation (will use fallback if IDL not ready)
    publicationTab.publicationManager.topicName = topicName
    publicationTab.publicationManager.generateSampleCode()
    console.log("[Main] ✓ Code generation triggered")

    // Open popup
    publicationTabPopup.open()
    console.log("[Main] ✓ Publication Tab opened")
    console.log("[Main] ========================================")
}




}
