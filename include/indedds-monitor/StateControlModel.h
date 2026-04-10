#ifndef STATECONTROLMODEL_H
#define STATECONTROLMODEL_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSqlDatabase>

class StateControlModel : public QObject
{
    Q_OBJECT

    // Distributed Logger Properties
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)
    Q_PROPERTY(QString applicationKind READ applicationKind NOTIFY applicationKindChanged)
    Q_PROPERTY(QString filterLevel READ filterLevel WRITE setFilterLevel NOTIFY filterLevelChanged)

    // Logger Properties
    Q_PROPERTY(QString printFormat READ printFormat WRITE setPrintFormat NOTIFY printFormatChanged)
    Q_PROPERTY(QString platformVerbosity READ platformVerbosity WRITE setPlatformVerbosity NOTIFY platformVerbosityChanged)
    Q_PROPERTY(QString communicationVerbosity READ communicationVerbosity WRITE setCommunicationVerbosity NOTIFY communicationVerbosityChanged)
    Q_PROPERTY(QString databaseVerbosity READ databaseVerbosity WRITE setDatabaseVerbosity NOTIFY databaseVerbosityChanged)
    Q_PROPERTY(QString entitiesVerbosity READ entitiesVerbosity WRITE setEntitiesVerbosity NOTIFY entitiesVerbosityChanged)
    Q_PROPERTY(QString apiVerbosity READ apiVerbosity WRITE setApiVerbosity NOTIFY apiVerbosityChanged)

    // Command Response Properties
    Q_PROPERTY(QString commandResult READ commandResult NOTIFY commandResultChanged)
    Q_PROPERTY(QString commandInvocation READ commandInvocation NOTIFY commandInvocationChanged)
    Q_PROPERTY(QString commandHostId READ commandHostId NOTIFY commandHostIdChanged)
    Q_PROPERTY(QString commandMessage READ commandMessage NOTIFY commandMessageChanged)
    Q_PROPERTY(QString commandLastUpdate READ commandLastUpdate NOTIFY commandLastUpdateChanged)
    Q_PROPERTY(QString commandAppId READ commandAppId NOTIFY commandAppIdChanged)

public:
    explicit StateControlModel(QObject *parent = nullptr);
    ~StateControlModel();

    // Getters - Distributed Logger
    QString state() const { return m_state; }
    QString lastUpdate() const { return m_lastUpdate; }
    QString applicationKind() const { return m_applicationKind; }
    QString filterLevel() const { return m_filterLevel; }

    // Getters - Logger
    QString printFormat() const { return m_printFormat; }
    QString platformVerbosity() const { return m_platformVerbosity; }
    QString communicationVerbosity() const { return m_communicationVerbosity; }
    QString databaseVerbosity() const { return m_databaseVerbosity; }
    QString entitiesVerbosity() const { return m_entitiesVerbosity; }
    QString apiVerbosity() const { return m_apiVerbosity; }

    // Getters - Command Response
    QString commandResult() const { return m_commandResult; }
    QString commandInvocation() const { return m_commandInvocation; }
    QString commandHostId() const { return m_commandHostId; }
    QString commandMessage() const { return m_commandMessage; }
    QString commandLastUpdate() const { return m_commandLastUpdate; }
    QString commandAppId() const { return m_commandAppId; }

    // Setters
    void setFilterLevel(const QString &level);
    void setPrintFormat(const QString &format);
    void setPlatformVerbosity(const QString &verbosity);
    void setCommunicationVerbosity(const QString &verbosity);
    void setDatabaseVerbosity(const QString &verbosity);
    void setEntitiesVerbosity(const QString &verbosity);
    void setApiVerbosity(const QString &verbosity);

    // Invokable methods
    Q_INVOKABLE void loadStateFromDatabase(const QString &dbPath);
    // In StateControlModel.h, add this public method:
    Q_INVOKABLE void connectToDatabase(QSqlDatabase *database, const QString &dbPath);

    Q_INVOKABLE void refreshState();
    Q_INVOKABLE void applySettings();
    Q_INVOKABLE QString getIndexForVerbosity(const QString &verbosity);
    Q_INVOKABLE QString getIndexForPrintFormat(const QString &format);

signals:
    // Distributed Logger signals
    void stateChanged();
    void lastUpdateChanged();
    void applicationKindChanged();
    void filterLevelChanged();

    // Logger signals
    void printFormatChanged();
    void platformVerbosityChanged();
    void communicationVerbosityChanged();
    void databaseVerbosityChanged();
    void entitiesVerbosityChanged();
    void apiVerbosityChanged();

    // Command Response signals
    void commandResultChanged();
    void commandInvocationChanged();
    void commandHostIdChanged();
    void commandMessageChanged();
    void commandLastUpdateChanged();
    void commandAppIdChanged();

    void settingsApplied();

private:
    void updateFromDatabase();
    void executeCommand(const QString &command, const QString &param);
    QString currentDateTime();

    // Distributed Logger members
    QString m_state;
    QString m_lastUpdate;
    QString m_applicationKind;
    QString m_filterLevel;

    // Logger members
    QString m_printFormat;
    QString m_platformVerbosity;
    QString m_communicationVerbosity;
    QString m_databaseVerbosity;
    QString m_entitiesVerbosity;
    QString m_apiVerbosity;

    // Command Response members
    QString m_commandResult;
    QString m_commandInvocation;
    QString m_commandHostId;
    QString m_commandMessage;
    QString m_commandLastUpdate;
    QString m_commandAppId;

    QSqlDatabase m_database;
    QString m_dbPath;
};

#endif // STATECONTROLMODEL_H
