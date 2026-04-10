#include "indedds-monitor/publisher/PublicationManager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

PublicationManager::PublicationManager(QObject *parent)
    : QObject(parent)
    , m_domainId(0)
    , m_isPublishing(false)
    , m_topicIDLModel(nullptr)
    , m_publishTimer(new QTimer(this))
    , m_publishIndex(0)
    , m_controller(nullptr)  
{
    connect(m_publishTimer, &QTimer::timeout, this, &PublicationManager::onPeriodicPublishTimeout);
    setStatusMessage("Waiting for topic structure...");
    qDebug() << "[PublicationManager] Constructed";
}

PublicationManager::~PublicationManager()
{
    stopPeriodicPublishing();
    qDebug() << "[PublicationManager] Destroyed";
}

void PublicationManager::setTopicName(const QString& name)
{
    if (m_topicName != name) {
        m_topicName = name;
        emit topicNameChanged();
        qDebug() << "[PublicationManager] Topic name set to:" << name;
        
        // Trigger code generation if model is already available
        if (m_topicIDLModel && !m_topicIDLModel->textData().isEmpty()) {
            generateSampleCode();
        }
    }
}

void PublicationManager::setDomainId(int id)
{
    if (m_domainId != id) {
        m_domainId = id;
        emit domainIdChanged();
    }
}

void PublicationManager::setController(QObject* ctrl)
{
    qDebug() << "[PublicationManager] setController called with" << ctrl;
    
    if (m_controller != ctrl)
    {
        m_controller = ctrl;
        emit controllerChanged();
        qDebug() << "[PublicationManager] Controller set successfully";
    }
}

void PublicationManager::setTopicIDLModel(TopicIDLStruct* model)
{
    qDebug() << "[PublicationManager] setTopicIDLModel called, model =" << model;
    
    if (m_topicIDLModel == model) {
        return;
    }
    
    // Disconnect old model
    if (m_topicIDLModel) {
        disconnect(m_topicIDLModel, &TopicIDLStruct::textDataChanged,
                  this, &PublicationManager::onTopicIDLTextDataChanged);
    }
    
    m_topicIDLModel = model;
    
    // Connect new model
    if (m_topicIDLModel) {
        connect(m_topicIDLModel, &TopicIDLStruct::textDataChanged,
               this, &PublicationManager::onTopicIDLTextDataChanged);
        
        qDebug() << "[PublicationManager] TopicIDLModel connected successfully";
        
        // Try to generate code immediately if data exists
        if (!m_topicIDLModel->textData().isEmpty()) {
            qDebug() << "[PublicationManager] IDL data already available, generating code";
            generateSampleCode();
        } else {
            qDebug() << "[PublicationManager] IDL data not yet available, waiting...";
            setStatusMessage("Waiting for topic discovery...");
        }
    }
}

void PublicationManager::onTopicIDLTextDataChanged()
{
    qDebug() << "[PublicationManager] IDL text data changed, regenerating sample code";
    generateSampleCode();
}

void PublicationManager::generateSampleCode()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] generateSampleCode() called";
    
    if (!m_topicIDLModel) {
        qWarning() << "[PublicationManager] ✗ topicIDLModel is NULL!";
        setStatusMessage("Error: Topic model not connected");
        qDebug() << "[PublicationManager] ========================================";
        return;
    }
    
    qDebug() << "[PublicationManager] ✓ topicIDLModel is valid";

    if (m_topicName.isEmpty() && !m_topicIDLModel->topicName().isEmpty())
    {
        m_topicName = m_topicIDLModel->topicName();
        emit topicNameChanged();
        qDebug() << "[PublicationManager] Topic name synchronized from TopicIDLModel:" << m_topicName;
    }
    
    QString idlText = m_topicIDLModel->textData();
    qDebug() << "[PublicationManager] IDL text length:" << idlText.length();
    qDebug() << "[PublicationManager] IDL text content:\n" << idlText;
    
    if (idlText.isEmpty()) {
        qWarning() << "[PublicationManager] ⚠ IDL text is EMPTY. Waiting for discovered topic type.";
        m_topicType.clear();
        m_topicFields.clear();
        m_sampleCode.clear();
        emit topicTypeChanged();
        emit sampleCodeChanged();
        setStatusMessage("Waiting for discovered IDL...");
        emit logMessage("⚠ Waiting for discovered IDL before Execute/Publish");
        qDebug() << "[PublicationManager] ========================================";
        return;
    }

    
    qDebug() << "[PublicationManager] Generating sample code from IDL:\n" << idlText;
    
    // Extract struct name and fields
    m_topicType = extractStructName(idlText);
    m_topicFields = extractFields(idlText);
    
    if (m_topicFields.isEmpty()) {
        qWarning() << "[PublicationManager] Failed to extract fields from IDL";
        setStatusMessage("Error: Failed to parse topic structure");
        return;
    }
    
    qDebug() << "[PublicationManager] Extracted" << m_topicFields.size() << "fields from type:" << m_topicType;
    
    // Generate Python-style code
    m_sampleCode = generatePythonCode(m_topicFields);
    
    qDebug() << "[PublicationManager] ========================================";
