#ifndef TOPICIDLSTRUCT_H
#define TOPICIDLSTRUCT_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QFile>
#include <QTextStream>
#include <iostream>

class TopicIDLStruct : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString textData READ textData WRITE setTextData NOTIFY textDataChanged)
    Q_PROPERTY(QString topicname READ topicName WRITE setTopicName NOTIFY topicnamechanged)
    Q_PROPERTY(QString sampleInfo READ sampleInfo WRITE setSampleInfo NOTIFY sampleInfoChanged)
    Q_PROPERTY(QString dynamicData READ dynamicData WRITE setDynamicData NOTIFY dynamicDataChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY sampleCountChanged)
    Q_PROPERTY(QString reliability READ reliability WRITE setReliability NOTIFY reliabilityChanged)
    Q_PROPERTY(QString durability READ durability WRITE setDurability NOTIFY durabilityChanged)
    Q_PROPERTY(QString ownership READ ownership WRITE setOwnership NOTIFY ownershipChanged)
    Q_PROPERTY(QString selectedFields READ selectedFields NOTIFY selectedFieldsChanged)
    Q_PROPERTY(int domainId READ domainId WRITE setDomainId NOTIFY domainIdChanged)
    Q_PROPERTY(QVariantList treeData READ treeData NOTIFY treeDataChanged)

public:
    explicit TopicIDLStruct(QObject *parent = nullptr);

    QString topicName() const { return m_topicName; }
    QString textData() const { return m_textData; }
    QString sampleInfo() const { return m_sampleInfo; }
    QString dynamicData() const { return m_dynamicData; }
    int sampleCount() const { return m_samples.size(); }
    QString reliability() const { return m_reliability; }
    QString durability() const { return m_durability; }
    QString ownership() const { return m_ownership; }
    QString selectedFields() const { return m_selectedFields; }
    int domainId() const { return m_domainId; }
    QVariantList treeData() const { return m_treeData; }

    Q_INVOKABLE void setReliability(const QString &value) {
        if (m_reliability != value) {
            m_reliability = value;
            emit reliabilityChanged();
            std::cout << "Reliability set to in model is : " << m_reliability.toStdString() << std::endl;
        }
    }

    Q_INVOKABLE void setDurability(const QString &value) {
        if (m_durability != value) {
            m_durability = value;
            emit durabilityChanged();
            std::cout << "Durability set to in model is : " << m_durability.toStdString() << std::endl;
        }
    }

    Q_INVOKABLE void setOwnership(const QString &value) {
        if (m_ownership != value) {
            m_ownership = value;
            emit ownershipChanged();
            std::cout << "Ownership set to in model is : " << m_ownership.toStdString() << std::endl;
        }
    }

    Q_INVOKABLE void setDomainId(int id) {
        if (m_domainId != id) {
            m_domainId = id;
            emit domainIdChanged();
            std::cout << "[TopicIDLStruct] Domain ID set to: " << m_domainId << std::endl;
        }
    }

    Q_INVOKABLE void setTextData(const QString &text);

    Q_INVOKABLE void setTopicName(const QString &text) {
        if (m_topicName != text) {
            m_topicName = text;
            emit topicnamechanged();
        }
    }

    Q_INVOKABLE void setSampleInfo(const QString &info) {
        if (m_sampleInfo != info) {
            m_sampleInfo = info;
            emit sampleInfoChanged();
        }
    }

    Q_INVOKABLE void setDynamicData(const QString &data) {
        if (m_dynamicData != data) {
            m_dynamicData = data;
            emit dynamicDataChanged();
        }
    }

    Q_INVOKABLE QString getSampleTimestamp(int sampleIndex) const {
        QString dynamicData = m_samples.value(sampleIndex).value("dynamicData").toString();
        QStringList lines = dynamicData.split("\n");
        for (const QString& line : lines) {
            if (line.startsWith("Timestamp:")) {
                return line.mid(QString("Timestamp: ").length()).trimmed();
            }
        }
        return QString("No timestamp found");
    }

    Q_INVOKABLE void addSample(int sampleIndex, const QMap<QString, QVariant>& sampleData) {
        m_samples.insert(sampleIndex, sampleData);
        m_selectedFields = sampleData["selectedFields"].toString();
        emit sampleCountChanged();
        emit sampleAdded(sampleIndex, sampleData);
        emit selectedFieldsChanged();
    }

    Q_INVOKABLE QVariant getSampleData(int sampleIndex, const QString& key) const {
        return m_samples.value(sampleIndex).value(key);
    }

    Q_INVOKABLE QVariantMap getSample(int sampleIndex) const {
        return m_samples.value(sampleIndex);
    }
    
    Q_INVOKABLE void parseIDLToTree();
    
    // ========== NEW FILE EXPORT METHODS ==========
    Q_INVOKABLE bool saveTextToFile(const QString& filePath, const QString& content);
    Q_INVOKABLE QString generateCSVFromTree();

signals:
    void textDataChanged();
    void topicnamechanged();
    void sampleInfoChanged();
    void dynamicDataChanged();
    void sampleCountChanged();
    void reliabilityChanged();
    void durabilityChanged();
    void ownershipChanged();
    void sampleAdded(int sampleIndex, QVariantMap sampleData);
    void selectedFieldsChanged();
    void domainIdChanged();
    void treeDataChanged();

private:
    QString m_topicName;
    QString m_textData;
    QString m_sampleInfo;
    QString m_dynamicData;
    QMap<int, QVariantMap> m_samples;
    QString m_reliability;
    QString m_durability;
    QString m_ownership;
    QString m_selectedFields;
    int m_domainId = 0;
    QVariantList m_treeData;
    
    void parseStructToTree(const QString& idlText);
    QVariantMap createTreeNode(
        const QString& name, 
        const QString& type,
        const QString& extensibility, 
        const QString& optional,
        int minSize, 
        int maxSize, 
        const QString& typeCode,
        const QString& typeObject, 
        const QString& unionLabels,
        int depth);
    
    QString generateCSVRow(const QVariantMap& node, int indent);
};

#endif // TOPICIDLSTRUCT_H
