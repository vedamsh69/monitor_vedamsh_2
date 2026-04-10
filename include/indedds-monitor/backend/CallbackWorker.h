// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// CallbackWorker.h - Multi-threaded callback processing worker

#ifndef _EPROSIMA_INDEDDS_MONITOR_BACKEND_CALLBACKWORKER_H_
#define _EPROSIMA_INDEDDS_MONITOR_BACKEND_CALLBACKWORKER_H_

#include <QObject>
#include <QMutex>
#include <atomic>
#include <QString>
#include <QQueue>

#include <indedds-monitor/backend/backend_types.h>
#include <indedds-monitor/backend/Callback.h>
#include <indedds-monitor/backend/StatusCallback.h>

namespace backend {

class SyncBackendConnection;

/**
 * @brief Lightweight data packet for entity updates
 */
struct EntityUpdatePacket
{
    backend::EntityId entity_id;
    backend::EntityKind entity_kind;
    backend::EntityInfo entity_info;
    bool is_new_entity;
    
    EntityUpdatePacket()
        : is_new_entity(false)
    {}
};

/**
 * @brief Worker that processes callbacks on separate thread
 */
class CallbackWorker : public QObject
{
    Q_OBJECT

public:
    explicit CallbackWorker(
        SyncBackendConnection* backend,
        QObject* parent = nullptr);
    
    virtual ~CallbackWorker();

    void start();
    void stop();
    bool isRunning() const { return running_.load(); }

public slots:
    // Main slot - triggered by Engine to process queued callbacks
    
    
    // Individual callback processors
    void processCallback(const backend::Callback& callback);
    void processStatusCallback(const backend::StatusCallback& callback);
    void processCallbacksBatch(const QQueue<backend::Callback>& callbacks);
    void processStatusCallbacksBatch(const QQueue<backend::StatusCallback>& callbacks);



signals:
    void entityUpdateReady(const backend::EntityUpdatePacket& packet);
    void statusUpdateReady(const backend::EntityId& entity_id, 
                          const backend::StatusKind& status_kind);
    void errorOccurred(const QString& error_msg);
    void callbacksBatchDone();
    void statusBatchDone();

private:
    bool processCallbackInternal(const backend::Callback& callback);
    bool processStatusCallbackInternal(const backend::StatusCallback& callback);
    EntityInfo fetchEntityInfo(const backend::EntityId& entity_id);
    EntityUpdatePacket prepareEntityUpdate(
        const backend::EntityId& entity_id,
        const backend::EntityKind& entity_kind,
        bool is_new_entity);

    SyncBackendConnection* backend_connection_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    mutable QMutex mutex_;
    std::atomic<uint64_t> callbacks_processed_{0};
    std::atomic<uint64_t> errors_count_{0};
};

} // namespace backend

Q_DECLARE_METATYPE(backend::EntityUpdatePacket)
Q_DECLARE_METATYPE(QQueue<backend::Callback>)
Q_DECLARE_METATYPE(QQueue<backend::StatusCallback>)

#endif // _EPROSIMA_INDEDDS_MONITOR_BACKEND_CALLBACKWORKER_H_
