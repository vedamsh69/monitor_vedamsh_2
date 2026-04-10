import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "." as CurrentDirectory

Item {
    anchors.fill: parent

    CurrentDirectory.RealTimeWANDialog {
        id: realTimeWANDialog
    }

    function openEditDomainParticipantDialog() {
        var component = Qt.createComponent("EditDomainParticipantDialog.qml")
        if (component.status === Component.Ready) {
            var selectedDomain = selectedDomains[domainViewField.currentIndex]
            var dialog = component.createObject(mainRectangle, {
                                                    "selectedDomain": selectedDomain
                                                })
            if (dialog !== null) {
                dialog.open()
                dialog.accepted.connect(function() {
                    var newDomain = dialog.getUpdatedDomain()
                    selectedDomains[domainViewField.currentIndex] = newDomain
                    domainViewField.model = null  // Force update
                    domainViewField.model = selectedDomains
                })
            } else {
                console.error("Error creating dialog")
            }
        } else if (component.status === Component.Error) {
            console.error("Error loading component:", component.errorString())
        }
    }
    function showAlert(message) {
        alertText.text = message
        alertPopup.open()
    }

    Popup {
        id: alertPopup
        width: 550
        height: 280 // Increased height to accommodate the button
        modal: true
        anchors.centerIn: parent
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 10
            color: "white"
        }

        contentItem: Text {
            id: alertText

            ColumnLayout {
                id: mainPopupColumn
                anchors.fill: parent
                spacing: 0

                Row {
                    id: popupRow
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Rectangle {
                        id: rectangle
                        width: parent.width * 0.25
                        height: parent.height

                        Image {
                            source: "qrc:/resources/images/icons/danger.png"
                            width: 50
                            height: 50
                            anchors.left: parent.left
                            anchors.leftMargin: 30
                            anchors.top: parent.top
                            anchors.topMargin: 20
                        }
                    }
                    Rectangle {
                        id: rightRectangle
                        width: parent.width * 0.75
                        height: parent.height

                        TextArea {
                            id: popupTextHeading
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 1 // Increase this value to increase the gap
                            //font.pointSize: 10
                            wrapMode: TextArea.Wrap
                            readOnly: true
                            background: null
                            selectByMouse: false
                            font.bold: true
                            text: "Invalid domain range input"
                        }
                        TextArea {
                            id: popupText
                            anchors.top: popupTextHeading.bottom

                            anchors.topMargin: 5 // Increase this value to increase the gap
                            anchors.left: parent.left
                            anchors.right: parent.right
                            //font.pointSize: 10
                            wrapMode: TextArea.Wrap
                            readOnly: true
                            background: null
                            selectByMouse: false
                            text: "Unable to understand the requested range of domains.

Note,
You may list domain ids, separated  by commas (e.g. 1,2,3,4,5 )
enter a range of domains (e.g. 0-5 )
or enter a combination of two formats (e.g. 1-2, 3, 4-5)
"
                        }
                    }
                }
            }
        }
    }

    function updateDomainViewField() {
        domainViewField.text = selectedDomains.join("\n")
    }

    property var selectedDomains: ["0 AdminConsole::Default (default)"]

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        folder: shortcuts.home
        nameFilters: ["(*.xml)", "(*.txt)"]
        selectMultiple: true

        onAccepted: {
            var filePaths = fileDialog.fileUrls.map(function (url) {
                return url.toString().replace(/^(file:\/{2})/, "")
            })
            for (var i = 0; i < filePaths.length; i++) {
                if (filesViewField.text === "") {
                    filesViewField.text = filePaths[i]
                } else {
                    filesViewField.text += "\n" + filePaths[i]
                }
            }
        }
    }

    Rectangle {
        color: "#F2F2F2"
        id: mainRectangle
        anchors.fill: parent

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
                    text: "Administration"
                    //font.pointSize: 10
                    bottomPadding: 15
                    color: darkgray
                }

                Item {
                    Layout.fillWidth: true
                }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    Image {
                        source: "qrc:/resources/images/icons/left_arrow.png"
                        width: 20
                        height: 20
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                // Handle left arrow click
                            }
                        }
                    }

                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20
                        height: 20
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                // Handle down arrow click
                            }
                        }
                    }

                    Image {
                        source: "qrc:/resources/images/icons/right_arrow.png"
                        width: 20
                        height: 20
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                // Handle right arrow click
                            }
                        }
                    }

                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20
                        height: 20
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                // Handle second down arrow click
                            }
                        }
                    }

                    Image {
                        source: "qrc:/resources/images/icons/dots.png"
                        width: 20
                        height: 20
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {

                                // Handle dots click
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            id: lowerRectangle
            width: parent.width
            height: parent.height - upperRectangle.height
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            border.color: "lightgray"
            color: "#F2F2F2"

            ColumnLayout {
                anchors.fill: parent
                spacing: 20
                anchors.margins: 20
                height: parent.height * 0.95
                Rectangle {
                    id: cleanupField
                    Layout.fillWidth: true
                    height: 60
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        id: cleanupLabel
                        text: "Clean up not present entities"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 2
                        spacing: 2.1

                        Image {
                            id: broomImage
                            source: "qrc:/resources/images/icons/broom.png"
                            width: 35
                            height: 35
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {

                                    // Handle broom click
                                }
                            }
                        }

                        Text {
                            padding: -5
                            id: cleanuptext
                            text: "The period (in seconds) to automatically cleanup"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: broomImage.right
                            //font.pointSize: 10
                            elide: Text.ElideRight
                            width: parent.width - broomImage.width
                                   - cleanupPeriodField.width - parent.spacing * 3
                            wrapMode: Text.WordWrap
                        }

                        TextField {
                            id: cleanupPeriodField
                            width: 140
                            height: 35
                            text: "30"
                            leftPadding: 10 // Add left padding for the text
                            rightPadding: 60 // Make room for the buttons on the right
                            validator: IntValidator {
                                bottom: 1
                                top: 3600
                            }
                            horizontalAlignment: TextInput.AlignLeft
                            verticalAlignment: TextInput.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: cleanuptext.right
                            font.bold: true

                            background: Rectangle {
                                radius: 5
                                border.color: "gray"
                                border.width: 1
                            }

                            Row {
                                anchors.right: parent.right
                                spacing: 0
                                anchors.verticalCenter: parent.verticalCenter
                                Rectangle {
                                    width: 30
                                    height: parent.parent.height
                                    color: "transparent"
                                    border.color: "gray"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "+"
                                        //font.pointSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var value = parseInt(
                                                        cleanupPeriodField.text)
                                                    || 0
                                            cleanupPeriodField.text = Math.min(
                                                        value + 1,
                                                        3600).toString()
                                        }
                                    }
                                }
                                Rectangle {
                                    width: 30
                                    height: parent.parent.height
                                    color: "transparent"
                                    // border.color: "gray"
                                    // border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "-"
                                        //font.pointSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var value = parseInt(
                                                        cleanupPeriodField.text)
                                                    || 0
                                            cleanupPeriodField.text = Math.max(
                                                        value - 1, 1).toString()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: graphField
                    Layout.fillWidth: true
                    height: 60
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        id: graphLabel
                        text: "Graph update period"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 2
                        spacing: 2.1

                        Text {
                            id: graphtext
                            text: "The period (in seconds) to automatically update"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            //font.pointSize: 10
                            anchors.leftMargin: 10
                            elide: Text.ElideRight
                            width: parent.width - graphPeriodField.width - parent.spacing * 3
                            wrapMode: Text.WordWrap
                        }

                        TextField {
                            id: graphPeriodField
                            width: 160
                            height: 35
                            text: "5"
                            leftPadding: 10
                            rightPadding: 60
                            validator: IntValidator {
                                bottom: 1
                                top: 3600
                            }
                            horizontalAlignment: TextInput.AlignLeft
                            verticalAlignment: TextInput.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            font.bold: true

                            background: Rectangle {
                                radius: 5
                                border.color: "gray"
                                border.width: 1
                            }

                            Row {
                                anchors.right: parent.right
                                spacing: 0
                                anchors.verticalCenter: parent.verticalCenter
                                Rectangle {
                                    width: 30
                                    height: parent.parent.height
                                    color: "transparent"
                                    border.color: "gray"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "+"
                                        //font.pointSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var value = parseInt(graphPeriodField.text) || 0
                                            graphPeriodField.text = Math.min(value + 1, 3600).toString()
                                        }
                                    }
                                }
                                Rectangle {
                                    width: 30
                                    height: parent.parent.height
                                    color: "transparent"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "-"
                                        //font.pointSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var value = parseInt(graphPeriodField.text) || 0
                                            graphPeriodField.text = Math.max(value - 1, 1).toString()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: domainField
                    Layout.fillWidth: true
                    height: 550 // Increased height to accommodate the text
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        id: domainLabel
                        text: "Domains"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        anchors.bottomMargin: 10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    TextArea {
                        id: domainText
                        anchors.topMargin: 1 // Increase this value to increase the gap
                        anchors.top: domainLabel.bottom // Anchor to the bottom of the label
                        anchors.left: parent.left
                        anchors.right: parent.right
                        //font.pointSize: 10
                        wrapMode: TextArea.Wrap
                        readOnly: true
                        background: null
                        selectByMouse: false
                        text: "Distributed system that comply with the OMG Data Distribution Service
standard (DDS), may be divided into domains.
Please specify the DDS domain IDs, used by applications that you would like
the Administration Console to monitor and control

For each domain you may specify specific communciation characterisitics
called Quality of Service (QoS).

Select a QoS profile to use when joining a domain"
                    }

                    ComboBox {
                        id: domainComboBox
                        anchors.top: domainText.bottom
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.margins: 10
                        height: 40
                        model: ListModel {
                            id: myComboBoxListModel
                            ListElement {
                                key: "AdminConsole::Default (default)"
                            }
                            ListElement {
                                key: "AdminConsole::RealTimeWAN"
                            }
                        }
                        currentIndex: 0

                        onActivated: {
                            if (currentText === "AdminConsole::RealTimeWAN") {
                                realTimeWANDialog.open()
                            }
                        }

                        background: Rectangle {
                            anchors.fill: parent
                            color: "white"
                            radius: 5
                            border.color: "darkgray"
                            border.width: 1
                        }
                    }

                    Button {
                        id: domainButton
                        height: 40
                        anchors.top: domainText.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.right: parent.right
                        anchors.margins: 10

                        text: "Apply to currently joined domains"
                        padding: -50

                        background: Rectangle {
                            radius: 5
                            border.color: "darkgray"
                            border.width: 1
                        }
                        onClicked: {
                            var selectedQoS = domainComboBox.currentText
                            var updatedDomains = []
                            for (var i = 0; i < selectedDomains.length; i++) {
                                var domainParts = selectedDomains[i].split(" ")
                                var domainNumber = domainParts[0]
                                updatedDomains.push(domainNumber + " " + selectedQoS)
                            }
                            selectedDomains = updatedDomains
                            domainViewField.model = selectedDomains
                        }
                    }
                    ButtonGroup {
                        id: radioButtonGroup
                        exclusive: true
                        buttons: [radioButton1, radioButton2]
                    }

                    RadioButton {
                        id: radioButton1
                        text: "Automatically discover and join domains"
                        anchors.top: domainComboBox.bottom
                        //font.pointSize: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.topMargin: 40
                        checked: true
                        indicator: Rectangle {
                            id: customButton1Indicator
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: width / 2
                            x: 10
                            y: 8
                            border.color: "#34495e"
                            border.width: 1
                            color: radioButton1.checked ? "#DA3450" : "transparent"
                            anchors.margins: 10
                        }
                    }
                    RadioButton {
                        id: radioButton2
                        text: "Manually join and leave domains"
                        anchors.top: radioButton1.bottom
                        //font.pointSize: 10
                        anchors.left: parent.left
                        anchors.right: parent.right

                        indicator: Rectangle {
                            id: customButton2Indicator
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: width / 2
                            x: 10
                            y: 8
                            border.color: "#34495e"
                            border.width: 1
                            color: radioButton2.checked ? "#DA3450" : "transparent"
                            anchors.margins: 10
                        }
                    }

                    Text {
                        id: domainTextField
                        text: qsTr("Please specify the domains to be joined:")
                        anchors.top: radioButton2.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 15
                        visible: radioButton2.checked
                    }

                    TextField {
                        id: textFieldDomain
                        width: parent.width * 0.55
                        height: 35
                        anchors.top: domainTextField.bottom
                        anchors.left: parent.left
                        anchors.margins: 10
                        color: "#000000"
                        readOnly: !radioButton2.checked // Make it read-only when radioButton2 is not checked

                        background: Rectangle {
                            border.color: textFieldDomain.activeFocus
                                          && !textFieldDomain.readOnly ? "#DA3450" : "darkgray"
                            border.width: 1
                            radius: 5
                            color: textFieldDomain.readOnly ? "#F0F0F0" : "white" // Change background color when read-only
                        }
                    }
                    Button {
                        id: joinDomainButton
                        height: 35
                        width: parent.width * 0.40
                        anchors.top: domainTextField.bottom
                        anchors.right: parent.right
                        anchors.margins: 10
                        enabled: radioButton2.checked // Enable only when radioButton2 is checked
                        text: "Join Domains"

                        // padding: -50
                        background: Rectangle {
                            border.color: "darkgray"
                            border.width: 1
                            radius: 5
                            color: textFieldDomain.readOnly ? "#F0F0F0" : "white"
                        }
                        contentItem: Text {
                            text: joinDomainButton.text
                            color: textFieldDomain.readOnly ? "darkgray" : "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            var domainInput = textFieldDomain.text.trim()
                            
                            if (/^\d+$/.test(domainInput)) {
                                myDialog.domainInput = domainInput
                                var selectedQoS = domainComboBox.currentText
                                var newDomain = domainInput + " " + selectedQoS
                                if (!selectedDomains.includes(newDomain)) {
                                    selectedDomains.push(newDomain)
                                    domainViewField.model = selectedDomains
                                }
                                textFieldDomain.text = ""

                            } else {
                                showAlert("Invalid domain. Please enter only numbers.")
                            }
                        }
                    }
                    Rectangle {
                        id: domainViewFieldContainer
                        width: parent.width * 0.55
                        height: 100
                        anchors.top: textFieldDomain.bottom
                        anchors.left: parent.left
                        anchors.margins: 10
                        border.color: "darkgray"
                        border.width: 1
                        radius: 5

                        ListView {
                            id: domainViewField
                            anchors.fill: parent
                            anchors.margins: 1
                            clip: true
                            model: selectedDomains
                            ScrollBar.vertical: ScrollBar {
                                active: domainViewField.contentHeight > domainViewField.height
                            }

                            delegate: Rectangle {
                                width: domainViewField.width
                                height: 30
                                color: ListView.isCurrentItem ? "#DA3450" : "transparent"

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 5
                                    text: modelData
                                    //font.pointSize: 10
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        domainViewField.currentIndex = index
                                        editDomainParticipantButton.enabled = true
                                        leaveDomainButton.enabled = true
                                    }
                                    onDoubleClicked: {
                                        domainViewField.currentIndex = index
                                        openEditDomainParticipantDialog()
                                    }
                                }

                                Rectangle {
                                    width: parent.width
                                    height: 1
                                    color: "lightgray"
                                    anchors.bottom: parent.bottom
                                    visible: index < domainViewField.count - 1
                                }
                            }
                        }
                    }
                    Button {
                        id: editDomainParticipantButton
                        height: 35
                        width: parent.width * 0.40
                        anchors.top: joinDomainButton.bottom
                        anchors.right: parent.right
                        anchors.margins: 10
                        enabled: false
                        text: "Edit DomainParticipant(s)"

                        background: Rectangle {
                            radius: 5
                            color: parent.enabled ? "white" : "#F0F0F0"
                            border.color: "darkgray"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: parent.text
                            color: parent.enabled ? "black" : "darkgray"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: openEditDomainParticipantDialog()
                    }
                    Button {
                        id: leaveDomainButton
                        height: 35
                        width: parent.width * 0.40
                        anchors.top: editDomainParticipantButton.bottom
                        anchors.right: parent.right
                        anchors.margins: 10
                        anchors.topMargin: 28
                        enabled: false
                        text: "Leave Domain(s)"

                        background: Rectangle {
                            radius: 5
                            color: parent.enabled ? "white" : "#F0F0F0"
                            border.color: "darkgray"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: parent.text
                            color: parent.enabled ? "black" : "darkgray"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            selectedDomains.splice(domainViewField.currentIndex, 1)
                            domainViewField.model = selectedDomains
                            editDomainParticipantButton.enabled = false
                            leaveDomainButton.enabled = false
                        }
                    }
                }
                Rectangle {
                    id: filesField
                    Layout.fillWidth: true
                    height: 130
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        id: filesLabel
                        text: "Files specifying QoS profiles"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        anchors.bottomMargin: 10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }
                    TextField {
                        id: filesViewField
                        width: parent.width * 0.75
                        height: 100
                        anchors.top: filesLabel.bottom
                        anchors.left: parent.left
                        anchors.margins: 10

                        background: Rectangle {
                            border.color: "lightgray"
                            border.width: 1
                        }
                    }
                    Button {
                        id: addFilesButton
                        height: 35
                        width: parent.width * 0.20
                        anchors.top: filesLabel.bottom
                        anchors.right: parent.right
                        anchors.margins: 10
                        text: "Add Files(s)..."
                        background: Rectangle {
                            radius: 5
                        }
                        onClicked: fileDialog.open()
                    }
                    Button {
                        id: removeFilesButton
                        height: 35
                        width: parent.width * 0.20
                        anchors.top: addFilesButton.bottom
                        anchors.right: parent.right
                        anchors.margins: 10
                        anchors.topMargin: 28

                        // padding: -50
                        background: Rectangle {
                            radius: 5
                        }
                        contentItem: Text {
                            text: "Remove Files(s)"
                            color: "darkgray"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
                Item {
                    Layout.fillHeight: true // This will push the cleanup field to the top
                }
            }
        }
    }
}