qDebug() << "[PublicationManager] EMITTING SIGNALS...";
qDebug() << "[PublicationManager] About to emit topicTypeChanged()";
emit topicTypeChanged();
qDebug() << "[PublicationManager] About to emit sampleCodeChanged()";
emit sampleCodeChanged();
qDebug() << "[PublicationManager] sampleCode property value:";
qDebug() << m_sampleCode;
qDebug() << "[PublicationManager] ========================================";
setStatusMessage("Ready to execute ✓");

    
    qDebug() << "[PublicationManager] Sample code generated successfully";
    qDebug() << "[PublicationManager] Generated code:\n" << m_sampleCode;
}


QString PublicationManager::extractStructName(const QString& idlText)
{
    QRegularExpression structRegex(R"(struct\s+(\w+)\s*\{)");
    QRegularExpressionMatch match = structRegex.match(idlText);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    return "UnknownType";
}

QVariantMap PublicationManager::extractFields(const QString& idlText)
{
    QVariantMap fields;
    
    // Match struct definition
    QRegularExpression structRegex(R"(struct\s+\w+\s*\{([^}]+)\})");
    QRegularExpressionMatch structMatch = structRegex.match(idlText);
    
    if (!structMatch.hasMatch()) {
        qWarning() << "[PublicationManager] Failed to match struct pattern";
        return fields;
    }
    
    QString bodyText = structMatch.captured(1);
    QStringList lines = bodyText.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        
        // Skip comments and empty lines
        if (trimmed.startsWith("//") || trimmed.isEmpty()) {
            continue;
        }
        
        // Remove trailing comment and semicolon
        int commentPos = trimmed.indexOf("//");
        if (commentPos >= 0) {
            trimmed = trimmed.left(commentPos).trimmed();
        }
        trimmed = trimmed.remove(';').trimmed();
        
        // Parse: "type fieldName"
        QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        
        if (parts.size() >= 2) {
            QString fieldType = parts[0];
            QString fieldName = parts[1];
            
            // Handle arrays
            if (fieldName.contains('[')) {
                fieldName = fieldName.left(fieldName.indexOf('['));
                fieldType += "[]";
            }
            
            fields[fieldName] = fieldType;
            qDebug() << "[PublicationManager] Field:" << fieldName << "Type:" << fieldType;
        }
    }
    
    return fields;
}

QString PublicationManager::generatePythonCode(const QVariantMap& fields)
{
    QString code;
    code += QString("# Sample code for topic: %1\n").arg(m_topicName);
    code += QString("# Type: %1\n\n").arg(m_topicType);
    
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
        QString fieldName = it.key();
        QString fieldType = it.value().toString();
        QVariant defaultValue = getDefaultValue(fieldType);
        
        QString lowerType = fieldType.toLower();
        
        if (lowerType.contains("string")) {
            code += QString("sample.%1 = \"%2\"\n").arg(fieldName, defaultValue.toString());
        }
        else if (lowerType.contains("bool")) {
            code += QString("sample.%1 = %2\n").arg(fieldName, defaultValue.toBool() ? "True" : "False");
        }
        else if (lowerType.contains("int") || lowerType.contains("long") || lowerType.contains("short")) {
            code += QString("sample.%1 = %2\n").arg(fieldName).arg(defaultValue.toInt());
        }
        else if (lowerType.contains("float") || lowerType.contains("double")) {
            code += QString("sample.%1 = %2\n").arg(fieldName).arg(defaultValue.toDouble());
        }
        else {
            code += QString("sample.%1 = %2\n").arg(fieldName, defaultValue.toString());
        }
    }
    
    return code;
}

