#ifndef PUBLICATIONMANAGER_H
#define PUBLICATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QTimer>
#include "indedds-monitor/topic_idl_struct.h"

class Controller;  // Forward declaration

class PublicationManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString topicName READ topicName WRITE setTopicName NOTIFY topicNameChanged)
    Q_PROPERTY(QString topicType READ topicType NOTIFY topicTypeChanged)
    Q_PROPERTY(int domainId READ domainId WRITE setDomainId NOTIFY domainIdChanged)
    Q_PROPERTY(QString sampleCode READ sampleCode NOTIFY sampleCodeChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int sampleQueueSize READ sampleQueueSize NOTIFY sampleQueueSizeChanged)
    Q_PROPERTY(bool isPublishing READ isPublishing NOTIFY isPublishingChanged)
    Q_PROPERTY(QObject* controller READ controller WRITE setController NOTIFY controllerChanged)

public:
    explicit PublicationManager(QObject *parent = nullptr);
    ~PublicationManager();

    QString topicName() const { return m_topicName; }
    QString topicType() const { return m_topicType; }
    int domainId() const { return m_domainId; }
    QString sampleCode() const { return m_sampleCode; }
    QString statusMessage() const { return m_statusMessage; }
    int sampleQueueSize() const { return m_sampleQueue.size(); }
    bool isPublishing() const { return m_isPublishing; }
    QObject* controller() const { return m_controller; }
    Q_INVOKABLE void setController(QObject* ctrl);
    
    void setTopicName(const QString& name);
    void setDomainId(int id);
    
    Q_INVOKABLE void setTopicIDLModel(TopicIDLStruct* model);
    Q_INVOKABLE void generateSampleCode();
    Q_INVOKABLE QVariantMap executePythonCode(const QString& code);
    Q_INVOKABLE int addSampleToQueue(const QVariantMap& sampleData);
    Q_INVOKABLE void clearSampleQueue();
    Q_INVOKABLE bool publishSample(int index);
    Q_INVOKABLE void publishAllSamples();
    Q_INVOKABLE void startPeriodicPublishing(int periodMs);
    Q_INVOKABLE void stopPeriodicPublishing();
    Q_INVOKABLE QVariantList getAllSamples();

signals:
    void topicNameChanged();
    void topicTypeChanged();
    void domainIdChanged();
    void sampleCodeChanged();
    void statusMessageChanged();
    void sampleQueueSizeChanged();
    void isPublishingChanged();
    void samplePublished(int index, bool success);
    void errorOccurred(const QString& error);
    void logMessage(const QString& message);
    void publishSampleRequest(QString topicName, int domainId, QVariantMap sampleData);
    void controllerChanged();

private slots:
    void onTopicIDLTextDataChanged();
    void onPeriodicPublishTimeout();

private:
    QString extractStructName(const QString& idlText);
    QVariantMap extractFields(const QString& idlText);
    QString generatePythonCode(const QVariantMap& fields);
    QVariant getDefaultValue(const QString& type);
    void setStatusMessage(const QString& msg);
    
    QString m_topicName;
    QString m_topicType;
    int m_domainId;
    QString m_sampleCode;
    QString m_statusMessage;
    bool m_isPublishing;
    
    TopicIDLStruct* m_topicIDLModel;
    QVariantMap m_topicFields;
    QList<QVariantMap> m_sampleQueue;
    QTimer* m_publishTimer;
    int m_publishIndex;
    QObject* m_controller;
};

#endif // PUBLICATIONMANAGER_H
