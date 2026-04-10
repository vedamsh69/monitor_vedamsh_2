// TopicsTable.qml
import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.15
import Theme 1.0

Item {
    id: topicsTable
    width: 1300
    height: 400

    TableView {
        id: tableView
        anchors.fill: parent
        model: DataStore.tableData

        TableViewColumn {
            role: "topicName"
            title: "Topic Name"
            width: 200
        }
        TableViewColumn {
            role: "hostName"
            title: "Host Name"
            width: 400
        }
        TableViewColumn {
            role: "processId"
            title: "Process ID"
            width: 100
        }
        TableViewColumn {
            role: "participantName"
            title: "Participant Name"
            width: 150
        }
        TableViewColumn {
            role: "publisher"
            title: "Publisher"
            width: 150
        }
        TableViewColumn {
            role: "subscriber"
            title: "Subscriber"
            width: 150
        }
        TableViewColumn {
            title: "Domain ID"
            width: 100
            delegate: Text {
                text: DataStore.domainId
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Connections {
        target: DataStore
        function onTableDataChanged() {
            tableView.model = null
            tableView.model = DataStore.tableData
        }
    }
}