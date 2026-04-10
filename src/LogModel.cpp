#include "include/indedds-monitor/LogModel.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>


LogModel::LogModel(QObject *parent) : QAbstractListModel(parent), m_autoRefresh(false) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("/home/cdac/indedds/monitor/src/DataBase/logs.db");

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
    }

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &LogModel::refreshLogs);

    loadLogsFromDatabase();
}

LogModel::~LogModel()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_logs.count();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_logs.count())
        return QVariant();

    const LogEntry &log = m_logs[index.row()];

    switch (role)
    {
    case IdRole:
        return log.id;
    case HostIdRole:
        return log.hostId;
    case ProcessRole: // Updated role name
        return log.process;
    case MessageRole:
        return log.message;
    case TimestampRole:
        return log.timestamp;
    case FilenameRole:
        return log.filename;
    case LineRole:
        return log.line;
    case FunctionRole:
        return log.function;
    case CategoryRole:
        return log.category;
    case KindRole:
        return log.kind;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[HostIdRole] = "hostId";
    roles[ProcessRole] = "process";
    roles[MessageRole] = "message";
    roles[TimestampRole] = "timestamp";
    roles[FilenameRole] = "filename";
    roles[LineRole] = "line";
    roles[FunctionRole] = "function";
    roles[CategoryRole] = "category";
    roles[KindRole] = "kind";
    return roles;
}

QVariantMap LogModel::getLog(int index) const
{
    if (index < 0 || index >= m_logs.count())
        return QVariantMap();

    const LogEntry &log = m_logs[index];
    QVariantMap logMap;
    logMap["id"] = log.id;
    logMap["hostId"] = log.hostId;
    logMap["process"] = log.process; // Updated
    logMap["message"] = log.message;
    logMap["timestamp"] = log.timestamp;
    logMap["filename"] = log.filename;
    logMap["line"] = log.line;
    logMap["function"] = log.function;
    logMap["category"] = log.category;
    logMap["kind"] = log.kind;
    return logMap;
}

QVariantList LogModel::getLogs() const
{
    QVariantList logsList;
    for (const LogEntry &log : m_logs)
    {
        QVariantMap logMap;
        logMap["id"] = log.id;
        logMap["hostId"] = log.hostId;
        logMap["process"] = log.process; // Updated
        logMap["message"] = log.message;
        logMap["timestamp"] = log.timestamp;
        logMap["filename"] = log.filename;
        logMap["line"] = log.line;
        logMap["function"] = log.function;
        logMap["category"] = log.category;
        logMap["kind"] = log.kind;
        logsList.append(logMap);
    }
    return logsList;
}

void LogModel::setAutoRefresh(bool autoRefresh)
{
    if (m_autoRefresh != autoRefresh)
    {
        m_autoRefresh = autoRefresh;
        if (m_autoRefresh)
        {
            startRefreshTimer();
        }
        else
        {
            stopRefreshTimer();
        }
        emit autoRefreshChanged();
    }
}

void LogModel::startRefreshTimer()
{
    m_refreshTimer->start(1000); // Refresh every 1000 ms (1 second)
}

void LogModel::stopRefreshTimer()
{
    m_refreshTimer->stop();
}

void LogModel::refreshLogs()
{
    beginResetModel();
    m_logs.clear();
    loadLogsFromDatabase();
    endResetModel();
    emit logCountChanged();
    emit logsChanged();
}

void LogModel::loadLogsFromDatabase()
{
    QSqlQuery query("SELECT * FROM Logs ORDER BY Timestamp DESC");
    while (query.next())
    {
        LogEntry log;
        log.id = query.value("ID").toInt();
        log.hostId = query.value("HostID").toString();
        log.process = query.value("Process").toString(); // Updated
        log.message = query.value("Message").toString();
        log.timestamp = query.value("Timestamp").toString(); // Updated
        log.filename = query.value("Filename").toString();
        log.line = query.value("Line").toInt();
        log.function = query.value("Function").toString();
        log.category = query.value("Category").toString();
        log.kind = query.value("Kind").toString(); // Updated
        m_logs.append(log);
    }
}

bool LogModel::exportToFile(const QString &filePath, const QString &content)
{
    // Clean file path
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file:///")) {
        cleanPath = cleanPath.mid(8);
    } else if (cleanPath.startsWith("file://")) {
        cleanPath = cleanPath.mid(7);
    }
    
    qDebug() << "Writing to file:" << cleanPath;
    
    QFile file(cleanPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write file:" << cleanPath;
        qWarning() << "Error:" << file.errorString();
        return false;
    }
    
    QTextStream out(&file);
    out << content;
    file.close();
    
    qDebug() << "File written successfully";
    return true;
}


