import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

Item {
    anchors.fill: parent

    property var includePaths: [
        "/usr/lib/python3.12",
        "/usr/lib/python3.12/lib-dynload",
        "/usr/local/lib/python3.12/dist-packages",
        "/usr/lib/python3/dist-packages",
        "/opt/rti.com/rti_connext_dds-7.5.0/resource/app/app_support/admin_console/x64Linux/jep/Linux/jep_3.12"
    ]

    // File dialog for browsing executable / shared library
    FileDialog {
        id: filePickerDialog
        title: "Select File"
        folder: shortcuts.home
        selectMultiple: false
        property string targetField: ""
        onAccepted: {
            var path = filePickerDialog.fileUrl.toString().replace(/^(file:\/{2})/, "")
            if (targetField === "executable") executableField.text = path
            else if (targetField === "sharedLib") sharedLibField.text = path
        }
    }

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
                    text: "Python"
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

        // ── Scrollable Content Area ────────────────────────────────────
        ScrollView {
            id: scrollView
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                width: scrollView.width
                anchors.margins: 20
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 12

                // Top margin spacer
                Item { height: 8 }

                // ── Supported Python versions ──────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Supported Python versions:"
                        color: "black"
                        font.bold: false
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        text: "3.10, 3.11, 3.12, 3.8, 3.9"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                // ── Python Executable ──────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Python Executable:"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        Layout.preferredWidth: 160
                    }

                    TextField {
                        id: executableField
                        Layout.fillWidth: true
                        height: 32
                        text: "/usr/bin/python3"
                        leftPadding: 8
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            border.color: executableField.activeFocus ? "#DA3450" : "darkgray"
                            border.width: 1
                            color: "white"
                            radius: 2
                        }
                    }

                    // Folder browse button
                    Rectangle {
                        width: 32
                        height: 32
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                        radius: 3

                        Image {
                            source: "qrc:/resources/images/icons/folder.png"
                            width: 18; height: 18
                            anchors.centerIn: parent
                            // Fallback text if icon not available
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "📁"
                            font.pixelSize: 14
                            visible: false // set true if no icon image
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                filePickerDialog.targetField = "executable"
                                filePickerDialog.open()
                            }
                        }
                    }
                }

                // ── Python Shared Library ──────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Python Shared library:"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        Layout.preferredWidth: 160
                    }

                    TextField {
                        id: sharedLibField
                        Layout.fillWidth: true
                        height: 32
                        text: "/usr/lib/x86_64-linux-gnu/libpython3.12.so.1.0"
                        leftPadding: 8
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            border.color: sharedLibField.activeFocus ? "#DA3450" : "darkgray"
                            border.width: 1
                            color: "white"
                            radius: 2
                        }
                    }

                    Rectangle {
                        width: 32
                        height: 32
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                        radius: 3

                        Text {
                            anchors.centerIn: parent
                            text: "📁"
                            font.pixelSize: 14
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                filePickerDialog.targetField = "sharedLib"
                                filePickerDialog.open()
                            }
                        }
                    }
                }

                // ── Python Version ─────────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Python version:"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        Layout.preferredWidth: 160
                    }

                    TextField {
                        id: pythonVersionField
                        width: 80
                        height: 32
                        text: "3.12"
                        leftPadding: 8
                        readOnly: true
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            border.color: "darkgray"
                            border.width: 1
                            color: "#F0F0F0"
                            radius: 2
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                // ── Connext DDS Python API ─────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Connext DDS Python API:"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        Layout.preferredWidth: 160
                    }

                    // Red status dot
                    Rectangle {
                        width: 12; height: 12
                        radius: 6
                        color: "#DA3450"
                        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                    }

                    Text {
                        text: "Not found"
                        color: "#DA3450"
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item { Layout.fillWidth: true }
                }

                // ── Info text ──────────────────────────────────────────
                Text {
                    Layout.fillWidth: true
                    text: "Information extracted from the PATH and LD_LIBRARY_PATH environment variables."
                    color: "gray"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }

                // ── Include Paths label ────────────────────────────────
                Text {
                    text: "Include Paths:"
                    color: "black"
                }

                // ── Include Paths list + action buttons ────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    // Paths list view
                    Rectangle {
                        Layout.fillWidth: true
                        height: 200
                        border.color: "darkgray"
                        border.width: 1
                        color: "white"
                        radius: 2
                        clip: true

                        ListView {
                            id: includePathsView
                            anchors.fill: parent
                            anchors.margins: 2
                            model: includePaths
                            clip: true
                            ScrollBar.vertical: ScrollBar {
                                active: includePathsView.contentHeight > includePathsView.height
                            }

                            delegate: Rectangle {
                                width: includePathsView.width
                                height: 28
                                color: includePathsView.currentIndex === index
                                       ? "#DA3450" : "transparent"

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 6
                                    anchors.right: parent.right
                                    anchors.rightMargin: 6
                                    text: modelData
                                    color: includePathsView.currentIndex === index
                                           ? "white" : "black"
                                    elide: Text.ElideRight
                                    font.pixelSize: 12
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: includePathsView.currentIndex = index
                                }
                            }
                        }
                    }

                    // Action buttons column: +, edit, ×
                    ColumnLayout {
                        spacing: 4

                        // Add (+) button — green
                        Rectangle {
                            width: 28; height: 28
                            radius: 3
                            color: "white"
                            border.color: "#4CAF50"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "+"
                                font.pixelSize: 18
                                font.bold: true
                                color: "#4CAF50"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    includePaths.push("/new/path")
                                    includePathsView.model = null
                                    includePathsView.model = includePaths
                                }
                            }
                        }

                        // Edit (pencil) button — orange/yellow
                        Rectangle {
                            width: 28; height: 28
                            radius: 3
                            color: "white"
                            border.color: "#E67E22"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "✎"
                                font.pixelSize: 14
                                color: "#E67E22"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    // Open edit dialog for selected path
                                }
                            }
                        }

                        // Remove (×) button — red
                        Rectangle {
                            width: 28; height: 28
                            radius: 3
                            color: "white"
                            border.color: "#DA3450"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "✕"
                                font.pixelSize: 13
                                font.bold: true
                                color: "#DA3450"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (includePathsView.currentIndex >= 0) {
                                        includePaths.splice(includePathsView.currentIndex, 1)
                                        includePathsView.model = null
                                        includePathsView.model = includePaths
                                        includePathsView.currentIndex = -1
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // ── Include paths note ─────────────────────────────────
                Text {
                    Layout.fillWidth: true
                    text: "Include paths will be used to find Python modules in your system."
                    color: "gray"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }

                Text {
                    Layout.fillWidth: true
                    text: "NOTE: Changes to include paths can only be done before creating a publication."
                    color: "gray"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }

                // ── Spacer ─────────────────────────────────────────────
                Item { height: 10 }

                // ── Publications status ────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "Publications status:"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                    }

                    Rectangle {
                        width: 12; height: 12
                        radius: 6
                        color: "#DA3450"
                    }

                    Text {
                        text: "Disabled"
                        color: "#E67E22"
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item { Layout.fillWidth: true }
                }

                // ── Fix info text with link ────────────────────────────
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: "black"
                    font.pixelSize: 12
                    textFormat: Text.RichText
                    text: "For more information on how to fix your Python configuration, please see the following <a href='#' style='color:#E67E22;'>link</a>."
                    onLinkActivated: Qt.openUrlExternally("https://community.rti.com")
                }

                // ── Red error box ──────────────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: errorColumn.implicitHeight + 20
                    border.color: "#DA3450"
                    border.width: 1
                    color: "white"
                    radius: 2

                    ColumnLayout {
                        id: errorColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Text {
                            Layout.fillWidth: true
                            text: "We weren't able to fill some fields, publications have been disabled."
                            color: "#DA3450"
                            wrapMode: Text.WordWrap
                            font.pixelSize: 12
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "In order to fix them please follow these instructions:"
                            color: "#DA3450"
                            wrapMode: Text.WordWrap
                            font.pixelSize: 12
                        }

                        // Bullet points
                        Repeater {
                            model: [
                                "Please add a python/python3 executable to your PATH variable.",
                                "Please add a python shared library (libpythonX.Y.so.1.0) to your LD_LIBRARY_PATH variable.",
                                "After that you would need to restart Admin Console in order for changes to take effect."
                            ]

                            delegate: RowLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Text {
                                    text: "•"
                                    color: "#DA3450"
                                    font.pixelSize: 14
                                    Layout.leftMargin: 10
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData
                                    color: "#DA3450"
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }

                // ── Tutorial link text ─────────────────────────────────
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: "black"
                    font.pixelSize: 12
                    text: "For more information on configuring Admin Console to use the Publication View, please see"
                }

                Text {
                    text: "Tutorial Step 11: Publishing to a topic"
                    color: "#E67E22"
                    font.pixelSize: 12
                    font.underline: true

                   
                   MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://community.rti.com")
                    }
                }

                // Bottom margin spacer
                Item { height: 16 }
            }
        }

        // ── Bottom Bar: Apply only ─────────────────────────────────────
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