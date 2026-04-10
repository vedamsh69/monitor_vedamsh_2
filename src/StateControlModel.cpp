#include <indedds-monitor/StateControlModel.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QUrl>  // ADD THIS LINE


StateControlModel::StateControlModel(QObject *parent)
    : QObject(parent)
    , m_state("Operational")
    , m_applicationKind("Hello Distributed Logger C++ Example")
    , m_filterLevel("Trace")
    , m_printFormat("Default")
    , m_platformVerbosity("Trace")
    , m_communicationVerbosity("Trace")
    , m_databaseVerbosity("Trace")
    , m_entitiesVerbosity("Trace")
    , m_apiVerbosity("Trace")
    , m_commandResult("")
    , m_commandInvocation("")
    , m_commandHostId("")
    , m_commandMessage("")
    , m_commandAppId("")
{
    m_lastUpdate = currentDateTime();
}

StateControlModel::~StateControlModel()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

void StateControlModel::loadStateFromDatabase(const QString &dbPath)
{
    QString cleanPath = dbPath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    
    m_dbPath = cleanPath;
    
    if (QFile::exists(m_dbPath)) {
        m_database = QSqlDatabase::addDatabase("QSQLITE", "statecontrol_connection");
        m_database.setDatabaseName(m_dbPath);
        
        if (m_database.open()) {
            refreshState();
        } else {
            qWarning() << "Failed to open database:" << m_database.lastError().text();
        }
    } else {
        qWarning() << "Database file not found:" << m_dbPath;
    }
}

void StateControlModel::connectToDatabase(QSqlDatabase* database, const QString &dbPath)
{
    if (!database || !database->isOpen()) {
        qWarning() << "Invalid database connection provided";
        return;
    }
    
    m_dbPath = dbPath;
    m_database = *database; // Share the connection
    
    refreshState();
}


void StateControlModel::refreshState()
{
    if (!m_database.isOpen()) {
        return;
    }
    
    updateFromDatabase();
    
    m_lastUpdate = currentDateTime();
    emit lastUpdateChanged();
}

void StateControlModel::updateFromDatabase()
{
    if (!m_database.isOpen()) {
        return;
    }
    
    // Query for logger settings from database
    QSqlQuery query(m_database);
    
    // Check if settings table exists
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='logger_settings'")) {
        if (query.next()) {
            // Table exists, load settings
            if (query.exec("SELECT * FROM logger_settings ORDER BY id DESC LIMIT 1")) {
                if (query.next()) {
                    m_filterLevel = query.value("filter_level").toString();
                    m_printFormat = query.value("print_format").toString();
                    m_platformVerbosity = query.value("platform_verbosity").toString();
                    m_communicationVerbosity = query.value("communication_verbosity").toString();
                    m_databaseVerbosity = query.value("database_verbosity").toString();
                    m_entitiesVerbosity = query.value("entities_verbosity").toString();
                    m_apiVerbosity = query.value("api_verbosity").toString();
                    
                    emit filterLevelChanged();
                    emit printFormatChanged();
                    emit platformVerbosityChanged();
                    emit communicationVerbosityChanged();
                    emit databaseVerbosityChanged();
                    emit entitiesVerbosityChanged();
                    emit apiVerbosityChanged();
                }
            }
        }
    }
    
    // Query for command responses
    if (query.exec("SELECT * FROM command_responses ORDER BY timestamp DESC LIMIT 1")) {
        if (query.next()) {
            m_commandResult = query.value("result").toString();
            m_commandInvocation = query.value("invocation").toString();
            m_commandHostId = query.value("host_id").toString();
            m_commandMessage = query.value("message").toString();
            m_commandLastUpdate = query.value("timestamp").toString();
            m_commandAppId = query.value("app_id").toString();
            
            emit commandResultChanged();
            emit commandInvocationChanged();
            emit commandHostIdChanged();
            emit commandMessageChanged();
            emit commandLastUpdateChanged();
            emit commandAppIdChanged();
        }
    }
}

void StateControlModel::setFilterLevel(const QString &level)
{
    if (m_filterLevel != level) {
        m_filterLevel = level;
        emit filterLevelChanged();
        executeCommand("SET_FILTER_LEVEL", level);
    }
}