QVariant PublicationManager::getDefaultValue(const QString& type)
{
    QString lowerType = type.toLower();
    
    if (lowerType.contains("string")) {
        return QString("example_string");
    }
    else if (lowerType.contains("bool")) {
        return false;
    }
    else if (lowerType.contains("int") || lowerType.contains("short") || lowerType.contains("long")) {
        return 0;
    }
    else if (lowerType.contains("float") || lowerType.contains("double")) {
        return 0.0;
    }
    else if (lowerType.contains("octet") || lowerType.contains("byte")) {
        return 0;
    }
    
    return QString("");
}

QVariantMap PublicationManager::executePythonCode(const QString& code)
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] executePythonCode() called";
    qDebug() << "[PublicationManager] Code length:" << code.length();
    qDebug() << "[PublicationManager] Code content:\n" << code;
    
    QVariantMap result;
    QStringList lines = code.split('\n');
    qDebug() << "[PublicationManager] Total lines to parse:" << lines.size();
    
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        
        if (trimmed.startsWith('#') || trimmed.isEmpty()) {
            qDebug() << "[PublicationManager] Skipping comment/empty line";
            continue;
        }
        
        if (trimmed.startsWith("sample.") && trimmed.contains('=')) {
            qDebug() << "[PublicationManager] Processing line:" << trimmed;
            
            QStringList parts = trimmed.split('=');
            if (parts.size() >= 2) {
                QString fieldName = parts[0].trimmed().remove("sample.");
                QString value = parts[1].trimmed();
                
                qDebug() << "[PublicationManager] Field name:" << fieldName;
                qDebug() << "[PublicationManager] Raw value:" << value;
                
                bool wasQuoted = (parts[1].trimmed().startsWith('"') ||
                                  parts[1].trimmed().startsWith('\''));

                // Remove quotes from the raw value
                value = value.remove('"').remove('\'');
                qDebug() << "[PublicationManager] Value after quote removal:" << value
                         << "wasQuoted:" << wasQuoted;

                if (wasQuoted)
                {
                    // Value was explicitly quoted in the editor → always string.
                    // Do NOT attempt numeric conversion — the user declared this
                    // a string value. setDynamicDataField will call set_string_value.
                    result[fieldName] = value;
                    qDebug() << "[PublicationManager] ✓ Preserved as quoted string:" << value;
                    continue;
                }

                // Unquoted value — infer type from content
                bool ok;
                int intVal = value.toInt(&ok);
                if (ok) {
                    result[fieldName] = intVal;
                    qDebug() << "[PublicationManager] ✓ Parsed as int:" << intVal;
                    continue;
                }

                double doubleVal = value.toDouble(&ok);
                if (ok) {
                    result[fieldName] = doubleVal;
                    qDebug() << "[PublicationManager] ✓ Parsed as double:" << doubleVal;
                    continue;
                }

                if (value.toLower() == "true") {
                    result[fieldName] = true;
                    qDebug() << "[PublicationManager] ✓ Parsed as bool: true";
                } else if (value.toLower() == "false") {
                    result[fieldName] = false;
                    qDebug() << "[PublicationManager] ✓ Parsed as bool: false";
                } else {
                    result[fieldName] = value;
                    qDebug() << "[PublicationManager] ✓ Parsed as string:" << value;
                }
            }
        }
    }
    
    qDebug() << "[PublicationManager] Total fields parsed:" << result.size();
    qDebug() << "[PublicationManager] Result map:" << result;
    
    emit logMessage("✓ Executed code, generated " + QString::number(result.size()) + " fields");
    
    qDebug() << "[PublicationManager] ========================================";
    return result;
}

int PublicationManager::addSampleToQueue(const QVariantMap& sampleData)
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] addSampleToQueue() called";
    qDebug() << "[PublicationManager] Sample data size:" << sampleData.size();
    qDebug() << "[PublicationManager] Sample data content:" << sampleData;
    
    m_sampleQueue.append(sampleData);
    int index = m_sampleQueue.size() - 1;
    
    qDebug() << "[PublicationManager] ✓ Sample added at index:" << index;
    qDebug() << "[PublicationManager] Queue size now:" << m_sampleQueue.size();
    
    emit sampleQueueSizeChanged();
    emit logMessage(QString("✓ Sample #%1 added to queue (total: %2)").arg(index).arg(m_sampleQueue.size()));
    
    qDebug() << "[PublicationManager] ========================================";
    return index;
}

