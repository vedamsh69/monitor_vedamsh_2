// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// CallbackWorker.cpp

#include <QDebug>
#include <QThread>
#include <QMutexLocker>

#include <QString>
#include <iostream>

#include <indedds-monitor/backend/CallbackWorker.h>
#include <indedds-monitor/backend/SyncBackendConnection.h>
#include <indedds-monitor/utils.h>

namespace backend {

CallbackWorker::CallbackWorker(
    SyncBackendConnection* backend,
    QObject* parent)
    : QObject(parent)
    , backend_connection_(backend)
    , running_(false)
    , stop_requested_(false)
    , callbacks_processed_(0)
    , errors_count_(0)
{
    qDebug() << "[CallbackWorker] Created on thread:" << QThread::currentThreadId();
    
    if (!backend_connection_)
    {
        qCritical() << "[CallbackWorker] ERROR: Backend is NULL!";
    }
}

CallbackWorker::~CallbackWorker()
{
    qDebug() << "[CallbackWorker] Destructor - Processed:" << callbacks_processed_.load()
             << "Errors:" << errors_count_.load();
    stop();
}



void CallbackWorker::processCallbacksBatch(const QQueue<backend::Callback>& callbacks)
{
    if (!running_.load() || stop_requested_.load())
    {
        emit callbacksBatchDone();
        return;
    }

    for (int i = 0; i < callbacks.size(); ++i)
    {
        const auto& cb = callbacks.at(i);
        try
        {
            if (processCallbackInternal(cb))
            {
                callbacks_processed_.fetch_add(1);
            }
            else
            {
                errors_count_.fetch_add(1);
            }
        }
        catch (const std::exception& e)
        {
            errors_count_.fetch_add(1);
            emit errorOccurred(QString("Exception: %1").arg(e.what()));
        }
        catch (...)
        {
            errors_count_.fetch_add(1);
            emit errorOccurred("Unknown exception");
        }
    }

    emit callbacksBatchDone();
}

void CallbackWorker::processStatusCallbacksBatch(const QQueue<backend::StatusCallback>& callbacks)
{
    if (!running_.load() || stop_requested_.load())
    {
        emit statusBatchDone();
        return;
    }

    for (int i = 0; i < callbacks.size(); ++i)
    {
        const auto& scb = callbacks.at(i);
        try
        {
            if (processStatusCallbackInternal(scb))
            {
                callbacks_processed_.fetch_add(1);
            }
            else
            {
                errors_count_.fetch_add(1);
            }
        }
        catch (const std::exception& e)
        {
            errors_count_.fetch_add(1);
            emit errorOccurred(QString("Status exception: %1").arg(e.what()));
        }
        catch (...)
        {
            errors_count_.fetch_add(1);
            emit errorOccurred("Unknown status exception");
        }
    }

    emit statusBatchDone();
}


void CallbackWorker::start()
{
    QMutexLocker locker(&mutex_);
    
    if (running_.load())
    {
        qWarning() << "[CallbackWorker] Already running";
        return;
    }
    
    qDebug() << "[CallbackWorker] Starting on thread:" << QThread::currentThreadId();
    running_.store(true);
    stop_requested_.store(false);
    callbacks_processed_.store(0);
    errors_count_.store(0);
}

void CallbackWorker::stop()
{
    QMutexLocker locker(&mutex_);
    stop_requested_.store(true);
    running_.store(false);
    qDebug() << "[CallbackWorker] Stopped";
}

void CallbackWorker::processCallback(const backend::Callback& callback)
{
    std::cout << "[CallbackWorker::processCallback] ========== CALLED ==========" << std::endl;
    std::cout << "[CallbackWorker::processCallback] Thread: " 
              << QThread::currentThreadId() << std::endl;
    if (!running_.load() || stop_requested_.load())
    {
        qWarning() << "[CallbackWorker] Not running, skipping callback";
        return;
    }
    
    qDebug() << "[CallbackWorker] Processing entity:" 
             << backend_id_to_models_id(callback.entity_id)
             << "Kind:" << backend::entity_kind_to_QString(callback.entity_kind)
             << "Thread:" << QThread::currentThreadId();
    
    try
    {
        if (processCallbackInternal(callback))
        {
            callbacks_processed_.fetch_add(1);
        }
        else
        {
            errors_count_.fetch_add(1);
        }
    }
    catch (const std::exception& e)
    {
        errors_count_.fetch_add(1);
        qCritical() << "[CallbackWorker] Exception:" << e.what();
        emit errorOccurred(QString("Exception: %1").arg(e.what()));
    }
}

void CallbackWorker::processStatusCallback(const backend::StatusCallback& callback)
{
    if (!running_.load() || stop_requested_.load())
    {
        return;
    }
    
    qDebug() << "[CallbackWorker] Processing status for:" 
             << backend_id_to_models_id(callback.entity_id)
             << "Thread:" << QThread::currentThreadId();
    
    try
    {
        if (processStatusCallbackInternal(callback))
        {
            callbacks_processed_.fetch_add(1);
        }
        else
        {
            errors_count_.fetch_add(1);
        }
    }
    catch (const std::exception& e)
    {
        errors_count_.fetch_add(1);
        qCritical() << "[CallbackWorker] Status exception:" << e.what();
    }
}

bool CallbackWorker::processCallbackInternal(const backend::Callback& callback)
{
    if (!backend_connection_)
    {
        qCritical() << "[CallbackWorker] Backend is NULL!";
        return false;
    }
    
    // Prepare packet OFF UI thread (this is the key!)
    EntityUpdatePacket packet = prepareEntityUpdate(
        callback.entity_id,
        callback.entity_kind,
        !callback.is_update
    );
    
    // Send to UI thread via signal
    qDebug() << "[CallbackWorker] Emitting entityUpdateReady";
    emit entityUpdateReady(packet);
    
    return true;
}

bool CallbackWorker::processStatusCallbackInternal(const backend::StatusCallback& callback)
{
    if (!backend_connection_)
    {
        qCritical() << "[CallbackWorker] Backend is NULL!";
        return false;
    }
    
    qDebug() << "[CallbackWorker] Emitting statusUpdateReady";
    emit statusUpdateReady(callback.entity_id, callback.status_kind);
    
    return true;
}

EntityInfo CallbackWorker::fetchEntityInfo(const backend::EntityId& entity_id)
{
    // BLOCKING call - but on WORKER thread, not UI!
    qDebug() << "[CallbackWorker] Fetching info for:" 
             << backend_id_to_models_id(entity_id)
             << "(UI not blocked!)";
    
    EntityInfo info = backend_connection_->get_info(entity_id);
    
    qDebug() << "[CallbackWorker] Info fetched";
    return info;
}

EntityUpdatePacket CallbackWorker::prepareEntityUpdate(
    const backend::EntityId& entity_id,
    const backend::EntityKind& entity_kind,
    bool is_new_entity)
{
    EntityUpdatePacket packet;
    packet.entity_id = entity_id;
    packet.entity_kind = entity_kind;
    packet.is_new_entity = is_new_entity;
    
    // Fetch info on worker thread
    packet.entity_info = fetchEntityInfo(entity_id);
    
    qDebug() << "[CallbackWorker] Packet prepared for:" 
             << backend_id_to_models_id(entity_id);
    
    return packet;
}

} // namespace backend

