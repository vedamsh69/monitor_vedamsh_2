import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: advancedsettingssection
    width: 660
    height: 530
    visible: true


    Rectangle {
        id: tabViewBorder
        anchors.fill: parent
        color: "transparent"
        border.color: "#D4D4D4"
        border.width: 1

        TabView {
            id: tabView
            anchors.fill: parent
            anchors.margins: 1  // To prevent overlap with the border

            Tab {
                title: "Qos and Content Filter"
                property color textcolor: "black"
                Rectangle {
                    anchors.fill: parent
                    color: "white"

                    MouseArea
                    {
                        onClicked: qosandcontentfilter.open()
                        anchors.fill : parent
                    }

                    QosAndContentFilter
                    {
                        id : qosandcontentfilter
                    }
                }
            }
            Tab {
                title: "DataWriter Filter"

                Rectangle
                {
                    anchors.fill: parent
                    color: "white"

                    MouseArea
                    {
                        onClicked: dataWriterFilterRoot.open()
                        anchors.fill: parent

                    }

                    DataWriterFilter {
                        id: dataWriterFilterRoot
                    }
                }

            }

            Rectangle {
                id: rec
                width: parent.width
                height: 3
                color: "#D4D4D4"
            }

            style: TabViewStyle {
                frameOverlap: 1
                tab: Rectangle {
                    color: "white"  // All tabs have white background
                    border.color: styleData.selected ? "pink" : "transparent"
                    border.width: styleData.selected ? 1 : 0
                    implicitWidth: Math.max(text.width + 10, 80)
                    implicitHeight: 35
                    radius: 5

                    Text {
                        id: text
                        anchors.centerIn: parent
                        text: styleData.title
                        color: styleData.selected ? "black" : "black"
                    }

                    // Underline for selected tab
                    Rectangle {
                        color: "#c11c84"  // Color of the underline
                        height: 2  // Height of the underline
                        width: parent.width  // Width of the underline matches the tab width
                        anchors {
                            bottom: parent.bottom
                            left: parent.left
                            right: parent.right
                        }
                        visible: styleData.selected  // Only visible when tab is selected
                    }

                    // Add right margin to create space between tabs
                    Rectangle {
                        width: 5  // Adjust this value to increase/decrease space between tabs
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


