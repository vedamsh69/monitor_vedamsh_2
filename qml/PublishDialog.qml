import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0

Dialog {
    id: publishDialogid
    modal: true
    visible: false
    property int domainnumber: 0
        property string topicname: ""
            property string selectedTopic: ""

                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 720

    property int defaultSpacing: 10
    onVisibleChanged: {
        if (visible) {
            console.log("[PublishDialog] OPENED for domain=" + publishDialogid.domainnumber + " topic=" + publishDialogid.topicname)
        } else {
            console.log("[PublishDialog] CLOSED")
        }
    }

                    Column {
                        id: col
                        anchors.fill: parent
                        anchors.margins: defaultSpacing
                        spacing: defaultSpacing
                        property real cellwidth: col.width / 3 - spacing

                            Item {
                                width: parent.width
                                height: 80

                                Label {
                                    id: subscriptionlabel
                                    text: "Create Publication"
                                    anchors.left: parent.left
                                }

                                Image {
                                    source: "qrc:/resources/images/icons/create_subscription_icon.png"
                                    width: 50
                                    height: 50
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                }

                                Label {
                                    text: "Publish to topic Domain " + publishDialogid.domainnumber + ": " + publishDialogid.topicname
                                    visible: true
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.topMargin: 60
                                    width: parent.width - 60
                                    wrapMode: Text.WordWrap
                                }
                            }

                            Rectangle {
                                id: rec
                                width: parent.width
                                height: 1
                                color: "#D4D4D4"
                            }

                            Column {
                                id: col2
                                spacing: defaultSpacing

                                Label {
                                    text: "Data Type"
                                }

                                Grid {
                                    id: grid
                                    columns: 2
                                    spacing: defaultSpacing
                                    width: parent.width

                                    TextField {
                                        id: textbox
                                        height: 25
                                        width: 480
                                        placeholderText: "Filter Data Types"
                                    }

                                    CheckBox {
                                        height: 25
                                        text: "Hide irrelevant types"
                                    }
                                }

                                Grid {
                                    id: grid2
                                    columns: 2
                                    spacing: defaultSpacing
                                    width: parent.width

                                    ComboBox {
                                        id: combobox
                                        model: ["", publishDialogid.topicname]
                                        width: 500
                                        height: 30
                                        onCurrentIndexChanged: {
                                            if (currentIndex !== -1)
                                            {
                                                publishDialogid.selectedTopic = model[currentIndex]
                                            } else {
                                            publishDialogid.selectedTopic = ""
                                        }
                                    }
                                }
                            }

                            Text {
                                id: warningText
                                text: "Please select a topic from the DropDown"
                                color: "red"
                                visible: publishDialogid.selectedTopic === ""
                            }
                        }
                    }

                    footer: DialogButtonBox {
                        Button {
                            text: qsTr("OK")
                            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                            enabled: publishDialogid.selectedTopic !== ""
                        }
                        Button {
                            text: qsTr("Cancel")
                            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                        }
                    }

                    // ========== UPDATED: Discovery-First Approach ==========
                    onAccepted: {
                        console.log("[PublishDialog::onAccepted] ========================================")
                        console.log("[PublishDialog::onAccepted] OK Button clicked - Dialog accepted")
                        console.log("[PublishDialog::onAccepted] Selected topic: " + publishDialogid.selectedTopic)
                        console.log("[PublishDialog::onAccepted] Domain ID: " + publishDialogid.domainnumber)
                        console.log("[PublishDialog::onAccepted] ========================================")

                        // Start discovery process (will open Publication Tab when ready)
                        console.log("[PublishDialog::onAccepted] Starting type discovery...")
                        console.log("[PublishDialog::onAccepted] Calling controller.startPublisherWithDiscovery()...")
                        controller.startPublisherWithDiscovery(publishDialogid.selectedTopic, publishDialogid.domainnumber)

                        console.log("[PublishDialog::onAccepted] ✓ Discovery initiated, waiting for type discovery signal...")
                        console.log("[PublishDialog::onAccepted] ========================================")
                    }

                    onRejected: {
                        console.log("[PublishDialog::onRejected] ==================================================")
                        console.log("[PublishDialog::onRejected] CANCEL button clicked - Dialog rejected")
                        console.log("[PublishDialog::onRejected] ==================================================")
                    }

                    // ========== NEW: Handle Discovery Results ==========
                    Connections {
                        target: controller

                        function onPublisherTypeDiscovered(topicName)
                        {
                            console.log("[PublishDialog::TypeDiscovered] ========================================")
                            console.log("[PublishDialog::TypeDiscovered] onPublisherTypeDiscovered() signal RECEIVED")
                            console.log("[PublishDialog::TypeDiscovered] Topic name from signal: " + topicName)
                            console.log("[PublishDialog::TypeDiscovered] Our selected topic: " + publishDialogid.selectedTopic)

                            // Check if this signal is for our topic
                            if (topicName === publishDialogid.selectedTopic)
                            {
                                console.log("[PublishDialog::TypeDiscovered] ✓ MATCH! This is our topic")
                                console.log("[PublishDialog::TypeDiscovered] Type discovery SUCCESS!")

                                if (typeof topicIDLModel !== 'undefined')
                                {
                                    console.log("[PublishDialog::TypeDiscovered] topicIDLModel text data length: " + topicIDLModel.textData.length)
                                } else {
                                console.log("[PublishDialog::TypeDiscovered] ⚠ topicIDLModel not accessible")
                            }

                            // Now open Publication Tab with discovered type
                            if (typeof mainApplicationView !== 'undefined')
                            {
                                console.log("[PublishDialog::TypeDiscovered] Opening Publication Tab...")
                                mainApplicationView.openPublicationTab(topicName, publishDialogid.domainnumber)
                                console.log("[PublishDialog::TypeDiscovered] ✓ Publication Tab opened successfully")
                            } else {
                            console.error("[PublishDialog::TypeDiscovered] ✗ mainApplicationView not accessible!")
                        }
                    } else {
                    console.log("[PublishDialog::TypeDiscovered] Not our topic, ignoring signal (expected " + publishDialogid.selectedTopic + ", got " + topicName + ")")
                }
                console.log("[PublishDialog::TypeDiscovered] ========================================")
            }

            function onPublisherDiscoveryFailed(topicName, reason)
            {
                console.log("[PublishDialog::DiscoveryFailed] ========================================")
                console.log("[PublishDialog::DiscoveryFailed] onPublisherDiscoveryFailed() signal RECEIVED")
                console.log("[PublishDialog::DiscoveryFailed] Topic name from signal: " + topicName)
                console.log("[PublishDialog::DiscoveryFailed] Reason: " + reason)

                // Check if this signal is for our topic
                if (topicName === publishDialogid.selectedTopic)
                {
                    console.error("[PublishDialog::DiscoveryFailed] ✗ CRITICAL: Discovery failed for our topic!")

                    // Show error dialog
                    if (typeof errorDialog !== 'undefined')
                    {
                        console.log("[PublishDialog::DiscoveryFailed] Showing error dialog...")
                        errorDialog.text = "⚠ Failed to discover topic type structure:\n\n" +
                        reason +
                        "\n\nPlease ensure:\n" +
                        "• At least one publisher exists for this topic\n" +
                        "• The publisher is active and discoverable\n" +
                        "• Topic names match exactly (case-sensitive)"
                        errorDialog.open()
                    } else {
                    console.error("[PublishDialog::DiscoveryFailed] errorDialog not available, showing console message")
                    console.error("[PublishDialog::DiscoveryFailed] Error: " + reason)
                }
            }
            console.log("[PublishDialog::DiscoveryFailed] ========================================")
        }
    }
}
