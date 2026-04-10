#ifndef FILELOGGERMODEL_H
#define FILELOGGERMODEL_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QFileInfo>
#include <QDateTime>

class FileLoggerModel : public QObject
{
    Q_OBJECT
    
    // Properties exposed to QML
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(QString fileSize READ fileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(QString messagesWritten READ messagesWritten NOTIFY messagesWrittenChanged)
    Q_PROPERTY(int messagesDropped READ messagesDropped NOTIFY messagesDroppedChanged)
    Q_PROPERTY(QString lastException READ lastException NOTIFY lastExceptionChanged)
    Q_PROPERTY(int maxQueueSize READ maxQueueSize NOTIFY maxQueueSizeChanged)
    Q_PROPERTY(int currentQueueSize READ currentQueueSize NOTIFY currentQueueSizeChanged)
    Q_PROPERTY(int queuePercentage READ queuePercentage NOTIFY queuePercentageChanged)

public:
    explicit FileLoggerModel(QObject *parent = nullptr);
    ~FileLoggerModel();

    // Property getters
    bool running() const { return m_running; }
    QString filePath() const { return m_filePath; }
    QString fileSize() const;
    QString messagesWritten() const;
    int messagesDropped() const { return m_messagesDropped; }
    QString lastException() const { return m_lastException; }
    int maxQueueSize() const { return m_maxQueueSize; }
    int currentQueueSize() const { return m_currentQueueSize; }
    int queuePercentage() const;

    // Invokable methods for QML
    Q_INVOKABLE bool loadLogFile(const QString &path);
    Q_INVOKABLE void startLogging();
    Q_INVOKABLE void stopLogging();
    Q_INVOKABLE void refreshStats();
    Q_INVOKABLE void setQueueSize(int size);

signals:
    void runningChanged();
    void filePathChanged();
    void fileSizeChanged();
    void messagesWrittenChanged();
    void messagesDroppedChanged();
    void lastExceptionChanged();
    void maxQueueSizeChanged();
    void currentQueueSizeChanged();
    void queuePercentageChanged();
    void statsUpdated();

private:
    void updateFileStats();
    void updateDatabaseStats();
    qint64 getFileSize(const QString &path);
    int getMessageCount();
    
    bool m_running;
    QString m_filePath;
    QString m_dbPath;
    qint64 m_fileSize;
    qint64 m_prevFileSize;
    int m_messagesWritten;
    int m_prevMessagesWritten;
    int m_messagesDropped;
    QString m_lastException;
    int m_maxQueueSize;
    int m_currentQueueSize;
    
    QSqlDatabase m_database;
};

#endif // FILELOGGERMODEL_H
