import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    // ── Data model for health conditions ──────────────────────────────
    property var healthConditions: [
        { label: "Error log entry",                          state: "Error"   },
        { label: "Warning log entry",                        state: "Warning" },
        { label: "Security error log entry",                 state: "Error"   },
        { label: "Security warning log entry",               state: "Warning" },
        { label: "Requested/offered QoS",                    state: "Error"   },
        { label: "Transport message size max",               state: "Error"   },
        { label: "Transport identifiers",                    state: "Warning" },
        { label: "No matching Publisher/Subscriber partitions", state: "Warning" },
        { label: "Keyed consistency",                        state: "Error"   },
        { label: "Data type mismatch",                       state: "Error"   },
        { label: "Writer-only Topic",                        state: "Warning" },
        { label: "Reader-only Topic",                        state: "Warning" },
        { label: "Data type representation",                 state: "Error"   },
        { label: "Trust Protection",                         state: "Error"   },
        { label: "Trust Algorithm",                          state: "Error"   }
    ]

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        // ── Header Bar ─────────────────────────────────────────────────
        Rectangle {
            id: upperRectangle
            width: parent.width
            height: 60
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            border.color: "lightgray"
            color: "#F2F2F2"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: "Health"
                    color: "darkgray"
                    bottomPadding: 15
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    Image {
                        source: "qrc:/resources/images/icons/refresh_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/left_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/dots.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                }
            }
        }

        // ── Content Area ───────────────────────────────────────────────
        Rectangle {
            id: lowerRectangle
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            color: "#F2F2F2"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 0

                // ── Column Headers ─────────────────────────────────────
                Row {
                    Layout.fillWidth: true
                    height: 36

                    Text {
                        width: parent.width * 0.62
                        text: "Health condition"
                        font.bold: true
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        height: parent.height
                    }

                    Text {
                        width: parent.width * 0.38
                        text: "Health state"
                        font.bold: true
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        height: parent.height
                    }
                }

                // ── Thin divider under headers ─────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "lightgray"
                }

                // ── Scrollable rows ────────────────────────────────────
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ListView {
                        id: conditionListView
                        anchors.fill: parent
                        model: healthConditions
                        spacing: 0
                        clip: true

                        delegate: Column {
                            width: conditionListView.width

                            Row {
                                width: parent.width
                                height: 40
                                leftPadding: 0

                                // Condition label
                                Text {
                                    width: parent.width * 0.62
                                    height: parent.height
                                    text: modelData.label
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }

                                // State ComboBox
                                ComboBox {
                                    id: stateCombo
                                    width: parent.width * 0.35
                                    height: 30
                                    anchors.verticalCenter: parent.verticalCenter

                                    model: ["Error", "Warning", "OK", "None"]
                                    currentIndex: {
                                        var idx = model.indexOf(modelData.state)
                                        return idx >= 0 ? idx : 0
                                    }

                                    background: Rectangle {
                                        radius: 4
                                        color: "white"
                                        border.color: "darkgray"
                                        border.width: 1
                                    }

                                    contentItem: Text {
                                        leftPadding: 8
                                        text: stateCombo.displayText
                                        verticalAlignment: Text.AlignVCenter
                                        color: "black"
                                    }

                                    delegate: ItemDelegate {
                                        width: stateCombo.width
                                        contentItem: Text {
                                            text: modelData
                                            color: "black"
                                            verticalAlignment: Text.AlignVCenter
                                            leftPadding: 8
                                        }
                                        highlighted: stateCombo.highlightedIndex === index
                                        background: Rectangle {
                                            color: highlighted ? "#DA3450" : "white"
                                        }
                                    }

                                    popup: Popup {
                                        y: stateCombo.height
                                        width: stateCombo.width
                                        padding: 0
                                        contentItem: ListView {
                                            implicitHeight: contentHeight
                                            model: stateCombo.delegateModel
                                            clip: true
                                        }
                                        background: Rectangle {
                                            border.color: "darkgray"
                                            border.width: 1
                                            color: "white"
                                        }
                                    }
                                }
                            }

                            // Row divider
                            Rectangle {
                                width: parent.width
                                height: 1
                                color: "lightgray"
                            }
                        }
                    }
                }
            }
        }

        // ── Bottom Bar: Restore Defaults + Apply ───────────────────────
        Rectangle {
            id: bottomBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "white"
            border.color: "#E8E9EB"
            border.width: 1

            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16
                spacing: 8

                Button {
                    id: restoreDefaultsBtn
                    text: "Restore Defaults"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: restoreDefaultsBtn.text
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        // Reset all combos to default
                        conditionListView.model = null
                        conditionListView.model = healthConditions
                    }
                }

                Button {
                    id: applyBtn
                    text: "Apply"
                    implicitWidth: 80
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: applyBtn.text
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {}
                }
            }
        }
    }
}