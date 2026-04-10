import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0

Dialog
{
    id : dialogwindow
    title : "Load Data Types"
    height : 490
    width : 380


    ColumnLayout
    {
        id : typesxmlfiles
        anchors.top : parent.Top
        width : parent.width

        Column
        {

            Label
            {
                text : "Types XML files"
            }


            Row {

                TextField {
                    id: fileNameTextField
                    height : 80
                    width: 250
                    placeholderText: "No file selected"
                    readOnly: true
                }

                spacing : defaultSpacing


                Column {
                    anchors.top: dialogwindow.top
                    anchors.right: dialogwindow.right
                    anchors.margins: 10 // Optional: adjust to suit your needs
                    spacing: defaultSpacing


                    // Plus button
                    Item {
                        id : item1
                        width: 20
                        height: 20

                        Image {
                            source: "qrc:/resources/images/icons/plus.png" // Ensure this path is correct
                            anchors.fill: item1
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                fileDialog.folder = "file:///home/cdac"
                                fileDialog.open()
                            }
                        }
                    }

                    // Minus button
                    Item {
                        id : item2
                        width: 20
                        height: 20

                        Image {
                            source: "qrc:/resources/images/icons/minus.png" // Ensure this path is correct
                            anchors.fill: item2
                        }

                        MouseArea {
                            anchors.fill: item2
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                console.log("Minus button clicked")
                                // Add your action for the minus button here
                            }
                        }
                    }


                    FileDialog {
                        id: fileDialog
                        title: "Select a directory"
                        selectExisting: true
                        selectMultiple: false
                        selectFolder: true

                        onAccepted: {

                            if (fileUrls.length > 0) {
                                fileNameTextField.text = fileUrls[0].toString()
                            }
                        }
                    }

                }


            }

        }

        Column
        {



            Label
            {
                text : "Loaded Type Names"
            }

            TextField
            {
                id : loadedtypenames
                height : 80
                width : 280
                placeholderText: "Define Type Name"
                readOnly: true
            }

        }

        Label
        {
            text : "Include Directories"
        }

        Row
        {
            id : includedirectoriesrow

            spacing: defaultSpacing
            TextField
            {
                id : includedirectoriestextfield
                placeholderText: "Select the directories to include"
                width : 250
                readOnly: true

            }

            Item {
                id : item3
                width: 20
                height: 20

                Image {
                    source: "qrc:/resources/images/icons/plus.png" // Ensure this path is correct
                    anchors.fill: item3
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        fileDialog2.folder = "file:///home/cdac"
                        fileDialog2.open()
                    }
                }
            }

            FileDialog {
                id: fileDialog2
                title: "Select a directory"
                selectExisting: true
                selectMultiple: false
                selectFolder: true

                onAccepted: {

                    if (fileUrls.length > 0) {
                        includedirectoriestextfield.text = fileUrls[0].toString()
                    }
                }
            }

        }
        spacing : defaultSpacing

        Column
        {

            Row {
                id : unboundedrow
                spacing : 36 //this spacng is for the textfiled and the text

                Label
                {
                    text : "Unbounded String Max Length"
                    anchors.bottom: unboundedrow.bottom
                }

                Row
                {

                    TextField
                    {
                        id : unboundedtextfield
                        text : "255"
                        width : 70
                        height : 30

                    }

                    Button {
                        id: incrementbutton
                        text: "+"
                        height: 30
                        width: 20
                        onClicked: {
                            // Increment the value in the TextField by one
                            let currentValue10 = parseInt(unboundedtextfield.text);
                            if (!isNaN(currentValue10)) {
                                unboundedtextfield.text = (currentValue10 + 1).toString();
                            }
                        }
                    }

                    Button {
                        id: incrementbuttonnegative
                        text: "-"
                        height: 30
                        width: 20
                        onClicked: {
                            // Increment the value in the TextField by one
                            let currentValue20 = parseInt(unboundedtextfield.text);
                            if (!isNaN(currentValue20)) {
                                unboundedtextfield.text = (currentValue20 - 1).toString();
                            }
                        }
                    }

                }

 }
        spacing : 20
            Row
            {
            spacing : 10


            Label
            {
                id : unboundedsequencelabel
                text : "Unbounded Sequence Max Length"

            }

            Row
            {

                TextField
                {
                    id : unboundedtextfield2
                    text : "100"
                    width : 70
                    height : 30

                }

                Button {
                    id: incrementbutton2
                    text: "+"
                    height: 30
                    width: 20
                    onClicked: {
                        // Increment the value in the TextField by one
                        let currentValue30 = parseInt(unboundedtextfield2.text);
                        if (!isNaN(currentValue30)) {
                            unboundedtextfield2.text = (currentValue30 + 1).toString();
                        }
                    }
                }

                Button {
                    id: incrementbuttonnegative2
                    text: "-"
                    height: 30
                    width: 20
                    onClicked: {
                        // Increment the value in the TextField by one
                        let currentValue40 = parseInt(unboundedtextfield2.text);
                        if (!isNaN(currentValue40)) {
                            unboundedtextfield2.text = (currentValue40 - 1).toString();
                        }
                    }


                }

            }

}
        }













    }

    standardButtons: Dialog.Ok | Dialog.Cancel
}










































