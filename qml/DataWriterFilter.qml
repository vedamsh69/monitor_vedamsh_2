import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: dataWriterFilterRoot
    anchors.fill: parent

    ColumnLayout {
        id: dataWriterFiltercol
        anchors.fill: parent
        anchors.margins: 10  // Add some margin around the layout

        CheckBox {
            id: enableDataWriterFilter
            text: "Enable DataWriter Filter"
            Layout.fillWidth: true
            scale: 0.9
            transformOrigin: Item.Left  // Ensure scaling happens from the left side
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 10
            enabled: enableDataWriterFilter.checked  // This enables/disables all child items

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Label {
                    text: "Select DataWriters from which to receive Data"
                    Layout.fillWidth: true
                }

                Item {
                    id: item3
                    width: 20
                    height: 20

                    Image {
                        source: "qrc:/resources/images/icons/checkimages.png"
                        anchors.fill: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("checkall checkbox button clicked")
                        }
                    }
                }

                Item {
                    id: item4
                    width: 20
                    height: 20

                    Image {
                        source: "qrc:/resources/images/icons/unimages.png"
                        anchors.fill: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("unall checkbox button clicked")
                        }
                    }
                }
            }

            DatafilterTable {
                id: datafiltertable
            }
        }

        Item {
            Layout.fillHeight: true  // This will push all items to the top
        }
    }
}