void PublicationManager::clearSampleQueue()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] clearSampleQueue() called";
    qDebug() << "[PublicationManager] Clearing queue with" << m_sampleQueue.size() << "samples";
    
    m_sampleQueue.clear();
    
    qDebug() << "[PublicationManager] ✓ Queue cleared, size now:" << m_sampleQueue.size();
    
    emit sampleQueueSizeChanged();
    emit logMessage("✓ Sample queue cleared (all samples removed)");
    
    qDebug() << "[PublicationManager] ========================================";
}

bool PublicationManager::publishSample(int index)
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] publishSample called with index" << index;
    
    // Validate index
    if (index < 0 || index >= m_sampleQueue.size())
    {
        qWarning() << "[PublicationManager] Invalid sample index:" << index;
        emit errorOccurred("Invalid sample index");
        return false;
    }
    
    // Check controller exists
    if (!m_controller)
    {
        qCritical() << "[PublicationManager] ✗ Controller is NULL!";
        emit errorOccurred("Internal error: Controller not set");
        return false;
    }
    
    // Get the sample data
    QVariantMap sampleData = m_sampleQueue[index];
    qDebug() << "[PublicationManager] Sample data:" << sampleData;
    qDebug() << "[PublicationManager] Topic:" << m_topicName;
    qDebug() << "[PublicationManager] Domain:" << m_domainId;
    
    // Call Controller's publishOneSample method
    bool success = false;
    QMetaObject::invokeMethod(
        m_controller,
        "publishOneSample",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, success),
        Q_ARG(QString, m_topicName),
        Q_ARG(int, m_domainId),
        Q_ARG(QVariantMap, sampleData)
    );
    
    qDebug() << "[PublicationManager] publishOneSample returned:" << success;
    
    if (success)
    {
        qDebug() << "[PublicationManager] ✓ Sample published successfully";
        emit samplePublished(index, true);
        emit logMessage(QString("✓ Sample %1 published successfully").arg(index));
    }
    else
    {
        qWarning() << "[PublicationManager] ✗ Failed to publish sample";
        emit errorOccurred("Failed to publish sample via DDS");
        emit logMessage(QString("✗ Failed to publish sample %1").arg(index));
    }
    
    qDebug() << "[PublicationManager] ========================================";
    return success;
}


void PublicationManager::publishAllSamples()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] publishAllSamples() called";
    qDebug() << "[PublicationManager] Publishing" << m_sampleQueue.size() << "samples";
    
    for (int i = 0; i < m_sampleQueue.size(); ++i) {
        qDebug() << "[PublicationManager] Publishing sample" << i << "of" << m_sampleQueue.size();
        publishSample(i);
    }
    
    qDebug() << "[PublicationManager] ✓ All samples published";
    qDebug() << "[PublicationManager] ========================================";
}

void PublicationManager::startPeriodicPublishing(int periodMs)
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] startPeriodicPublishing() called";
    qDebug() << "[PublicationManager] Period:" << periodMs << "ms";
    qDebug() << "[PublicationManager] Queue size:" << m_sampleQueue.size();
    qDebug() << "[PublicationManager] Topic:" << m_topicName << "Domain:" << m_domainId;

    if (m_sampleQueue.isEmpty()) {
        qWarning() << "[PublicationManager] ⚠ Queue is EMPTY! Cannot start periodic publishing";
        emit errorOccurred("Cannot start periodic publishing - sample queue is empty");
        qDebug() << "[PublicationManager] ========================================";
        return;
    }

    if (!m_controller) {
        qCritical() << "[PublicationManager] ✗ Controller is NULL! Cannot start periodic publishing.";
        emit errorOccurred("Cannot start periodic publishing - controller not set");
        qDebug() << "[PublicationManager] ========================================";
        return;
    }

    m_publishIndex = 0;
    m_isPublishing = true;
    m_publishTimer->start(periodMs);
    
    qDebug() << "[PublicationManager] ✓ Periodic publishing STARTED";
    qDebug() << "[PublicationManager] Starting index:" << m_publishIndex;
    
    emit isPublishingChanged();
    emit logMessage(QString("✓ Started periodic publishing every %1 ms (%2 samples in queue)").arg(periodMs).arg(m_sampleQueue.size()));

    // Publish first sample immediately so users can verify the flow instantly.
    qDebug() << "[PublicationManager] Publishing first sample immediately after START";
    onPeriodicPublishTimeout();

    qDebug() << "[PublicationManager] ========================================";
}

