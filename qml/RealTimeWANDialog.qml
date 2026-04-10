// RealTimeWANDialog.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

Dialog {
    id: realTimeWANDialog
    title: "Warning: RTI Real-Time WAN Transport required"
    width: parent.width * 1.35
    height: parent.height * 0.8

    Rectangle {
        width: parent.width
        height: parent.height
        color: "white"
        anchors.fill: parent

        TextArea {
            id: domainText
            anchors.topMargin: 10 // Increase this value to increase the gap
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            ////font.pointSize: 10
            wrapMode: TextArea.Wrap
            readOnly: true
            background: null
            selectByMouse: false
            text: "The selected profile QoS (AdminConsole::RealTimeWAN) requires the installation of RTI Real-Time WAN
Transport to work correctly.

If it i not installed, please install the RTI Real-Time WAN Transport, please check the
RTI Real TIme WAN Transport installation Guide.

In additon to using this profile, you will likely want to provide the initial peer(s) for the Real-Time WAN
Transport. This configuration can be done in the Preferences page.

for point-to-point connections, the format of the paper is:
udpv4_wan://<peer's public IP address>:<CDS's public IP port>

Important: The auto-join feature will not work when using the Real-TIme WAN Transport,
because multicast is not available in WAN environments. You will have to join the WAN domain(s) explicitly.

If you need more information about RTI Real-Time WAN Transport, please check the online documentation.
"
            CheckBox {
                id: checkBox
                text: "Do not show this message again"
                anchors.top: domainText.bottom
                scale: 1
                anchors.margins: 10

                indicator: Rectangle {
                    implicitWidth: 20 // Adjust the width as needed
                    implicitHeight: 20
                    border.color: "darkgray"
                    radius: 4
                    x: checkBox.leftPadding
                    y: parent.height / 2 - height / 2

                    Rectangle {
                        width: 14
                        height: 14
                        anchors.centerIn: parent
                        radius: 2 // Rounded corners for the inner rectangle
                        color: checkBox.down ? "#eb9b46" : "#eb9b46"
                        visible: checkBox.checked
                    }
                }
                // Adjust text position
                contentItem: Text {
                    text: checkBox.text
                    font: checkBox.font
                    opacity: enabled ? 1.0 : 0.3
                    color: "#36332b"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: checkBox.indicator.width + checkBox.spacing
                }
            }
        }
    }
}
