import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15

Dialog {
    id: myDialog
    height: parent.width * 1.5
    width: parent.width * 0.7
    title: "Preferences"
    property string selectedDomain: ""
    function getUpdatedDomain() {
        var domainNumber = selectedDomain.split(" ")[0]
        return domainNumber + " " + domainComboBox.currentText
    }
    contentItem: Item {
        id: rootitem

        ColumnLayout {
            id: mainColumn
            anchors.fill: parent
            spacing: 10
            anchors.margins: 20

            Text {
                id: qosProfileText
                text: qsTr("QoS Profile")
                //font.pixelSize: 10
            }

            ComboBox {
                id: domainComboBox
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                model: ListModel {
                    id: myComboBoxListModel
                    ListElement { key: "AdminConsole::Default (default)" }
                    ListElement { key: "AdminConsole::RealTimeWAN" }
                }


                currentIndex: 0

                Component.onCompleted: {
                    // Set the current index based on the selected domain
                    for (var i = 0; i < model.count; i++) {
                        if (model.get(i).key === selectedDomain.split(" ").slice(1).join(" ")) {
                            currentIndex = i;
                            break;
                        }
                    }
                }
                onActivated: {
                    if (currentText === "AdminConsole::RealTimeWAN") {
                        realTimeWANDialog.open()
                    }
                }
                background: Rectangle {
                    color: "white"
                    radius: 5
                    border.color: "#E8E9EB"
                    border.width: 1
                }
            }

            Text {
                id: partitionsText
                text: qsTr("DomainParticipant partitions")
                //font.pixelSize: 10
                Layout.topMargin: 20
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "black"
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        clip: true

                        ListView {
                            id: partitionScrollView
                            anchors.fill: parent
                            model: ListModel {}
                            delegate: ItemDelegate {
                                width: partitionScrollView.width
                                height: 40
                                text: model.content

                                contentItem: Text {
                                    text: model.content
                                    color: "black"
                                    //font.pixelSize: 10
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 10
                                }

                                background: Rectangle {
                                    color: highlighted ? "#DA3450" : "transparent"
                                    border.color: "#E8E9EB"
                                    border.width: 1
                                }

                                highlighted: ListView.isCurrentItem

                                onClicked: {
                                    partitionScrollView.currentIndex = index
                                }
                            }

                            highlight: Rectangle {
                                color: "#DA3450"
                                opacity: 0.5
                            }
                            highlightFollowsCurrentItem: true
                        }
                    }
                }

                ColumnLayout {
                    spacing: 10
                    Layout.alignment: Qt.AlignTop

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/plus.png"
                            width: 15
                            height: 15
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                addPartitionDialog.open()
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/pencil.png"
                            width: 24
                            height: 24
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: partitionScrollView.currentIndex !== -1
                            onClicked: {
                                editPartitionDialog.setContent(partitionScrollView.model.get(partitionScrollView.currentIndex).content)
                                editPartitionDialog.open()
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/close.png"
                            width: 15
                            height: 15
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: partitionScrollView.currentIndex !== -1
                            onClicked: {
                                partitionScrollView.model.remove(partitionScrollView.currentIndex)
                            }
                        }
                    }
                }
            }

            Text {
                id: discoveryText
                text: qsTr("Discovery Peers")
                //font.pixelSize: 10
                Layout.topMargin: 20
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "black"
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        clip: true

                        ListView {
                            id: peersScrollView
                            anchors.fill: parent
                            model: ListModel {}
                            delegate: ItemDelegate {
                                width: peersScrollView.width
                                height: 40
                                text: model.content

                                contentItem: Text {
                                    text: model.content
                                    color: "black"
                                    //font.pixelSize: 10
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 10
                                }

                                background: Rectangle {
                                    color: highlighted ? "#DA3450" : "transparent"
                                    border.color: "#E8E9EB"
                                    border.width: 1
                                }

                                highlighted: ListView.isCurrentItem

                                onClicked: {
                                    peersScrollView.currentIndex = index
                                }
                            }

                            highlight: Rectangle {
                                color: "#DA3450"
                                opacity: 0.5
                            }
                            highlightFollowsCurrentItem: true
                        }
                    }
                }

                ColumnLayout {
                    spacing: 10
                    Layout.alignment: Qt.AlignTop

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/plus.png"
                            width: 15
                            height: 15
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                addPeerDialog.openCentered()
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/pencil.png"
                            width: 24
                            height: 24
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: peersScrollView.currentIndex !== -1
                            onClicked: {
                                editPeerDialog.setContent(peersScrollView.model.get(peersScrollView.currentIndex).content)
                                editPeerDialog.open()
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 30
                        border.color: "#CCCCCC"
                        border.width: 1
                        radius: 4

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/resources/images/icons/close.png"
                            width: 15
                            height: 15
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: peersScrollView.currentIndex !== -1
                            onClicked: {
                                peersScrollView.model.remove(peersScrollView.currentIndex)
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            // Button rectangle (unchanged)
            Rectangle {
                id: buttonRectangle
                Layout.fillWidth: true
                height: 70
                color: "white"

                RowLayout {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 18
                    spacing: 5

                    Button {
                        text: "Cancel"
                        implicitWidth: 130
                        implicitHeight: 35

                        background: Rectangle {
                            color: "white"
                            radius: 5
                            border.width: 0.5
                            border.color: "#E8E9EB"
                        }

                        contentItem: Text {
                            text: parent.text
                            //font.pixelSize: 10
                            color: "gray"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }

                        onClicked: myDialog.reject()
                    }

                    Button {
                        text: "Apply"
                        implicitWidth: 130
                        implicitHeight: 35

                        background: Rectangle {
                            color: "#249225"
                            radius: 5
                        }

                        contentItem: Text {
                            text: parent.text
                            //font.pixelSize: 10
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }

                        onClicked: {
                            myDialog.accept()
                        }
                    }
                }
            }
        }
    }

    // Remove the standard buttons
    standardButtons: Dialog.NoButton

    // Add Partition Dialog
    Dialog {
        id: addPartitionDialog
        title: "Add"
        width: parent.width * 0.8  // Adjusted to be more reasonable
        height: parent.height * 0.3  // Adjusted to fit content better

        // Remove standard buttons
        standardButtons: Dialog.NoButton

        // Function to open and center the dialog
        function openCentered() {
            x = (parent.width - width) / 2
            y = (parent.height - height) / 2
            open()
        }

        contentItem: ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            Label {
                id: partitionLabel
                text: "Add Partition"
                //font.pointSize: 10
                Layout.fillWidth: true
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 10
            }

            TextField {
                id: addPartitionTextField
                Layout.fillWidth: true
                background: Rectangle {
                    radius: 5
                    border.color: "#E8E9EB"
                    border.width: 1
                }
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: partitionLabel.bottom
                anchors.topMargin: 10
            }

            Text {
                id: partitionText
                text: qsTr("DomainParticipant partitions can contain any kind of text ({empty} will be considered as \"\" partition).")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                //font.pointSize: 10
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: addPartitionTextField.bottom
                anchors.topMargin: 50
            }

            Item { Layout.fillHeight: true } // Spacer

            // Custom buttons
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                spacing: 10

                Button {
                    text: "Cancel"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.width: 0.5
                        border.color: "#E8E9EB"
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "gray"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: addPartitionDialog.reject()
                }

                Button {
                    text: "OK"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "#249225"
                        radius: 5
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: {
                        if (addPartitionTextField.text.trim() !== "") {
                            partitionScrollView.model.append({content: addPartitionTextField.text.trim()})
                            addPartitionTextField.text = ""
                            addPartitionDialog.accept()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: editPartitionDialog
        title: "Edit"
        width: parent.width * 0.8
        height: parent.height * 0.3

        // Center the dialog
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        // Remove standard buttons
        standardButtons: Dialog.NoButton

        property string content: ""

        contentItem: ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            Label {
                id: editPartitionLabel
                text: "Edit Partition"
                //font.pointSize: 10
                Layout.fillWidth: true
            }

            TextField {
                id: editPartitionTextField
                Layout.fillWidth: true
                background: Rectangle {
                    radius: 5
                    border.color: "#E8E9EB"
                    border.width: 1
                }
            }

            Text {
                id: editPartitionText
                text: qsTr("DomainParticipant partitions can contain any kind of text ({empty} will be considered as \"\" partition).")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                //font.pointSize: 10
            }

            Item { Layout.fillHeight: true } // Spacer

            // Custom buttons
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                spacing: 10

                Button {
                    text: "Cancel"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.width: 0.5
                        border.color: "#E8E9EB"
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "gray"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: editPartitionDialog.reject()
                }

                Button {
                    text: "OK"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "#249225"
                        radius: 5
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: {
                        if (editPartitionTextField.text.trim() !== "") {
                            partitionScrollView.model.set(partitionScrollView.currentIndex, {content: editPartitionTextField.text.trim()})
                            editPartitionDialog.accept()
                        }
                    }
                }
            }
        }

        // Use a function to set the content
        function setContent(newContent) {
            content = newContent
            editPartitionTextField.text = newContent
        }
    }

    // Add Peer Dialog
    Dialog {
        id: addPeerDialog
        title: "Add"
        width: parent.width * 0.8
        height: parent.height * 0.3

        // Remove standard buttons
        standardButtons: Dialog.NoButton

        // Function to open and center the dialog
        function openCentered() {
            x = (parent.width - width) / 2
            y = (parent.height - height) / 2
            open()
        }

        contentItem: ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            Label {
                id: peerLabel
                text: "Add discovery Peer"
                //font.pointSize: 10
                Layout.fillWidth: true
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 10
            }

            TextField {
                id: addPeerTextField
                Layout.fillWidth: true
                background: Rectangle {
                    radius: 5
                    border.color: "#E8E9EB"
                    border.width: 1
                }
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: peerLabel.bottom
                anchors.topMargin: 10
            }

            Text {
                id: peerText
                text: qsTr("Peer descriptor format examples: '192.168.10.101', '20@builtin.udpv4://localhost',
'somewhere.somewhere''")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                //font.pointSize: 10
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: addPeerTextField.bottom
                anchors.topMargin: 50
            }

            Item { Layout.fillHeight: true } // Spacer

            // Custom buttons
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                spacing: 10

                Button {
                    text: "Cancel"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.width: 0.5
                        border.color: "#E8E9EB"
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "gray"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: addPeerDialog.reject()
                }

                Button {
                    text: "OK"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "#249225"
                        radius: 5
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: {
                        if (addPeerTextField.text.trim() !== "") {
                            peersScrollView.model.append({content: addPeerTextField.text.trim()})
                            addPeerTextField.text = ""
                            addPeerDialog.accept()
                        }
                    }
                }
            }
        }
    }

    // Edit Peer Dialog
    Dialog {
        id: editPeerDialog
        title: "Edit"
        width: parent.width * 0.8
        height: parent.height * 0.3

        // Center the dialog
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        // Remove standard buttons
        standardButtons: Dialog.NoButton

        property string content: ""

        contentItem: ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            Label {
                id: editPeerLabel
                text: "Edit discovery Peer"
                //font.pointSize: 10
                Layout.fillWidth: true
            }

            TextField {
                id: editPeerTextField
                Layout.fillWidth: true
                background: Rectangle {
                    radius: 5
                    border.color: "#E8E9EB"
                    border.width: 1
                }
            }

            Text {
                id: editPeerText
                text: qsTr("Peer descriptor format examples: '192.168.10.101', '20@builtin.udpv4://localhost',
'somewhere.somewhere'/")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                //font.pointSize: 10
            }

            Item { Layout.fillHeight: true } // Spacer

            // Custom buttons
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                spacing: 10

                Button {
                    text: "Cancel"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.width: 0.5
                        border.color: "#E8E9EB"
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "gray"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: editPeerDialog.reject()
                }

                Button {
                    text: "OK"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        color: "#249225"
                        radius: 5
                    }

                    contentItem: Text {
                        text: parent.text
                        //font.pixelSize: 10
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onClicked: {
                        if (editPeerTextField.text.trim() !== "") {
                            peersScrollView.model.set(peersScrollView.currentIndex, {content: editPeerTextField.text.trim()})
                            editPeerDialog.accept()
                        }
                    }
                }
            }
        }

        // Use a function to set the content
        function setContent(newContent) {
            content = newContent
            editPeerTextField.text = newContent
        }
    }
}
