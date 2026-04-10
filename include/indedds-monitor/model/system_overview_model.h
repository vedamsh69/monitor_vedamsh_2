    #ifndef SYSTEMOVERVIEWMODEL_H
#define SYSTEMOVERVIEWMODEL_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QMap>
#include <QTimer>
#include <memory>

class SystemOverviewModel : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString highlightMode READ highlightMode WRITE setHighlightMode NOTIFY highlightModeChanged)
    Q_PROPERTY(QString selectedMeasurement READ selectedMeasurement WRITE setSelectedMeasurement NOTIFY selectedMeasurementChanged)
    Q_PROPERTY(QString selectedScale READ selectedScale WRITE setSelectedScale NOTIFY selectedScaleChanged)
    Q_PROPERTY(QVariantList hostEntities READ hostEntities NOTIFY hostEntitiesChanged)
    Q_PROPERTY(QVariantMap selectedEntity READ selectedEntity NOTIFY selectedEntityChanged)
    Q_PROPERTY(QString displayAllMode READ displayAllMode WRITE setDisplayAllMode NOTIFY displayAllModeChanged)
    Q_PROPERTY(QString displayHostMode READ displayHostMode WRITE setDisplayHostMode NOTIFY displayHostModeChanged)
    Q_PROPERTY(QString displayProcessMode READ displayProcessMode WRITE setDisplayProcessMode NOTIFY displayProcessModeChanged)

public:
    explicit SystemOverviewModel(QObject *parent = nullptr);
    ~SystemOverviewModel();

    // Highlight modes
    enum HighlightMode {
        Notifications,
        Matches,
        Measurement
    };
    Q_ENUM(HighlightMode)

    // Entity status
    enum EntityStatus {
        Normal,
        Warning,
        Error,
        Matched,
        PartiallyMatched,
        Unmatched,
        Selected
    };
    Q_ENUM(EntityStatus)

    // Display modes
    enum DisplayMode {
        Hide,
        NoName,
        Terse,
        RoleName,
        TopicName
    };
    Q_ENUM(DisplayMode)

    // Getters
    QString highlightMode() const { return m_highlightMode; }
    QString selectedMeasurement() const { return m_selectedMeasurement; }
    QString selectedScale() const { return m_selectedScale; }
    QVariantList hostEntities() const { return m_hostEntities; }
    QVariantMap selectedEntity() const { return m_selectedEntity; }
    QString displayAllMode() const { return m_displayAllMode; }
    QString displayHostMode() const { return m_displayHostMode; }
    QString displayProcessMode() const { return m_displayProcessMode; }

    // Invokable methods
    Q_INVOKABLE void setHighlightMode(const QString& mode);
    Q_INVOKABLE void setSelectedMeasurement(const QString& measurement);
    Q_INVOKABLE void setSelectedScale(const QString& scale);
    Q_INVOKABLE void setDisplayAllMode(const QString& mode);
    Q_INVOKABLE void setDisplayHostMode(const QString& mode);
    Q_INVOKABLE void setDisplayProcessMode(const QString& mode);
    Q_INVOKABLE void selectEntity(const QString& entityId);
    Q_INVOKABLE void addHost(const QString& hostName, const QString& hostId);
    Q_INVOKABLE void addDomainParticipant(const QString& hostId, const QString& dpName, int domainId, int processId);
    Q_INVOKABLE void addPublisher(const QString& dpId, const QString& pubId, const QString& pubName);
    Q_INVOKABLE void addSubscriber(const QString& dpId, const QString& subId, const QString& subName);
    Q_INVOKABLE void addDataWriter(const QString& pubId, const QString& dwId, const QString& topicName);
    Q_INVOKABLE void addDataReader(const QString& subId, const QString& drId, const QString& topicName);
    Q_INVOKABLE void addTopic(const QString& dpId, const QString& topicId, const QString& topicName);
    Q_INVOKABLE void updateEntityStatus(const QString& entityId, const QString& status);
    Q_INVOKABLE void updateMatches();
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE QString getEntityColor(const QString& status);
    Q_INVOKABLE QString getEntityDisplayName(const QString& entityType, const QString& entityName, const QString& topicName);

signals:
    void highlightModeChanged();
    void selectedMeasurementChanged();
    void selectedScaleChanged();
    void hostEntitiesChanged();
    void selectedEntityChanged();
    void displayAllModeChanged();
    void displayHostModeChanged();
    void displayProcessModeChanged();

private:
    QString m_highlightMode;
    QString m_selectedMeasurement;
    QString m_selectedScale;
    QVariantList m_hostEntities;
    QVariantMap m_selectedEntity;
    QString m_displayAllMode;
    QString m_displayHostMode;
    QString m_displayProcessMode;
    
    QMap<QString, QVariantMap> m_entities;
    QMap<QString, QStringList> m_matches; // entityId -> list of matched entities
    
    QTimer* m_matchRefreshTimer;
    
    void recalculateMatches();
    void updateEntityTree();
    QVariantMap createEntityMap(const QString& id, const QString& type, const QString& name, 
                                 const QString& status, const QVariantList& children = QVariantList());
    QString determineStatus(const QString& entityId);
    bool areEntitiesMatched(const QString& entityId1, const QString& entityId2);
};

#endif // SYSTEMOVERVIEWMODEL_H
