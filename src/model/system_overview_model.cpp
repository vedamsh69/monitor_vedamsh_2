#include <include/indedds-monitor/model/system_overview_model.h>
#include <QDebug>
#include <iostream>

SystemOverviewModel::SystemOverviewModel(QObject *parent)
    : QObject(parent)
    , m_highlightMode("Notifications")
    , m_selectedMeasurement("Samples Received Count")
    , m_selectedScale("No Scaling")
    , m_displayAllMode("TopicName")
    , m_displayHostMode("TopicName")
    , m_displayProcessMode("TopicName")
{
    // std::cout << "[SystemOverviewModel] Initializing..." << std::endl;
    
    // Setup match refresh timer (default 5 seconds)
    m_matchRefreshTimer = new QTimer(this);
    m_matchRefreshTimer->setInterval(5000);
    connect(m_matchRefreshTimer, &QTimer::timeout, this, &SystemOverviewModel::updateMatches);
    m_matchRefreshTimer->start();
    
    // std::cout << "[SystemOverviewModel] ✓ Initialized" << std::endl;
}

SystemOverviewModel::~SystemOverviewModel()
{
    // std::cout << "[SystemOverviewModel] Destroyed" << std::endl;
}

void SystemOverviewModel::setHighlightMode(const QString& mode)
{
    if (m_highlightMode != mode) {
        m_highlightMode = mode;
        // std::cout << "[SystemOverviewModel] Highlight mode: " << mode.toStdString() << std::endl;
        emit highlightModeChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::setSelectedMeasurement(const QString& measurement)
{
    if (m_selectedMeasurement != measurement) {
        m_selectedMeasurement = measurement;
        // std::cout << "[SystemOverviewModel] Measurement: " << measurement.toStdString() << std::endl;
        emit selectedMeasurementChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::setSelectedScale(const QString& scale)
{
    if (m_selectedScale != scale) {
        m_selectedScale = scale;
        // std::cout << "[SystemOverviewModel] Scale: " << scale.toStdString() << std::endl;
        emit selectedScaleChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::setDisplayAllMode(const QString& mode)
{
    if (m_displayAllMode != mode) {
        m_displayAllMode = mode;
        emit displayAllModeChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::setDisplayHostMode(const QString& mode)
{
    if (m_displayHostMode != mode) {
        m_displayHostMode = mode;
        emit displayHostModeChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::setDisplayProcessMode(const QString& mode)
{
    if (m_displayProcessMode != mode) {
        m_displayProcessMode = mode;
        emit displayProcessModeChanged();
        updateEntityTree();
    }
}

void SystemOverviewModel::addHost(const QString& hostName, const QString& hostId)
{
    // std::cout << "[SystemOverviewModel] Adding host: " << hostName.toStdString() << std::endl;
    
    QVariantMap host;
    host["id"] = hostId;
    host["type"] = "Host";
    host["name"] = hostName;
    host["status"] = "Normal";
    host["children"] = QVariantList();
    
    m_entities[hostId] = host;
    updateEntityTree();
}

void SystemOverviewModel::addDomainParticipant(const QString& hostId, const QString& dpName, int domainId, int processId)
{
    QString dpId = QString("%1_DP_%2").arg(hostId).arg(dpName);
    /* std::cout << "[SystemOverviewModel] Adding DomainParticipant: " << dpName.toStdString() 
              << " (Domain: " << domainId << ", PID: " << processId << ")" << std::endl;
    */
    
    QVariantMap dp;
    dp["id"] = dpId;
    dp["type"] = "DomainParticipant";
    dp["name"] = dpName;
    dp["domainId"] = domainId;
    dp["processId"] = processId;
    dp["status"] = "Normal";
    dp["parentId"] = hostId;
    dp["children"] = QVariantList();
    
    m_entities[dpId] = dp;
    updateEntityTree();
}

void SystemOverviewModel::addPublisher(const QString& dpId, const QString& pubId, const QString& pubName)
{
    // std::cout << "[SystemOverviewModel] Adding Publisher: " << pubName.toStdString() << std::endl;
    
    QVariantMap pub;
    pub["id"] = pubId;
    pub["type"] = "Publisher";
    pub["name"] = pubName;
    pub["status"] = "Normal";
    pub["parentId"] = dpId;
    pub["children"] = QVariantList();
    
    m_entities[pubId] = pub;
    updateEntityTree();
}

void SystemOverviewModel::addSubscriber(const QString& dpId, const QString& subId, const QString& subName)
{
    // std::cout << "[SystemOverviewModel] Adding Subscriber: " << subName.toStdString() << std::endl;
    
    QVariantMap sub;
    sub["id"] = subId;
    sub["type"] = "Subscriber";
    sub["name"] = subName;
    sub["status"] = "Normal";
    sub["parentId"] = dpId;
    sub["children"] = QVariantList();
    
    m_entities[subId] = sub;
    updateEntityTree();
}

void SystemOverviewModel::addDataWriter(const QString& pubId, const QString& dwId, const QString& topicName)
{
    // std::cout << "[SystemOverviewModel] Adding DataWriter for topic: " << topicName.toStdString() << std::endl;
    
    QVariantMap dw;
    dw["id"] = dwId;
    dw["type"] = "DataWriter";
    dw["name"] = "DW";
    dw["topicName"] = topicName;
    dw["status"] = "Normal";
    dw["parentId"] = pubId;
    
    m_entities[dwId] = dw;
    updateEntityTree();
}

void SystemOverviewModel::addDataReader(const QString& subId, const QString& drId, const QString& topicName)
{
    // std::cout << "[SystemOverviewModel] Adding DataReader for topic: " << topicName.toStdString() << std::endl;
    
    QVariantMap dr;
    dr["id"] = drId;
    dr["type"] = "DataReader";
    dr["name"] = "DR";
    dr["topicName"] = topicName;
    dr["status"] = "Normal";
    dr["parentId"] = subId;
    
    m_entities[drId] = dr;
    updateEntityTree();
}

void SystemOverviewModel::addTopic(const QString& dpId, const QString& topicId, const QString& topicName)
{
    // std::cout << "[SystemOverviewModel] Adding Topic: " << topicName.toStdString() << std::endl;
    
    QVariantMap topic;
    topic["id"] = topicId;
    topic["type"] = "Topic";
    topic["name"] = topicName;
    topic["status"] = "Normal";
    topic["parentId"] = dpId;
    
    m_entities[topicId] = topic;
    updateEntityTree();
}

void SystemOverviewModel::updateEntityStatus(const QString& entityId, const QString& status)
{
    if (m_entities.contains(entityId)) {
        m_entities[entityId]["status"] = status;
        updateEntityTree();
    }
}

void SystemOverviewModel::updateMatches()
{
    if (m_highlightMode == "Matches") {
        // std::cout << "[SystemOverviewModel] Updating matches..." << std::endl;
        recalculateMatches();
        updateEntityTree();
    }
}

void SystemOverviewModel::recalculateMatches()
{
    m_matches.clear();
    
    // Find all DataWriters and DataReaders
    QStringList dataWriters;
    QStringList dataReaders;
    
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        QString type = it.value()["type"].toString();
        if (type == "DataWriter") {
            dataWriters.append(it.key());
        } else if (type == "DataReader") {
            dataReaders.append(it.key());
        }
    }
    
    // Match DataWriters with DataReaders based on topic name
    for (const QString& dwId : dataWriters) {
        QString dwTopic = m_entities[dwId]["topicName"].toString();
        
        for (const QString& drId : dataReaders) {
            QString drTopic = m_entities[drId]["topicName"].toString();
            
            if (dwTopic == drTopic) {
                m_matches[dwId].append(drId);
                m_matches[drId].append(dwId);
                
                m_entities[dwId]["status"] = "Matched";
                m_entities[drId]["status"] = "Matched";
                
                /* std::cout << "[SystemOverviewModel] Matched: DW(" << dwTopic.toStdString() 
                         << ") <-> DR(" << drTopic.toStdString() << ")" << std::endl;
                */
            }
        }
        
        // Mark unmatched
        if (!m_matches.contains(dwId) || m_matches[dwId].isEmpty()) {
            m_entities[dwId]["status"] = "Unmatched";
        }
    }
    
    for (const QString& drId : dataReaders) {
        if (!m_matches.contains(drId) || m_matches[drId].isEmpty()) {
            m_entities[drId]["status"] = "Unmatched";
        }
    }
}

void SystemOverviewModel::updateEntityTree()
{
    // std::cout << "[SystemOverviewModel] Updating entity tree..." << std::endl;
    
    m_hostEntities.clear();
    
    // Build hierarchy
    QMap<QString, QVariantList> childrenMap;
    
    // Group entities by parent
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        QString entityId = it.key();
        QVariantMap entity = it.value();
        QString parentId = entity["parentId"].toString();
        
        if (!parentId.isEmpty()) {
            childrenMap[parentId].append(entity);
        }
    }
    
    // Build tree from hosts
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        QVariantMap entity = it.value();
        if (entity["type"].toString() == "Host") {
            QString hostId = entity["id"].toString();
            entity["children"] = childrenMap[hostId];
            
            // Add children recursively
            QVariantList children = childrenMap[hostId];
            for (int i = 0; i < children.size(); ++i) {
                QVariantMap child = children[i].toMap();
                QString childId = child["id"].toString();
                child["children"] = childrenMap[childId];
                children[i] = child;
            }
            entity["children"] = children;
            
            m_hostEntities.append(entity);
        }
    }
    
    // std::cout << "[SystemOverviewModel] ✓ Tree updated with " << m_hostEntities.size() << " hosts" << std::endl;
    emit hostEntitiesChanged();
}

void SystemOverviewModel::selectEntity(const QString& entityId)
{
    if (m_entities.contains(entityId)) {
        m_selectedEntity = m_entities[entityId];
        // std::cout << "[SystemOverviewModel] Selected: " << entityId.toStdString() << std::endl;
        emit selectedEntityChanged();
    }
}

void SystemOverviewModel::clearAll()
{
    // std::cout << "[SystemOverviewModel] Clearing all entities" << std::endl;
    m_entities.clear();
    m_matches.clear();
    m_hostEntities.clear();
    emit hostEntitiesChanged();
}

QString SystemOverviewModel::getEntityColor(const QString& status)
{
    // Notifications mode colors
    if (m_highlightMode == "Notifications") {
        if (status == "Normal") return "#90EE90"; // Light green
        if (status == "Warning") return "#FFD700"; // Gold
        if (status == "Error") return "#FF6B6B"; // Light red
        if (status == "Selected") return "#4169E1"; // Royal blue
    }
    
    // Matches mode colors
    if (m_highlightMode == "Matches") {
        if (status == "Matched") return "#90EE90"; // Green
        if (status == "PartiallyMatched") return "#FFD700"; // Yellow
        if (status == "Unmatched") return "#FF6B6B"; // Red
        if (status == "Selected") return "#4169E1"; // Blue
    }
    
    // Measurement mode colors (gradient based on value)
    if (m_highlightMode == "Measurement") {
        return "#87CEEB"; // Sky blue (default)
    }
    
    return "#D3D3D3"; // Light gray (default)
}

QString SystemOverviewModel::getEntityDisplayName(const QString& entityType, const QString& entityName, const QString& topicName)
{
    QString displayMode = m_displayAllMode;
    
    if (displayMode == "Hide") return "";
    if (displayMode == "NoName") return "";
    
    if (displayMode == "Terse") {
        if (entityType == "Host") return "H";
        if (entityType == "DomainParticipant") return "DP";
        if (entityType == "Publisher") return "Pub";
        if (entityType == "Subscriber") return "Sub";
        if (entityType == "DataWriter") return "DW";
        if (entityType == "DataReader") return "DR";
        if (entityType == "Topic") return "T";
    }
    
    if (displayMode == "RoleName") {
        return entityName;
    }
    
    if (displayMode == "TopicName") {
        if (!topicName.isEmpty()) {
            return topicName;
        }
        return entityName;
    }
    
    return entityName;
}

QVariantMap SystemOverviewModel::createEntityMap(const QString& id, const QString& type, 
                                                  const QString& name, const QString& status, 
                                                  const QVariantList& children)
{
    QVariantMap entity;
    entity["id"] = id;
    entity["type"] = type;
    entity["name"] = name;
    entity["status"] = status;
    entity["children"] = children;
    return entity;
}
