import QtQuick 2.15
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.15

Item
{
    id : datafiltertable
    height : 230
    width : 630

    TableView {
        id:tableView
        anchors.fill: parent
        //sortIndicatorVisible: true


        clip: true
        TableViewColumn {
            id:dataRow
            role: "Id"
            title: "Id"
            width: 50
            movable: false
            visible: false //hide column
            //visible: true
        }

        TableViewColumn {
            role: "publication handle"
            title: "Publication Handle"
            width: 120
            movable: false
        }
        TableViewColumn {
            role: "publication name"
            title: "Publication Name"
            width: 105
            movable: false
        }
        TableViewColumn {
            role: "participant name"
            title: "Participant Name"
            width: 100
            movable: false
        }
        TableViewColumn {
            role: "process"
            title: "Process"
            width: 100
            movable: false
        }
        TableViewColumn {
            role: "host name"
            title: "Host Name"
            width: 100
            movable: false
        }
        TableViewColumn {
            role: "ip addresses"
            title: "Ip Addresses"
            width: 100
            movable: false
        }
    }

}
