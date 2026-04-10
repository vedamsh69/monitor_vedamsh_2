import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4 as Controls1
import QtQuick.Controls.Styles 1.4
import "." as Local

Dialog {
    id: distributedlogger
    width: 900
    height: 850

    // Positioning relative to hosting window
    x: 0  // Removed window reference to avoid ReferenceError
    y: 0  // Removed window reference to avoid ReferenceError

    ColumnLayout {
        id: maincolumn
        anchors.fill: parent
        spacing: 10

        Label {
            id: toplabel
            text: "Distributted Logger"
            Layout.fillWidth: true
        }

        Rectangle {
            id: tabViewBorder
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: "#D4D4D4"
            border.width: 1

            Controls1.TabView {
                id: tabsfordl
                anchors.fill: parent
                anchors.margins: 1  // To prevent overlap with the border

                Controls1.Tab {
                    id: tabformessages
                    title: "Messages"
                    Rectangle {
                        anchors.fill: parent
                        color: "white"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: messagetab.open()
                        }
                        MessageTab {
                            id: messagetab
                        }
                    }
                }

                Controls1.Tab {
                    id: tabforstateandcontrol
                    title: "State and Control"
                    Rectangle {
                        anchors.fill: parent
                        color: "white"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: stateandcontroltab.open()
                        }
                        StateAndControlTab {
                            id: stateandcontroltab
                        }
                    }
                }

                Controls1.Tab {
                    id: tabforfilelogger
                    title: "File Logger"
                    Rectangle {
                        anchors.fill: parent
                        color: "white"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: fileloggertab.open()
                        }
                        FileLogger {
                            id: fileloggertab
                        }
                    }
                }

                style: TabViewStyle {
                    frameOverlap: 1
                    tab: Rectangle {
                        color: styleData.selected ? "white" : (styleData.hovered ? "#F0F0F0" : "white")
                        border.color: styleData.selected ? "gray" : "transparent"
                        border.width: styleData.selected ? 1 : 0
                        implicitWidth: Math.max(text.width + 40, 120)  // Increased tab width
                        implicitHeight: 30  // Increased tab height

                        Text {
                            id: text
                            anchors.centerIn: parent
                            text: styleData.title
                            color: "black"
                        }

                        // Blue underline for selected tab
                        Rectangle {
                            color: "blue"  // Blue underline
                            height: 2  // Height of the underline
                            width: parent.width
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                right: parent.right
                            }
                            visible: styleData.selected
                        }

                        // Grey shadow on hover
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            radius: 5
                            visible: styleData.hovered && !styleData.selected
                            border.color: "#D0D0D0"
                            border.width: 1
                        }

                        // Add right margin to create space between tabs
                        Rectangle {
                            width: 5
                            height: parent.height
                            anchors.right: parent.right
                            color: "transparent"
                        }
                    }
                    tabBar: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }
    }
}