void PublicationManager::stopPeriodicPublishing()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] stopPeriodicPublishing() called";
    qDebug() << "[PublicationManager] Was publishing:" << m_isPublishing;
    qDebug() << "[PublicationManager] Last index published:" << m_publishIndex;
    
    m_isPublishing = false;
    m_publishTimer->stop();
    
    qDebug() << "[PublicationManager] ✓ Periodic publishing STOPPED";
    
    emit isPublishingChanged();
    emit logMessage("⏸ Periodic publishing stopped");
    
    qDebug() << "[PublicationManager] ========================================";
}

void PublicationManager::onPeriodicPublishTimeout()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] onPeriodicPublishTimeout() - TIMER FIRED";
    qDebug() << "[PublicationManager] Queue size:" << m_sampleQueue.size();
    qDebug() << "[PublicationManager] Current index:" << m_publishIndex;
    
    if (m_sampleQueue.isEmpty())
    {
        qWarning() << "[PublicationManager] ⚠ Periodic publish: queue is EMPTY! Stopping...";
        stopPeriodicPublishing();
        return;
    }
    
    qDebug() << "[PublicationManager] Publishing sample at index" << m_publishIndex;
    
    // Get the sample data
    QVariantMap sampleData = m_sampleQueue.at(m_publishIndex);
    qDebug() << "[PublicationManager] Sample data:" << sampleData;

    if (!m_controller)
    {
        qCritical() << "[PublicationManager] ✗ Controller is NULL during periodic publish";
        emit errorOccurred("Periodic publish failed - controller not available");
        stopPeriodicPublishing();
        return;
    }

    bool success = false;
    qDebug() << "[PublicationManager] Calling Controller::publishOneSample()...";
    QMetaObject::invokeMethod(
        m_controller,
        "publishOneSample",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, success),
        Q_ARG(QString, m_topicName),
        Q_ARG(int, m_domainId),
        Q_ARG(QVariantMap, sampleData)
    );
    qDebug() << "[PublicationManager] publishOneSample returned:" << success;

    // Keep signal for backward compatibility with existing QML hooks/loggers.
    emit publishSampleRequest(m_topicName, m_domainId, sampleData);
    if (!success)
    {
        emit errorOccurred(QString("Periodic publish failed for sample #%1").arg(m_publishIndex));
        emit logMessage(QString("✗ Periodic publish FAILED for sample #%1").arg(m_publishIndex));
    }
    else
    {
        emit logMessage(QString("✓ Periodic publish OK for sample #%1").arg(m_publishIndex));
    }
    
    // Move to next sample
    int nextIndex = (m_publishIndex + 1) % m_sampleQueue.size();
    qDebug() << "[PublicationManager] Moving to next sample: index" << nextIndex;
    m_publishIndex = nextIndex;
    
    emit logMessage(QString("⏱ Periodic cycle complete (next: #%1)").arg(m_publishIndex));
    
    qDebug() << "[PublicationManager] ========================================";
}


QVariantList PublicationManager::getAllSamples()
{
    qDebug() << "[PublicationManager] ========================================";
    qDebug() << "[PublicationManager] getAllSamples() called";
    qDebug() << "[PublicationManager] Total samples in queue:" << m_sampleQueue.size();
    
    QVariantList result;
    for (const auto& sample : m_sampleQueue) {
        result.append(sample);
    }
    
    qDebug() << "[PublicationManager] Returning" << result.size() << "samples";
    qDebug() << "[PublicationManager] ========================================";
    
    return result;
}

void PublicationManager::setStatusMessage(const QString& msg)
{
    if (m_statusMessage != msg) {
        m_statusMessage = msg;
        emit statusMessageChanged();
    }
}
