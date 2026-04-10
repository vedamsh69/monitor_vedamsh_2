// DataStore.qml
pragma Singleton
import QtQuick 2.15
QtObject {
    property string domainId: ""
    property var tableData: []

    function updateData(model) {
        console.log("Updating DataStore with new model")
        
        if (model && model.domain) {
            domainId = model.domain
        }

        var newTableData = []

        if (model && model.hosts && model.topics) {
            for (var i = 0; i < model.hosts.length; i++) {
                var host = model.hosts[i]
                var hostName = host.alias
                
                for (var j = 0; j < host.users.length; j++) {
                    var user = host.users[j]
                    
                    for (var k = 0; k < user.processes.length; k++) {
                        var process = user.processes[k]
                        var processId = process.pid
                        
                        for (var l = 0; l < process.participants.length; l++) {
                            var participant = process.participants[l]
                            var participantName = participant.alias
                            
                            for (var m = 0; m < participant.endpoints.length; m++) {
                                var endpoint = participant.endpoints[m]
                                var topic = model.topics.find(t => t.id === endpoint.topic)
                                
                                if (topic) {
                                    newTableData.push({
                                        topicName: topic.alias,
                                        hostName: hostName,
                                        processId: processId,
                                        participantName: participantName,
                                        publisher: endpoint.kind === "DataWriter" ? endpoint.alias : "",
                                        subscriber: endpoint.kind === "DataReader" ? endpoint.alias : ""
                                    })
                                }
                            }
                        }
                    }
                }
            }
        }

        tableData = newTableData

        console.log("DataStore updated:", JSON.stringify({
            domainId: domainId,
            tableData: tableData
        }, null, 2))
    }
}