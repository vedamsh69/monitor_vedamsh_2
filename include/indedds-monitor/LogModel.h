#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QTimer>

class LogEntry
{
public:
    int id;
    QString hostId;
    QString process; // Updated to QString to match the database schema
    QString message;
    QString timestamp; // Changed to QString
    QString filename;
    int line;
    QString function;
    QString category;
    QString kind; // Changed to QString
};

class LogModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int logCount READ getLogCount NOTIFY logCountChanged)
    Q_PROPERTY(QVariantList logs READ getLogs NOTIFY logsChanged)
    Q_PROPERTY(bool autoRefresh READ autoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)

public:
    enum LogRoles
    {
        IdRole = Qt::UserRole + 1,
        HostIdRole,
        ProcessRole, // Updated role name
        MessageRole,
        TimestampRole,
        FilenameRole,
        LineRole,
        FunctionRole,
        CategoryRole,
        KindRole
    };

    explicit LogModel(QObject *parent = nullptr);
    ~LogModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE bool exportToFile(const QString &filePath, const QString &content);
    Q_INVOKABLE int getLogCount() const { return m_logs.count(); }
    Q_INVOKABLE QVariantMap getLog(int index) const;
    QVariantList getLogs() const;

    bool autoRefresh() const { return m_autoRefresh; }
    void setAutoRefresh(bool autoRefresh);

public slots:
    void refreshLogs();

signals:
    void logCountChanged();
    void logsChanged();
    void autoRefreshChanged();

private:
    QList<LogEntry> m_logs;
    QSqlDatabase m_db;
    QTimer *m_refreshTimer;
    bool m_autoRefresh;

    void loadLogsFromDatabase();
    void startRefreshTimer();
    void stopRefreshTimer();
};

#endif // LOGMODEL_H
