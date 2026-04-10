#include <indedds-monitor/FileLoggerModel.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>

FileLoggerModel::FileLoggerModel(QObject *parent)
    : QObject(parent)
    , m_running(false)
    , m_fileSize(0)
    , m_prevFileSize(0)
    , m_messagesWritten(0)
    , m_prevMessagesWritten(0)
    , m_messagesDropped(0)
    , m_maxQueueSize(5)
    , m_currentQueueSize(0)
{
}

FileLoggerModel::~FileLoggerModel()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool FileLoggerModel::loadLogFile(const QString &path)
{
    QString cleanPath = path;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    
    QFileInfo fileInfo(cleanPath);
    
    // Reset paths
    m_filePath = "";
    m_dbPath = "";
    
    // Check if it's a .log or .db file
    if (fileInfo.suffix() == "log") {
        m_filePath = cleanPath;
        // Assume corresponding .db file exists in same directory
        m_dbPath = fileInfo.absolutePath() + "/logs.db";
        
        // Alternative: try same name with .db extension
        if (!QFile::exists(m_dbPath)) {
            QString baseName = fileInfo.completeBaseName(); // gets "logs" from "logs.log"
            m_dbPath = fileInfo.absolutePath() + "/" + baseName + ".db";
        }
        
    } else if (fileInfo.suffix() == "db") {
        m_dbPath = cleanPath;
        
        // Look for corresponding .log file
        QString baseName = fileInfo.completeBaseName(); // gets "logs" from "logs.db"
        QString logPath = fileInfo.absolutePath() + "/" + baseName + ".log";
        
        if (QFile::exists(logPath)) {
            m_filePath = logPath;
        } else {
            // Try default logs.log
            logPath = fileInfo.absolutePath() + "/logs.log";
            if (QFile::exists(logPath)) {
                m_filePath = logPath;
            }
        }
        
    } else if (fileInfo.isDir()) {
        // Directory selected, look for logs.db and logs.log
        m_dbPath = cleanPath + "/logs.db";
        m_filePath = cleanPath + "/logs.log";
    }
    
    // Validate that we found both files
    qDebug() << "Loading log file:";
    qDebug() << "  DB Path:" << m_dbPath << "Exists:" << QFile::exists(m_dbPath);
    qDebug() << "  Log Path:" << m_filePath << "Exists:" << QFile::exists(m_filePath);
    
    // Check if database exists
    if (!QFile::exists(m_dbPath)) {
        m_lastException = "Database file not found at: " + m_dbPath;
        emit lastExceptionChanged();
        return false;
    }
    
    // Initialize database connection
    // Remove old connection if exists
    if (QSqlDatabase::contains("filelogger_connection")) {
        QSqlDatabase::removeDatabase("filelogger_connection");
    }
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "filelogger_connection");
    m_database.setDatabaseName(m_dbPath);
    
    if (!m_database.open()) {
        m_lastException = "Failed to open database: " + m_database.lastError().text();
        emit lastExceptionChanged();
        return false;
    }
    
    // Check if log file exists (optional, can work with just DB)
    if (m_filePath.isEmpty() || !QFile::exists(m_filePath)) {
        qWarning() << "Log file not found, using database only";
        // Set file path to database path for display purposes
        if (m_filePath.isEmpty()) {
            m_filePath = m_dbPath;
        }
    }
    
    m_running = true;
    m_lastException = ""; // Clear any previous exceptions
    
    emit runningChanged();
    emit filePathChanged();
    emit lastExceptionChanged();
    emit databaseLoaded(); // ADD THIS: Notify that database is ready
    
    refreshStats();
    return true;
}



void FileLoggerModel::refreshStats()
{
    if (!m_running || !m_database.isOpen()) {
        return;
    }
    
    updateFileStats();
    updateDatabaseStats();
    
    emit statsUpdated();
}

void FileLoggerModel::updateFileStats()
{
    if (m_filePath.isEmpty() || !QFile::exists(m_filePath)) {
        return;
    }
    
    m_prevFileSize = m_fileSize;
    m_fileSize = getFileSize(m_filePath);
    emit fileSizeChanged();
}

void FileLoggerModel::updateDatabaseStats()
{
    if (!m_database.isOpen()) {
        return;
    }
    
    // Debug: List all tables
    qDebug() << "Database tables:" << m_database.tables();
    
    m_prevMessagesWritten = m_messagesWritten;
    m_messagesWritten = getMessageCount();
    emit messagesWrittenChanged();
    
    // Query for dropped messages if metadata table exists
    QSqlQuery query(m_database);
    
    // Check what tables exist
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table'")) {
        qDebug() << "Available tables:";
        while (query.next()) {
            qDebug() << "  -" << query.value(0).toString();
        }
    }
    
    // Try to get dropped messages (adjust based on your schema)
    if (query.exec("SELECT COUNT(*) FROM logs WHERE kind = 'DL_Error'")) {
        if (query.next()) {
            // Adjust this query based on your actual schema for dropped messages
            int errorCount = query.value(0).toInt();
            qDebug() << "Error messages count:" << errorCount;
        }
    }
    
    // Update queue size (calculate based on current active messages)
    query.exec("SELECT COUNT(*) FROM logs WHERE timestamp > datetime('now', '-1 minute')");
    if (query.next()) {
        m_currentQueueSize = query.value(0).toInt();
        if (m_currentQueueSize > m_maxQueueSize * 100) {
            m_currentQueueSize = m_maxQueueSize * 100; // Cap at max
        }
        emit currentQueueSizeChanged();
        emit queuePercentageChanged();
    }
}


qint64 FileLoggerModel::getFileSize(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.size();
}

int FileLoggerModel::getMessageCount()
{
    if (!m_database.isOpen()) {
        return 0;
    }
    
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM logs")) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    }
    return 0;
}

QString FileLoggerModel::fileSize() const
{
    qint64 delta = m_fileSize - m_prevFileSize;
    QString sizeStr = QString::number(m_fileSize);
    QString deltaStr = QString(" ( Δ %1 )").arg(delta);
    return sizeStr + (delta != 0 ? deltaStr : " ( Δ 0 )");
}

QString FileLoggerModel::messagesWritten() const
{
    int delta = m_messagesWritten - m_prevMessagesWritten;
    QString countStr = QString::number(m_messagesWritten);
    QString deltaStr = QString(" ( Δ %1 )").arg(delta);
    return countStr + (delta != 0 ? deltaStr : " ( Δ 0 )");
}

int FileLoggerModel::queuePercentage() const
{
    if (m_maxQueueSize == 0) return 0;
    return (m_currentQueueSize * 100) / (m_maxQueueSize * 100);
}

void FileLoggerModel::setQueueSize(int size)
{
    if (m_maxQueueSize != size) {
        m_maxQueueSize = size;
        emit maxQueueSizeChanged();
        emit queuePercentageChanged();
    }
}

void FileLoggerModel::startLogging()
{
    m_running = true;
    m_lastException = "";
    emit runningChanged();
    emit lastExceptionChanged();
}

void FileLoggerModel::stopLogging()
{
    m_running = false;
    emit runningChanged();
}