void StateControlModel::setPrintFormat(const QString &format)
{
    if (m_printFormat != format) {
        m_printFormat = format;
        emit printFormatChanged();
        executeCommand("SET_PRINT_FORMAT", format);
    }
}

void StateControlModel::setPlatformVerbosity(const QString &verbosity)
{
    if (m_platformVerbosity != verbosity) {
        m_platformVerbosity = verbosity;
        emit platformVerbosityChanged();
        executeCommand("SET_PLATFORM_VERBOSITY", verbosity);
    }
}

void StateControlModel::setCommunicationVerbosity(const QString &verbosity)
{
    if (m_communicationVerbosity != verbosity) {
        m_communicationVerbosity = verbosity;
        emit communicationVerbosityChanged();
        executeCommand("SET_COMMUNICATION_VERBOSITY", verbosity);
    }
}

void StateControlModel::setDatabaseVerbosity(const QString &verbosity)
{
    if (m_databaseVerbosity != verbosity) {
        m_databaseVerbosity = verbosity;
        emit databaseVerbosityChanged();
        executeCommand("SET_DATABASE_VERBOSITY", verbosity);
    }
}

void StateControlModel::setEntitiesVerbosity(const QString &verbosity)
{
    if (m_entitiesVerbosity != verbosity) {
        m_entitiesVerbosity = verbosity;
        emit entitiesVerbosityChanged();
        executeCommand("SET_ENTITIES_VERBOSITY", verbosity);
    }
}

void StateControlModel::setApiVerbosity(const QString &verbosity)
{
    if (m_apiVerbosity != verbosity) {
        m_apiVerbosity = verbosity;
        emit apiVerbosityChanged();
        executeCommand("SET_API_VERBOSITY", verbosity);
    }
}

void StateControlModel::applySettings()
{
    if (!m_database.isOpen()) {
        qWarning() << "Database not open";
        return;
    }
    
    QSqlQuery query(m_database);
    
    // Create table if not exists
    query.exec("CREATE TABLE IF NOT EXISTS logger_settings ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "filter_level TEXT, "
               "print_format TEXT, "
               "platform_verbosity TEXT, "
               "communication_verbosity TEXT, "
               "database_verbosity TEXT, "
               "entities_verbosity TEXT, "
               "api_verbosity TEXT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
    
    // Insert new settings
    query.prepare("INSERT INTO logger_settings (filter_level, print_format, platform_verbosity, "
                  "communication_verbosity, database_verbosity, entities_verbosity, api_verbosity) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(m_filterLevel);
    query.addBindValue(m_printFormat);
    query.addBindValue(m_platformVerbosity);
    query.addBindValue(m_communicationVerbosity);
    query.addBindValue(m_databaseVerbosity);
    query.addBindValue(m_entitiesVerbosity);
    query.addBindValue(m_apiVerbosity);
    
    if (query.exec()) {
        m_lastUpdate = currentDateTime();
        emit lastUpdateChanged();
        emit settingsApplied();
        qDebug() << "Settings applied successfully";
    } else {
        qWarning() << "Failed to apply settings:" << query.lastError().text();
    }
}

void StateControlModel::executeCommand(const QString &command, const QString &param)
{
    if (!m_database.isOpen()) {
        return;
    }
    
    QSqlQuery query(m_database);
    
    // Create command responses table if not exists
    query.exec("CREATE TABLE IF NOT EXISTS command_responses ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "result TEXT, "
               "invocation TEXT, "
               "host_id TEXT, "
               "message TEXT, "
               "app_id TEXT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
    
    // Insert command response
    query.prepare("INSERT INTO command_responses (result, invocation, host_id, message, app_id) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue("OK");
    query.addBindValue("1");
    query.addBindValue("101Bed1");
    query.addBindValue(command + ": " + param);
    query.addBindValue("e3902b00");
    
    if (query.exec()) {
        refreshState();
    }
}

QString StateControlModel::getIndexForVerbosity(const QString &verbosity)
{
    QStringList levels = {"Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"};
    return QString::number(levels.indexOf(verbosity));
}

QString StateControlModel::getIndexForPrintFormat(const QString &format)
{
    QStringList formats = {"Default", "Timestamped", "Verbose", "Verbose Timestamped", "Debug", "Minimal", "Maximal"};
    return QString::number(formats.indexOf(format));
}

QString StateControlModel::currentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}
