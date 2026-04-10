import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0

Rectangle {
    id: qosandcontentfilter
    anchors.fill: parent
    anchors.margins: 10
    
    ColumnLayout {
        id: columnforqosandcontentfilter
        anchors.fill: parent
        anchors.topMargin: 10 // Add some top margin if needed
        spacing: 10 // Add spacing between items in the column
        
        RowLayout {
            id: rowforqosandcontentfilter
            Layout.fillWidth: true
            
            Label {
                text: "Select Qos Profile"
            }
            
            Item {
                Layout.fillWidth: true
            }
            
            Label {
                text: "Manage Qos Profile Files"
                color: "#a3386c"
            }
        }
        
        ComboBox
        {
            model : ["AdminConsole::Default(default)","AdminConsole::RealTimeWAN"]
            Layout.preferredHeight : 28
            Layout.preferredWidth: 580
        }
        
        CheckBox {
            id: enableDataWriterFilter
            text: "Show Built-in QoS profiles"
            Layout.fillWidth: true
            transformOrigin: Item.Left  // Ensure scaling happens from the left side
        }
        
        Label
        {
            id : labelforrectangle
            text : "QoS Values to Override"
        }
        
        
        
        Rectangle {
            id: qosvaluestooverride
            border.width: 0.5
            border.color: "#D4D4D4"
            width: 640
            height: 320
            
            ColumnLayout {
                id: qosandcontentfiltercolumn
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15
                
                RowLayout {
                    id: qoscombobox
                    Layout.fillWidth: true
                    spacing: 60
                    
                    Text {
                        text: qsTr("Reliability")
                        Layout.preferredWidth: 70
                    }
                    
                    ComboBox {
                        id: reliabilitycombobox
                        model: ["", "Best Effort", "Reliable"]  // Add an empty option
                        Layout.preferredHeight: 28
                        Layout.preferredWidth: 150
                        currentIndex: 0  // Set to 0 to select the empty option initially
                        
                        onActivated: {
                            if (currentIndex > 0) {  // Only set if a non-empty option is selected
                                topicIDLModel.reliability = currentText
                            } else {
                                topicIDLModel.reliability = ""  // Clear the value if empty option is selected
                            }
                        }
                        
                        background: Rectangle {
                            color: "white"
                            border.color: reliabilitycombobox.pressed ? "#AAAAAA" : "#CCCCCC"
                            border.width: 1
                            radius: 6
                        }
                    }
                    
                    Text {
                        text: qsTr("Durability")
                        Layout.preferredWidth: 70
                    }
                    
                    ComboBox {
                        id: durabilitycombobox
                        model: ["", "Volatile", "Transient", "Transient Local", "Persistent"]  // Add an empty option
                        Layout.preferredHeight: 28
                        Layout.preferredWidth: 150
                        currentIndex: 0  // Set to 0 to select the empty option initially
                        
                        onActivated: {
                            if (currentIndex > 0) {  // Only set if a non-empty option is selected
                                topicIDLModel.durability = currentText
                            } else {
                                topicIDLModel.durability = ""  // Clear the value if empty option is selected
                            }
                        }
                        
                        background: Rectangle {
                            color: "white"
                            border.color: durabilitycombobox.pressed ? "#AAAAAA" : "#CCCCCC"
                            border.width: 1
                            radius: 6
                        }
                    }
                }
                
                RowLayout {
                    id: qoscombobox2
                    Layout.fillWidth: true
                    spacing: 60
                    
                    Text {
                        text: qsTr("Ownership")
                        Layout.preferredWidth: 70
                    }
                    
                    ComboBox {
                        id: ownershipcombobox
                        model: ["", "Shared", "Exclusive"]  // Add an empty option
                        Layout.preferredHeight: 28
                        Layout.preferredWidth: 150
                        currentIndex: 0  // Set to 0 to select the empty option initially
                        
                        onActivated: {
                            if (currentIndex > 0) {  // Only set if a non-empty option is selected
                                topicIDLModel.ownership = currentText
                            } else {
                                topicIDLModel.ownership = ""  // Clear the value if empty option is selected
                            }
                        }
                        
                        background: Rectangle {
                            color: "white"
                            border.color: ownershipcombobox.pressed ? "#AAAAAA" : "#CCCCCC"
                            border.width: 1
                            radius: 6
                        }
                    }
                    
                    // Text {
                    //     text: qsTr("Time Based filter(ms)")
                    //     Layout.preferredWidth: 120
                    // }
                    
                    // TextField {
                    //     id: timebasedfiltertextfield
                    //     Layout.preferredHeight: 28
                    //     Layout.preferredWidth: 100
                    //     placeholderText: "0.0"
                    
                    //     background: Rectangle {
                    //         color: "white"
                    //         border.color: timebasedfiltertextfield.activeFocus ? "#AAAAAA" : "#CCCCCC"
                    //         border.width: 1
                    //         radius: 6
                    //     }
                    // }
                }
                
                RowLayout
                {
                    spacing : 15
                    Label
                    {
                        text : "Instance State Recovery Kind"
                    }
                    
                    ComboBox
                    {
                        id : recoverystatecombobox
                        model : ["No instance state recovery","Recover instance state"]
                        Layout.preferredHeight : 28
                        Layout.preferredWidth : 410
                        
                        background: Rectangle{
                            color : "white"
                            border.color : recoverystatecombobox.activeFocus ? "#AAAAAA" : "#CCCCCC"
                            border.width : 1
                            radius : 6
                        }
                    }
                }
                
                RowLayout {
                    id: subscriberpartitionrow
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Label {
                        text: "Subscriber Partitions"
                        Layout.preferredWidth: 180  // Match the width of "Instance State Recovery Kind" label
                    }
                    
                    Item {
                        // This invisible item acts as a spacer
                        Layout.preferredWidth: 10  // Adjust this value to match the space before the recovery state ComboBox
                    }
                    
                    TextField {
                        id: subscriberpartitiontextfield
                        Layout.preferredHeight: 28
                        Layout.fillWidth: true
                        Layout.minimumWidth: 200
                        readOnly: true
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
                            
                            onClicked:
                            {
                                editpartitionforsubscriber.open()
                            }
                        }
                        
                        
                        
                    }
                }
                
                
                
                RowLayout {
                    id: dppartitionrow
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Label {
                        text: "DomainParticipant Partitions"
                        Layout.preferredWidth: 180  // Match the width of "Instance State Recovery Kind" label
                    }
                    
                    Item {
                        // This invisible item acts as a spacer
                        Layout.preferredWidth: 10  // Adjust this value to match the space before the recovery state ComboBox
                    }
                    
                    TextField {
                        id: dppartitiontextfield
                        Layout.preferredHeight: 28
                        Layout.fillWidth: true
                        Layout.minimumWidth: 200
                        readOnly: true
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
                            
                            onClicked:
                            {
                                editpartitionfordomainparticipant.open()
                            }
                        }
                        
                        
                        
                    }
                }
                
                
                Label
                {
                    id : contentfilterexpression
                    text : "Content Filter Expression"
                }
                
                TextField
                {
                    id : contentfilterexpressiontextfiled
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                }
                
                
                // Add this spacer to push content to the top
                Item {
                    Layout.fillHeight: true
                }
            }
        }
        
        
        
        
        Item {
            Layout.fillHeight: true
        }
        
    }
    EditPartitiondialogforsubscriber
    {
        id : editpartitionforsubscriber
        parent : qosandcontentfilter
    }
    
    EditPartitiondialogfordomianpartition
    {
        id : editpartitionfordomainparticipant
        parent : qosandcontentfilter
    }
    
}







