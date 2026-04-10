// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.
//
// eProsima Fast DDS Monitor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Monitor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Monitor. If not, see <https://www.gnu.org/licenses/>.

#include <chrono>
#include <sstream>

#include <QDateTime>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QtCore/QRandomGenerator>
#include <qqmlcontext.h>

#include <indedds-monitor/backend/backend_types.h>
#include <indedds-monitor/backend/Listener.h>
#include <indedds-monitor/backend/SyncBackendConnection.h>
#include <indedds-monitor/Controller.h>
#include <indedds-monitor/Engine.h>
#include <indedds-monitor/io/csv.h>
#include <indedds-monitor/model/dds/ParticipantModelItem.h>
#include <indedds-monitor/model/logical/DomainModelItem.h>
#include <indedds-monitor/model/physical/HostModelItem.h>
#include <indedds-monitor/model/statistics/EntityItem.h>
#include <indedds-monitor/model/SubListedListItem.h>
#include <indedds-monitor/model/SubListedListModel.h>
#include <indedds-monitor/model/tree/TreeModel.h>
#include <indedds-monitor/statistics/StatisticsData.h>
#include <indedds-monitor/statistics/dynamic/DynamicStatisticsData.h>
#include <indedds-monitor/statistics/historic/HistoricStatisticsData.h>
#include <indedds-monitor/model/mainwindow.h>
#include <indedds-monitor/model/widget.h>
#include <indedds-monitor/publisher/PublicationManager.h>
#include <indedds-monitor/model/system_overview_model.h>

using EntityInfo = backend::EntityInfo;

Engine::Engine()
        : enabled_(false)
    , callback_worker_thread_(nullptr)
    , callback_worker_(nullptr)
    , use_multithreading_(true)
    , inactive_visible_(true)
    , metatraffic_visible_(false)


{
    qDebug() << "========================================";
    qDebug() << "[Engine::Engine] CONSTRUCTOR STARTED";
    qDebug() << "========================================";
}

QObject *Engine::enable()
{
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    std::cout << "[Engine::enable] FUNCTION STARTED" << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    
    // Initialize async backend
    listener_ = new backend::Listener(this);
    backend_connection_.set_listener(listener_);

    // Initialize models
    participants_model_ = new models::SubListedListModel(new models::ParticipantModelItem());
    fill_dds_data_();

    physical_model_ = new models::SubListedListModel(new models::HostModelItem());
    fill_physical_data_();

    logical_model_ = new models::SubListedListModel(new models::DomainModelItem());
    fill_logical_data_();

    info_model_ = new models::InfoModel();
    fill_first_entity_info_();

    summary_model_ = new models::TreeModel();
    fill_summary_(backend::ID_ALL);

    // Creates a default structure for issue json and fills the tree model with it
    issue_model_ = new models::TreeModel();
    generate_new_issue_info_();
    fill_issue_();

    // Creates a default json structure for logging messages and fills the tree model with it
    log_model_ = new models::TreeModel();
    generate_new_log_info_();
    fill_log_();

    // Creates a default json structure for status messages and fills the tree model with it
    status_model_ = new models::TreeModel();
    generate_new_status_info_();
    fill_status_();

    // Creates a default json structure for statuses and fills the tree model with it
    entity_status_model_ = new models::StatusTreeModel();
    update_entity_status(backend::ID_ALL, backend::StatusKind::INVALID);

    // Creates the proxy model to allow filtering
    entity_status_proxy_model_ = new models::StatusTreeModel();
    entity_status_proxy_model_->set_source_model(entity_status_model_);

    source_entity_id_model_ = new models::ListModel(new models::EntityItem());
    fill_available_entity_id_list_(backend::EntityKind::HOST, "getDataDialogSourceEntityId");
    destination_entity_id_model_ = new models::ListModel(new models::EntityItem());
    fill_available_entity_id_list_(backend::EntityKind::HOST, "getDataDialogDestinationEntityId");

    historic_statistics_data_ = new HistoricStatisticsData();
    dynamic_statistics_data_ = new DynamicStatisticsData();
    controller_ = new Controller(this);

    // Initialize multi-threading components
    std::cout << "[Engine] Initializing callback worker thread..." << std::endl;
    
    // Initialize multi-threading components
    std::cout << "[Engine] Initializing callback worker thread..." << std::endl;
    
    // ========== CRITICAL: Register types for Qt meta-object system ==========
    qRegisterMetaType<backend::Callback>("backend::Callback");
    qRegisterMetaType<backend::StatusCallback>("backend::StatusCallback");
    qRegisterMetaType<backend::EntityUpdatePacket>("backend::EntityUpdatePacket");
    qRegisterMetaType<QQueue<backend::Callback>>("QQueue<backend::Callback>");
    qRegisterMetaType<QQueue<backend::StatusCallback>>("QQueue<backend::StatusCallback>");

    std::cout << "[Engine] ✓ Meta types registered for cross-thread communication" << std::endl;
    
    // Create worker thread
    callback_worker_thread_ = new QThread(this);
    
    // Create worker object
    callback_worker_ = new backend::CallbackWorker(&backend_connection_);
    
    // Move worker to thread
    callback_worker_->moveToThread(callback_worker_thread_);
    
    std::cout << "[Engine::enable] ✓✓✓ CALLBACK WORKER THREAD CREATED ✓✓✓" << std::endl;
    std::cout << "[Engine::enable] Worker thread pointer: " << callback_worker_thread_ << std::endl;
    std::cout << "[Engine::enable] Worker object pointer: " << callback_worker_ << std::endl;

    // ========== CRITICAL: Connect trigger signal to worker ==========
    QObject::connect(
    this,
    &Engine::triggerCallbackProcessing,
    this,
    &Engine::new_callback_slot);

QObject::connect(
    this,
    &Engine::triggerStatusCallbackProcessing,
    this,
    &Engine::new_status_callback_slot);

// Done connections (Worker -> Engine) to re-arm scheduling flags
QObject::connect(
    callback_worker_,
    &backend::CallbackWorker::callbacksBatchDone,
    this,
    &Engine::onWorkerCallbacksBatchDone);

QObject::connect(
    callback_worker_,
    &backend::CallbackWorker::statusBatchDone,
    this,
    &Engine::onWorkerStatusBatchDone);
    
    std::cout << "[Engine::enable] ✓ Signal triggerCallbackProcessing connected to worker" << std::endl;
    
    // Connect worker result signals back to Engine
    QObject::connect(
        callback_worker_,
        &backend::CallbackWorker::entityUpdateReady,
        this,
        &Engine::onEntityUpdateReady);
    
    // Connect worker signals to Engine slots
    QObject::connect(
        callback_worker_,
        &backend::CallbackWorker::entityUpdateReady,
        this,
        &Engine::onEntityUpdateReady);
    
    QObject::connect(
        callback_worker_,
        &backend::CallbackWorker::statusUpdateReady,
        this,
        &Engine::onStatusUpdateReady);
    
    QObject::connect(
        callback_worker_,
        &backend::CallbackWorker::errorOccurred,
        this,
        [this](const QString& error) {
            qCritical() << "[Engine] Worker error:" << error;
        });
    

    system_overview_update_timer_.setSingleShot(true);
    system_overview_update_timer_.setInterval(250); // 200–500ms is fine

    QObject::connect(&system_overview_update_timer_, &QTimer::timeout, this, [this]()
    {
        system_overview_update_pending_.store(false);
        updateSystemOverviewPanel();
    });

    // Start worker thread
    callback_worker_thread_->start();
    std::cout << "[Engine::enable] ✓✓✓ WORKER THREAD STARTED ✓✓✓" << std::endl;
    qDebug() << "[Engine::Engine] Main UI thread ID:" << QThread::currentThreadId();

    callback_worker_->start();
    
    qDebug() << "[Engine] Callback worker thread started successfully";

    // Set the initial time
    initial_time_ = std::chrono::system_clock::now();

    // Initialized qml
    rootContext()->setContextProperty("participantModel", participants_model_);
    rootContext()->setContextProperty("hostModel", physical_model_);
    rootContext()->setContextProperty("domainModel", logical_model_);

    rootContext()->setContextProperty("qosModel", info_model_);
    rootContext()->setContextProperty("summaryModel", summary_model_);
    rootContext()->setContextProperty("issueModel", issue_model_);  
    rootContext()->setContextProperty("logModel", log_model_);
    rootContext()->setContextProperty("statusModel", status_model_);
    rootContext()->setContextProperty("entityStatusModel", entity_status_proxy_model_);

    rootContext()->setContextProperty("entityModelFirst", source_entity_id_model_);
    rootContext()->setContextProperty("entityModelSecond", destination_entity_id_model_);

    rootContext()->setContextProperty("historicData", historic_statistics_data_);
    rootContext()->setContextProperty("dynamicData", dynamic_statistics_data_);
    rootContext()->setContextProperty("controller", controller_);

    rootContext()->setContextProperty("topicIDLModel", &topicIDLModel);
    rootContext()->setContextProperty("dllogmodel", &dllogmodel);

    // ============================================================
    // Register PublicationManager type for QML
    // ============================================================
// In the function that registers QML types:
qmlRegisterType<PublicationManager>("indedds.monitor", 1, 0, "PublicationManager");
qmlRegisterType<TopicIDLStruct>("indedds.monitor", 1, 0, "TopicIDLStruct");    qDebug() << "[Engine] PublicationManager registered for QML usage";
    
    std::vector<std::string> submodel;

    addImportPath(":/imports");
    addImportPath(":/imports/TreeView");

    load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    // load(QUrl(QLatin1String("qrc:/qml/test.qml")));

    // Connect Callback Listener to this object

    // Set enable as True
    enabled_ = true;

    return rootObjects().value(0);
}

Engine::~Engine()
{
    if (enabled_)
    {
        // First free the listener to stop new entities from appear
        if (listener_)
        {
            backend_connection_.unset_listener();
            delete listener_;
        }

        // Stop and cleanup worker thread
        if (callback_worker_)
        {
            qDebug() << "[Engine] Stopping callback worker...";
            callback_worker_->stop();
        }
        
        if (callback_worker_thread_)
        {
            qDebug() << "[Engine] Stopping worker thread...";
            callback_worker_thread_->quit();
            callback_worker_thread_->wait(5000); // Wait up to 5 seconds
            delete callback_worker_thread_;
            qDebug() << "[Engine] Worker thread stopped";
        }
        
        if (callback_worker_)
        {
            delete callback_worker_;
        }

        // View models
        if (participants_model_)
        {
            delete participants_model_;
        }

        if (physical_model_)
        {
            delete physical_model_;
        }

        if (logical_model_)
        {
            delete logical_model_;
        }

        // Info models
        if (summary_model_)
        {
            delete summary_model_;
        }

        if (info_model_)
        {
            delete info_model_;
        }

        if (issue_model_)
        {
            delete issue_model_;
        }

        if (log_model_)
        {
            delete log_model_;
        }

        if (status_model_)
        {
            delete status_model_;
        }

        if (entity_status_model_)
        {
            delete entity_status_model_;
        }

        if (entity_status_proxy_model_)
        {
            delete entity_status_proxy_model_;
        }

        // Auxiliar models
        if (source_entity_id_model_)
        {
            delete source_entity_id_model_;
        }

        if (destination_entity_id_model_)
        {
            delete destination_entity_id_model_;
        }

        // Interactive models
        if (historic_statistics_data_)
        {
            delete historic_statistics_data_;
        }

        if (dynamic_statistics_data_)
        {
            delete dynamic_statistics_data_;
        }
        // WARNING: destroying this object heads to an error in javascript because cannot access methods of null:
        // qrc:/qml/AboutDialog.qml:57: TypeError: Cannot call method '...' of null
        // The elements that fail are:
        // system_info; qt_version; fastdds_statistics_backend_version; fastdds_version;
        // build_date; git_commit; fastdds_monitor_version; inactive_visible
        if (controller_)
        {
            // delete controller_;
        }
    }
}

TopicIDLStruct *Engine::getTopicIDLModel()
{
    return &topicIDLModel;
}

backend::SyncBackendConnection* Engine::get_backend_connection()
{
    return &backend_connection_;
}

void Engine::init_monitor(
    int domain)
{
    std::lock_guard<std::recursive_mutex> lock(initializing_monitor_);

    backend::EntityId domain_id = backend_connection_.init_monitor(domain);

    if (domain_id.is_valid())
    {
        shared_init_monitor_(domain_id);
    }
    else
    {
        process_error(
            "Error trying to initialize monitor with DomainId: " + std::to_string(domain),
            ErrorType::INIT_MONITOR);
    }
}

void Engine::init_monitor(
    QString discovery_server_guid_prefix,
    QString discovery_server_locators)
{
    std::lock_guard<std::recursive_mutex> lock(initializing_monitor_);

    backend::EntityId domain_id = backend_connection_.init_monitor(
        utils::to_string(discovery_server_guid_prefix),
        utils::to_string(discovery_server_locators));

    if (domain_id.is_valid())
    {
        shared_init_monitor_(domain_id);
    }
    else
    {
        process_error(
            "Error trying to initialize monitor in Discovery Server with GUID " +
                utils::to_string(discovery_server_guid_prefix) +
                " and locators " + utils::to_string(discovery_server_locators),
            ErrorType::INIT_DS_MONITOR);
    }
}

void Engine::shared_init_monitor_(
    backend::EntityId domain_id)
{
    // if init_monitor fail it does not arrive here

    if (domain_id.is_valid())
    {
        add_status_domain_(backend_connection_.get_name(domain_id), utils::now());

        // WARNING
        // When a new Domain is created its entities begin to appear in callbacks
        // If the dds model is updated with this model, the callbacks will refer to new entities but they
        // will be already in the model as the model will call backend to update it completly
        // This, entity_clicked must be called but do not update dds model (but reset it)
        update_entity(domain_id, &Engine::update_domain, true, false);

        entity_clicked(domain_id, backend::EntityKind::DOMAIN, false);

        emit controller_->monitorInitialized();
    }
}

bool Engine::fill_entity_info_(
    backend::EntityId id /*ID_ALL*/)
{
    if (id == backend::ID_ALL)
    {
        EntityInfo default_info;
        default_info["No entity"] = "Click over any entity to see its values";
        info_model_->update(default_info);
        info_model_->update_selected_entity(
            backend::entity_kind_to_QString(backend::EntityKind::INVALID),
            "No entity selected",
            "UNKNOWN_APP");
    }
    else
    {
        EntityInfo entity_info = backend_connection_.get_info(id);
        std::string app_id = "UNKNOWN_APP";
        //  if the entity has the property "app_id" defined
        if (entity_info.contains("app_id"))
        {
            // check if the property is a known app_id
            if (std::find(
                    std::begin(backend::app_id_str),
                    std::end(backend::app_id_str),
                    entity_info["app_id"]) != std::end(backend::app_id_str))
            {
                app_id = entity_info["app_id"];
            }
        }
        info_model_->update(entity_info);
        info_model_->update_selected_entity(
            utils::to_QString(entity_info["kind"]),
            utils::to_QString(entity_info["alias"]),
            utils::to_QString(app_id));
    }
    return true;
}

bool Engine::fill_summary_(
    backend::EntityId id /*ID_ALL*/)
{
    summary_model_->update(backend_connection_.get_summary(id));
    return true;
}

bool Engine::fill_issue_()
{
    issue_model_->update(issue_info_);
    return true;
}

bool Engine::fill_log_()
{
    log_model_->update(log_info_);
    return true;
}

bool Engine::fill_status_()
{
    status_model_->update(status_info_);
    return true;
}

void Engine::generate_new_issue_info_()
{
    EntityInfo info;

    info["Issues"] = EntityInfo();

    issue_info_ = info;
}

void Engine::generate_new_log_info_()
{
    EntityInfo info;

    info["Callbacks"] = EntityInfo();

    log_info_ = info;
}

void Engine::generate_new_status_info_()
{
    EntityInfo info;

    info["Entities"] = EntityInfo();
    info["Entities"]["Domains"] = EntityInfo();
    info["Entities"]["Entities"] = 0;

    status_info_ = info;
}

void Engine::sum_entity_number_issue(
    int n)
{
    status_info_["Entities"]["Entities"] = status_info_["Entities"]["Entities"].get<double>() + n;
    fill_status_();
}

bool Engine::add_status_domain_(
    std::string name,
    std::string time)
{
    status_info_["Entities"]["Domains"][time] = name;
    fill_status_();
    add_log_callback_("Monitor initialized in domain " + name, time);

    return true;
}

bool Engine::add_log_callback_(
    std::string callback,
    std::string time)
{
    log_info_["Callbacks"][time] = callback;
    fill_log_();

    return true;
}

void Engine::clear_callback_log_()
{
    log_info_["Callbacks"] = EntityInfo();
    fill_log_();
}

bool Engine::add_issue_info_(
    std::string issue,
    std::string time)
{
    issue_info_["Issues"][time] = issue;
    fill_issue_();

    return true;
}

void Engine::clear_issue_info_()
{
    issue_info_["Issues"] = EntityInfo();
    fill_issue_();
}

bool Engine::fill_first_entity_info_()
{
    EntityInfo info = R"({"No monitors active.":"Start a monitor in a specific domain"})"_json;
    info_model_->update(info);
    return true;
}

/// Backend API
bool Engine::fill_physical_data_()
{
    physical_model_->clear();
    return backend_connection_.update_physical_model(physical_model_, inactive_visible(), metatraffic_visible());
}

//=====================================================================================================
bool Engine::update_host(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_host(physical_model_, id, new_entity, inactive_visible(), metatraffic_visible(),
                                           last_clicked);
}

//=======================================================================================================
bool Engine::update_user(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_user(physical_model_, id, new_entity, inactive_visible(), metatraffic_visible(),
                                           last_clicked);
}
//============================================================================================================

bool Engine::update_process(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    std::cout << "[DEBUG] Entering Engine::update_process for id: " << backend_connection_.get_alias(id) << std::endl;
    // Look for parent id in model
    backend::EntityId parent_id;
    std::vector<backend::EntityId> parents = backend_connection_.get_entities(backend::EntityKind::HOST, id);
    // It must be just one host
    if (parents.size() != 1)
    {
        std::cout << "[DEBUG] Parents related with entity " << id.value() << " are expected to be 1 but are: " << parents.size() << std::endl;
    }
    else
    {
        parent_id = parents[0];
        struct ProcessData data;
        data.host = backend_connection_.get_alias(parent_id);
        std::string str = backend_connection_.get_alias(id);
        std::cout << "[DEBUG] Before std::stoi, str = " << str << std::endl;
        int number = 0;
        try
        {
            number = std::stoi(str);
            std::cout << "[DEBUG] After std::stoi, number = " << number << std::endl;
        }
        catch (const std::invalid_argument &ex)
        {
            std::cerr << "[ERROR] std::invalid_argument in std::stoi: " << ex.what() << std::endl;
        }
        catch (const std::out_of_range &ex)
        {
            std::cerr << "[ERROR] std::out_of_range in std::stoi: " << ex.what() << std::endl;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "[ERROR] Exception in std::stoi: " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "[ERROR] Unknown exception in std::stoi" << std::endl;
        }
        data.id = number;
        main_window_->updateProcessTable(data);
        std::cout << "[DEBUG] Updated process table for id: " << id.value() << std::endl;
    }
    bool result = backend_connection_.update_process(physical_model_, id, new_entity, inactive_visible(),
                                                     metatraffic_visible(), last_clicked);
    std::cout << "[DEBUG] Exiting Engine::update_process for id: " << backend_connection_.get_alias(id) << std::endl;
    return result;
}

//===========================================================================================================

// LOGICAL PARTITION
bool Engine::fill_logical_data_()
{
    logical_model_->clear();
    return backend_connection_.update_logical_model(logical_model_, inactive_visible(), metatraffic_visible());
}

bool Engine::update_domain(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    std::cout << "Domain ID :  ===========" << backend_connection_.get_alias(id) << std::endl;
    return backend_connection_.update_domain(logical_model_, id, new_entity, inactive_visible(), metatraffic_visible(),
                                             last_clicked);
}

// bool Engine::update_topic(
//     const backend::EntityId &id,
//     bool new_entity, /* true */
//     bool last_clicked /* false */)
// {
//     std::cout << "Topic :  ===========" << backend_connection_.get_alias(id) << std::endl;
//     return backend_connection_.update_topic(logical_model_, id, new_entity, inactive_visible(), metatraffic_visible(),
//                                             last_clicked);
// }

bool Engine::update_topic(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    try
    {
        std::cout << "Topic :  ===========" << backend_connection_.get_alias(id) << std::endl;

        return backend_connection_.update_topic(logical_model_, id, new_entity, inactive_visible(), metatraffic_visible(),
                                                last_clicked);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[Error] update_topic failed with exception: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[Error] update_topic failed with unknown exception " << std::endl;
    }

    return false;
}

// DDS PARTITION
bool Engine::fill_dds_data_()
{
    participants_model_->clear();
    return backend_connection_.update_dds_model(participants_model_, last_entities_clicked_.physical_logical.id,
                                                inactive_visible(), metatraffic_visible());
}

void Engine::reset_all_data()
{
    reset_dds_data();
    reset_logic_data();
    reset_physical_data();
}

void Engine::reset_dds_data()
{
    participants_model_->clear();
}

void Engine::reset_logic_data()
{
    logical_model_->clear();
}

void Engine::reset_physical_data()
{
    physical_model_->clear();
}

bool Engine::update_dds_data(
    const backend::EntityId &id /*ID_ALL*/)
{
    return backend_connection_.update_dds_model(participants_model_, id, inactive_visible(), metatraffic_visible());
}

// Update the model with a new or updated entity
bool Engine::update_participant(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_participant(
        participants_model_,
        id,
        new_entity,
        last_entities_clicked_.physical_logical.id,
        inactive_visible(),
        metatraffic_visible(),
        last_clicked);
}

bool Engine::update_datawriter(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_datawriter(
        participants_model_,
        id,
        new_entity,
        inactive_visible(),
        metatraffic_visible(),
        last_clicked);
}

bool Engine::update_datareader(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_datareader(
        participants_model_,
        id,
        new_entity,
        inactive_visible(),
        metatraffic_visible(),
        last_clicked);
}

bool Engine::update_locator(
    const backend::EntityId &id,
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    return backend_connection_.update_locator(
        participants_model_,
        id,
        new_entity,
        inactive_visible(),
        metatraffic_visible(),
        last_clicked);
}

bool Engine::entity_clicked(
    backend::EntityId id,
    backend::EntityKind kind,
    bool update_dds /* = true */,
    bool reset_dds /* = true */)
{
    qDebug() << "Clicked entity: " << id.value();

    auto click_result = last_entities_clicked_.click(id, kind);
    bool res = false;

    if (std::get<2>(click_result).is_set())
    {
        res = update_entity_generic(
                  std::get<2>(click_result).id,
                  std::get<2>(click_result).kind,
                  true,
                  false) ||
              res;
    }

    if (std::get<1>(click_result).is_set())
    {
        res = update_entity_generic(
                  std::get<1>(click_result).id,
                  std::get<1>(click_result).kind,
                  true,
                  false) ||
              res;
    }

    switch (std::get<0>(click_result))
    {
    case EntitiesClicked::EntityKindClicked::all:
        // Reset dds model and update if needed
        if (reset_dds)
        {
            reset_dds_data();
        }
        if (update_dds)
        {
            res = update_dds_data(id) || res;
        }
        break;

    case EntitiesClicked::EntityKindClicked::dds:
        res = update_entity_generic(id, kind, true, true) || res;
        break;

    case EntitiesClicked::EntityKindClicked::logical_physical:
        // Update new entity
        res = update_entity_generic(id, kind, true, true) || res;

        // Reset dds model and update if needed
        if (reset_dds)
        {
            reset_dds_data();
        }
        if (update_dds)
        {
            res = update_dds_data(id) || res;
        }
        break;

    default:
        break;
    }

    // All entities
    res = fill_entity_info_(id) || res;
    res = fill_summary_(id) || res;

    return res;
}

bool Engine::fill_available_entity_id_list_(
    backend::EntityKind entity_kind,
    QString entity_model_id)
{
    return on_selected_entity_kind(entity_kind, entity_model_id);
}

bool Engine::on_selected_entity_kind(
    backend::EntityKind entity_kind,
    QString entity_model_id)
{
    if (entity_model_id == "getDataDialogSourceEntityId")
    {
        source_entity_id_model_->clear();
        return backend_connection_.update_get_data_dialog_entity_id(
            source_entity_id_model_,
            entity_kind,
            inactive_visible(),
            metatraffic_visible());
    }
    else if (entity_model_id == "getDataDialogDestinationEntityId")
    {
        destination_entity_id_model_->clear();
        return backend_connection_.update_get_data_dialog_entity_id(
            destination_entity_id_model_,
            entity_kind,
            inactive_visible(),
            metatraffic_visible());
    }
    else
    {
        return false;
    }
}

QtCharts::QVXYModelMapper *Engine::on_add_statistics_data_series(
    quint64 chartbox_id,
    backend::DataKind data_kind,
    backend::EntityId source_entity_id,
    backend::EntityId target_entity_id,
    quint16 bins,
    quint64 start_time,
    bool start_time_default,
    quint64 end_time,
    bool end_time_default,
    backend::StatisticKind statistic_kind)
{
    backend::Timestamp time_from =
        start_time_default ? initial_time_ : backend::Timestamp(std::chrono::milliseconds(start_time));
    backend::Timestamp time_to =
        end_time_default ? std::chrono::system_clock::now() : backend::Timestamp(std::chrono::milliseconds(end_time));

    std::vector<backend::StatisticsData> statistic_data = backend_connection_.get_data(
        data_kind,
        source_entity_id,
        target_entity_id,
        bins,
        statistic_kind,
        time_from,
        time_to);

    QVector<QPointF> points;
    points.reserve(static_cast<int>(statistic_data.size()));

    for (backend::StatisticsData data : statistic_data)
    {
        if (!std::isnan(data.second)) // This should not be needed as filter is done before... But lets keep it
        {
            points.append(QPointF(
                std::chrono::duration_cast<std::chrono::milliseconds>(data.first.time_since_epoch()).count(),
                data.second));
        }
    }

    // In case there are Nans at the beggining or end of the series, it adapts the chartbox to the real axis
    historic_statistics_data_->newXValue(
        chartbox_id,
        start_time_default
            ? std::chrono::duration_cast<std::chrono::milliseconds>(initial_time_.time_since_epoch()).count()
            : start_time);
    historic_statistics_data_->newXValue(
        chartbox_id,
        end_time_default
            ? std::chrono::duration_cast<std::chrono::milliseconds>(
                  time_to.time_since_epoch())
                  .count()
            : end_time);

    return historic_statistics_data_->add_series(chartbox_id, points);
}

void Engine::refresh_engine(
    bool maintain_clicked /* = false */)
{
    qDebug() << "Refresh engine";

    // Store the entities that were clicked in case it must be reclicked
    auto old_entities_clicked = last_entities_clicked_;
    last_entities_clicked_.unclick();
    reset_all_data();

    fill_physical_data_();
    fill_logical_data_();

    if (!maintain_clicked)
    {
        qDebug() << "Do not keep old clicked in refresh";
        entity_clicked(backend::ID_ALL, backend::EntityKind::INVALID, true, false);
    }
    else
    {
        // Check that physical/logical entity still exist
        if (old_entities_clicked.is_physical_logical_clicked() &&
            backend_connection_.entity_exists(old_entities_clicked.physical_logical.id))
        {
            qDebug() << "Keep old clicked in refresh and setting old entity";

            // Click first the logical physical
            entity_clicked(
                old_entities_clicked.physical_logical.id,
                old_entities_clicked.physical_logical.kind,
                true,
                false);
        }
        else
        {
            qDebug() << "Clicking all";
            entity_clicked(backend::ID_ALL, backend::EntityKind::INVALID, true, false);
        }

        // Check that dds entity still exist
        if (old_entities_clicked.is_dds_clicked() &&
            backend_connection_.entity_exists(old_entities_clicked.dds.id))
        {
            qDebug() << "Keep old clicked in refresh and setting old DDS entity";

            // Click first the logical physical
            entity_clicked(
                old_entities_clicked.dds.id,
                old_entities_clicked.dds.kind,
                true,
                false);
        }
    }
}

void Engine::clear_entities()
{
    backend_connection_.clear_entities();
    refresh_engine(true);
}

void Engine::clear_statistics_data(
    quint64 time_to)
{
    if (time_to == 0)
    {
        backend_connection_.clear_statistics_data();
    }
    else
    {
        // Get time into Timestamp
        auto now = std::chrono::system_clock::now();
        backend::Timestamp time_to_timestamp_ = backend::Timestamp(now - std::chrono::seconds(time_to));
        backend_connection_.clear_statistics_data(time_to_timestamp_);
    }
}

void Engine::clear_log()
{
    clear_callback_log_();
    // In case there are expected callbacks emit signal to read them and
    // update the models from the graphic thread
    emit triggerCallbackProcessing();
}

void Engine::clear_issues()
{
    clear_issue_info_();
}

// ========== DEPRECATED: Old blocking callback processing ==========
// These functions are no longer used - callbacks now processed on worker thread
// Kept for reference only

void Engine::process_callback_queue()
{
    // It iterates while run_ is activate and the queue has elements
    while (!callback_queue_.empty())
    {
        process_callback_();
    }
}

void Engine::process_status_callback_queue()
{
    // It iterates while run_ is activate and the queue has elements
    while (!status_callback_queue_.empty())
    {
        process_status_callback_();
    }
}

bool Engine::are_callbacks_to_process_()
{
    std::lock_guard<std::recursive_mutex> ml(callback_queue_mutex_);
    return !callback_queue_.isEmpty();
}

bool Engine::are_status_callbacks_to_process_()
{
    std::lock_guard<std::recursive_mutex> ml(status_callback_queue_mutex_);
    return !status_callback_queue_.isEmpty();
}

bool Engine::add_callback(
    backend::Callback callback)
{
    std::lock_guard<std::recursive_mutex> ml(callback_queue_mutex_);
    callback_queue_.append(callback);

    // Emit signal to specify there are new data
    emit triggerCallbackProcessing();


    return true;
}

bool Engine::add_callback(
    backend::StatusCallback status_callback)
{
    std::lock_guard<std::recursive_mutex> ml(status_callback_queue_mutex_);
    status_callback_queue_.append(status_callback);

    // Emit signal to specify there are new data
    emit triggerStatusCallbackProcessing();

    return true;
}

void Engine::new_callback_slot()
{
    if (!callback_worker_ || !callback_worker_->isRunning())
    {
        std::cout << "[Engine::new_callback_slot] WARNING: Worker not available!" << std::endl;
        callback_drain_scheduled_.store(false);
        return;
    }

    bool expected = false;
    if (!callback_drain_scheduled_.compare_exchange_strong(expected, true))
    {
        // A batch is already scheduled/in-progress
        return;
    }

    QQueue<backend::Callback> local_batch;
    {
        std::lock_guard<std::recursive_mutex> lock(callback_queue_mutex_);
        if (callback_queue_.isEmpty())
        {
            callback_drain_scheduled_.store(false);
            return;
        }
        local_batch.swap(callback_queue_);
    }

    QMetaObject::invokeMethod(
        callback_worker_,
        "processCallbacksBatch",
        Qt::QueuedConnection,
        Q_ARG(QQueue<backend::Callback>, local_batch));
}



void Engine::new_status_callback_slot()
{
    if (!callback_worker_ || !callback_worker_->isRunning())
    {
        std::cout << "[Engine::new_status_callback_slot] WARNING: Worker not available!" << std::endl;
        status_drain_scheduled_.store(false);
        return;
    }

    bool expected = false;
    if (!status_drain_scheduled_.compare_exchange_strong(expected, true))
    {
        return;
    }

    QQueue<backend::StatusCallback> local_batch;
    {
        std::lock_guard<std::recursive_mutex> lock(status_callback_queue_mutex_);
        if (status_callback_queue_.isEmpty())
        {
            status_drain_scheduled_.store(false);
            return;
        }
        local_batch.swap(status_callback_queue_);
    }

    QMetaObject::invokeMethod(
        callback_worker_,
        "processStatusCallbacksBatch",
        Qt::QueuedConnection,
        Q_ARG(QQueue<backend::StatusCallback>, local_batch));
}




bool Engine::process_callback_()
{
    backend::Callback first_callback;

    {
        std::lock_guard<std::recursive_mutex> ml(callback_queue_mutex_);
        first_callback = callback_queue_.front();
        callback_queue_.pop_front();
    }

    qDebug() << "Processing callback: " << backend::backend_id_to_models_id(first_callback.entity_id);

    return read_callback_(first_callback);
}

bool Engine::process_status_callback_()
{
    backend::StatusCallback first_status_callback;

    {
        std::lock_guard<std::recursive_mutex> ml(status_callback_queue_mutex_);
        first_status_callback = status_callback_queue_.front();
        status_callback_queue_.pop_front();
    }

    qDebug() << "Processing status callback: " << backend::backend_id_to_models_id(first_status_callback.entity_id);

    return read_callback_(first_status_callback);
}

// FIND THIS FUNCTION (around line 1500):
bool Engine::read_callback_(
    backend::Callback callback)
{
    // It should not read callbacks while a domain is being initialized
    std::lock_guard<std::recursive_mutex> lock(initializing_monitor_);

    if (callback.is_update)
    {
        // Add callback of updating entity to log model
        add_log_callback_(
            "Update info in entity " + backend_connection_.get_name(callback.entity_id), 
            utils::now());
    }
    else
    {
        // Add callback to log model
        add_log_callback_(
            "New entity " + backend_connection_.get_name(callback.entity_id) + " discovered", 
            utils::now());

        // Add one to the number of discovered entities
        sum_entity_number_issue(1);
    }

    // ADD THIS LINE HERE (right after sum_entity_number_issue):
    notifySystemOverviewNewEntity(callback.entity_id, callback.entity_kind);

    return update_entity_generic(callback.entity_id, callback.entity_kind, callback.is_update);
}


bool Engine::read_callback_(
    backend::StatusCallback status_callback)
{
    // It should not read callbacks while a domain is being initialized
    std::lock_guard<std::recursive_mutex> lock(initializing_monitor_);

    // Add callback to log model
    add_log_callback_("New entity (" + backend_connection_.get_name(status_callback.entity_id) + ") status reported: " + backend::status_kind_to_string(status_callback.status_kind),
                      utils::now());

    // Remove entities from status layout if needed
    remove_inactive_entities_from_status_model(status_callback.entity_id);

    // update status model
    return update_entity_status(status_callback.entity_id, status_callback.status_kind);
}

bool Engine::update_entity_status(
    const backend::EntityId &id,
    backend::StatusKind kind)
{
    int counter = 0;
    if (id == backend::ID_ALL)
    {
        auto empty_item = new models::StatusTreeItem(backend::ID_ALL,
                                                     std::string("No issues found"), backend::StatusLevel::OK_STATUS, std::string(""));
        entity_status_model_->addTopLevelItem(empty_item);
    }
    else
    {
        backend::StatusLevel new_status = backend::StatusLevel::OK_STATUS;
        std::string description = backend::entity_status_description(kind);

        switch (kind)
        {
        case backend::StatusKind::DEADLINE_MISSED:
        {
            backend::DeadlineMissedSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    std::string handle_string;
                    auto deadline_missed_item = new models::StatusTreeItem(id, kind, std::string("Deadline missed"),
                                                                           sample.status, std::string(""), description);
                    auto total_count_item = new models::StatusTreeItem(id, kind, std::string("Total count:"),
                                                                       sample.status, std::to_string(sample.deadline_missed_status.total_count()), std::string(""));
                    for (uint8_t handler : sample.deadline_missed_status.last_instance_handle())
                    {
                        handle_string = handle_string + std::to_string(handler);
                    }
                    auto last_instance_handle_item = new models::StatusTreeItem(id, kind,
                                                                                std::string("Last instance handle:"), sample.status, handle_string,
                                                                                std::string(""));
                    entity_status_model_->addItem(deadline_missed_item, total_count_item);
                    entity_status_model_->addItem(deadline_missed_item, last_instance_handle_item);
                    entity_status_model_->addItem(entity_item, deadline_missed_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }
        case backend::StatusKind::INCOMPATIBLE_QOS:
        {
            backend::IncompatibleQosSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    std::string fastdds_version = "v2.14.0";
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    auto incompatible_qos_item = new models::StatusTreeItem(id, kind, std::string("Incompatible QoS"),
                                                                            sample.status, std::string(""), description);
                    for (eprosima::fastdds::statistics::QosPolicyCount_s policy :
                         sample.incompatible_qos_status.policies())
                    {
                        if (policy.count() > 0)
                        {
                            auto policy_item = new models::StatusTreeItem(id, kind,
                                                                          std::string(backend::policy_id_to_string(policy.policy_id()) + ":"),
                                                                          sample.status, std::to_string(policy.count()),
                                                                          std::string(
                                                                              "<html><style type=\"text/css\"></style>Check for compatible rules ") +
                                                                              std::string(
                                                                                  "<a href=\"https://fast-dds.docs.eprosima.com/en/") +
                                                                              fastdds_version +
                                                                              std::string("/fastdds/dds_layer/core/policy/standardQosPolicies.html") +
                                                                              backend::policy_documentation_description(policy.policy_id()) +
                                                                              std::string("\">here</a></html>"));
                            entity_status_model_->addItem(incompatible_qos_item, policy_item);
                        }
                    }
                    entity_status_model_->addItem(entity_item, incompatible_qos_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }
        case backend::StatusKind::INCONSISTENT_TOPIC:
        {
            backend::InconsistentTopicSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    auto inconsistent_topic_item =
                        new models::StatusTreeItem(id, kind, std::string("Inconsistent topics:"),
                                                   sample.status, std::to_string(sample.inconsistent_topic_status.total_count()), description);
                    entity_status_model_->addItem(entity_item, inconsistent_topic_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }
        case backend::StatusKind::LIVELINESS_CHANGED:
        {
            backend::LivelinessChangedSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    auto liveliness_changed_item =
                        new models::StatusTreeItem(id, kind, std::string("Liveliness changed"),
                                                   sample.status, std::string(""), description);
                    std::string handle_string;
                    auto alive_count_item = new models::StatusTreeItem(id, kind, std::string("Alive count:"),
                                                                       sample.status, std::to_string(sample.liveliness_changed_status.alive_count()), std::string(""));
                    auto not_alive_count_item = new models::StatusTreeItem(id, kind, std::string("Not alive count:"),
                                                                           sample.status, std::to_string(sample.liveliness_changed_status.not_alive_count()), std::string(""));
                    for (uint8_t handler : sample.liveliness_changed_status.last_publication_handle())
                    {
                        handle_string = handle_string + std::to_string(handler);
                    }
                    auto last_publication_handle_item = new models::StatusTreeItem(id, kind,
                                                                                   std::string(
                                                                                       "Last publication handle:"),
                                                                                   sample.status, handle_string, std::string(""));

                    entity_status_model_->addItem(liveliness_changed_item, alive_count_item);
                    entity_status_model_->addItem(liveliness_changed_item, not_alive_count_item);
                    entity_status_model_->addItem(liveliness_changed_item, last_publication_handle_item);
                    entity_status_model_->addItem(entity_item, liveliness_changed_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }
        case backend::StatusKind::LIVELINESS_LOST:
        {
            backend::LivelinessLostSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    auto liveliness_lost_item = new models::StatusTreeItem(id, kind, std::string("Liveliness lost:"),
                                                                           sample.status, std::to_string(sample.liveliness_lost_status.total_count()), description);
                    entity_status_model_->addItem(entity_item, liveliness_lost_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }
        case backend::StatusKind::SAMPLE_LOST:
        {
            backend::SampleLostSample sample;
            if (backend_connection_.get_status_data(id, sample))
            {
                if (sample.status != backend::StatusLevel::OK_STATUS)
                {
                    backend::StatusLevel entity_status = backend_connection_.get_status(id);
                    auto entity_item = entity_status_model_->getTopLevelItem(
                        id, backend_connection_.get_name(id), entity_status, description);
                    new_status = sample.status;
                    auto samples_lost_item = new models::StatusTreeItem(id, kind, std::string("Samples lost:"),
                                                                        sample.status, std::to_string(sample.sample_lost_status.total_count()), description);
                    entity_status_model_->addItem(entity_item, samples_lost_item);
                    counter = entity_item->recalculate_entity_counter();
                }
            }
            break;
        }

        case backend::StatusKind::CONNECTION_LIST:
        case backend::StatusKind::PROXY:
        // case backend::StatusKind::STATUSES_SIZE:
        default:
        {
            // No entity status updates, as always returns OK
            break;
        }
        }
        if (new_status != backend::StatusLevel::OK_STATUS)
        {
            if (new_status == backend::StatusLevel::ERROR_STATUS)
            {
                std::map<backend::EntityId, uint32_t>::iterator it = controller_->status_counters.errors.find(id);
                if (it != controller_->status_counters.errors.end())
                {
                    controller_->status_counters.total_errors -= controller_->status_counters.errors[id];
                }
                controller_->status_counters.errors[id] = counter;
                controller_->status_counters.total_errors += controller_->status_counters.errors[id];
            }
            else if (new_status == backend::StatusLevel::WARNING_STATUS)
            {
                std::map<backend::EntityId, uint32_t>::iterator it = controller_->status_counters.warnings.find(id);
                if (it != controller_->status_counters.warnings.end())
                {
                    controller_->status_counters.total_warnings -= controller_->status_counters.warnings[id];
                }
                controller_->status_counters.warnings[id] = counter;
                controller_->status_counters.total_warnings += controller_->status_counters.warnings[id];
            }
            // notify status model layout changed to refresh layout view
            emit entity_status_proxy_model_->layoutAboutToBeChanged();

            emit controller_->update_status_counters(
                QString::number(controller_->status_counters.total_errors),
                QString::number(controller_->status_counters.total_warnings));

            // remove empty message if exists
            if (entity_status_model_->is_empty())
            {
                entity_status_model_->removeEmptyItem();
            }

            // update view
            entity_status_proxy_model_->set_source_model(entity_status_model_);

            // notify status model layout changed to refresh layout view
            emit entity_status_proxy_model_->layoutChanged();
        }
    }
    return true;
}

bool Engine::remove_inactive_entities_from_status_model(
    const backend::EntityId &id)
{
    // check if there are entities in the status model
    if (!entity_status_model_->is_empty())
    {
        // get info from id
        EntityInfo entity_info = backend_connection_.get_info(id);

        // update status model if not alive
        if (!entity_info["alive"])
        {
            // remove item from tree
            entity_status_model_->removeItem(entity_status_model_->getTopLevelItem(id, "",
                                                                                   backend::StatusLevel::OK_STATUS, ""));

            // add empty item if removed last item
            if (entity_status_model_->rowCount(entity_status_model_->rootIndex()) == 0)
            {
                entity_status_model_->addTopLevelItem(new models::StatusTreeItem(
                    backend::ID_ALL, std::string("No issues found"), backend::StatusLevel::OK_STATUS,
                    std::string("")));
            }

            // update error counter
            std::map<backend::EntityId, uint32_t>::iterator err_it = controller_->status_counters.errors.find(id);
            if (err_it != controller_->status_counters.errors.end())
            {
                // element found;
                controller_->status_counters.total_errors -= err_it->second;
                if (controller_->status_counters.total_errors < 0)
                {
                    controller_->status_counters.total_errors = 0;
                }
            }
            controller_->status_counters.errors.erase(id);

            // update warning counter
            std::map<backend::EntityId, uint32_t>::iterator warn_it = controller_->status_counters.warnings.find(id);
            if (warn_it != controller_->status_counters.warnings.end())
            {
                // element found;
                controller_->status_counters.total_warnings -= warn_it->second;
                if (controller_->status_counters.total_warnings < 0)
                {
                    controller_->status_counters.total_warnings = 0;
                }
            }
            controller_->status_counters.warnings.erase(id);

            // refresh layout
            emit entity_status_proxy_model_->layoutAboutToBeChanged();

            emit controller_->update_status_counters(
                QString::number(controller_->status_counters.total_errors),
                QString::number(controller_->status_counters.total_warnings));

            // update view
            entity_status_proxy_model_->set_source_model(entity_status_model_);

            // notify status model layout changed to refresh layout view
            emit entity_status_proxy_model_->layoutChanged();
        }
        return true;
    }
    return false;
}

bool Engine::update_entity_generic(
    backend::EntityId entity_id,
    backend::EntityKind entity_kind,
    bool is_update /* false */,
    bool is_last_clicked /* false */)
{
    switch (entity_kind)
    {
    case backend::EntityKind::HOST:
        return update_entity(
            entity_id, &Engine::update_host, !is_update, is_last_clicked);

    case backend::EntityKind::USER:
        return update_entity(
            entity_id, &Engine::update_user, !is_update, is_last_clicked);

    case backend::EntityKind::PROCESS:
        return update_entity(
            entity_id, &Engine::update_process, !is_update, is_last_clicked);

    case backend::EntityKind::DOMAIN:
        return update_entity(
            entity_id, &Engine::update_domain, !is_update, is_last_clicked);

    case backend::EntityKind::TOPIC:
        return update_entity(
            entity_id, &Engine::update_topic, !is_update, is_last_clicked);

    case backend::EntityKind::PARTICIPANT:
        remove_inactive_entities_from_status_model(entity_id);
        return update_entity(
            entity_id, &Engine::update_participant, !is_update, is_last_clicked);

    case backend::EntityKind::DATAWRITER:
        remove_inactive_entities_from_status_model(entity_id);
        return update_entity(
            entity_id, &Engine::update_datawriter, !is_update, is_last_clicked);

    case backend::EntityKind::DATAREADER:
        remove_inactive_entities_from_status_model(entity_id);
        return update_entity(
            entity_id, &Engine::update_datareader, !is_update, is_last_clicked);

    case backend::EntityKind::LOCATOR:
        return update_entity(
            entity_id, &Engine::update_locator, !is_update, is_last_clicked);

    default:
        qWarning() << "Update callback of an Entity with unknown EntityKind";
        return false;
    }
}

void Engine::refresh_summary()
{
    // TODO check what is happening when 2 refreshes are done
    fill_summary_(last_entities_clicked_.dds.id);
}

void Engine::process_error(
    std::string error_msg,
    ErrorType error_type /* = ErrorType::GENERIC */)
{
    add_issue_info_(error_msg, utils::now());
    controller_->send_error(utils::to_QString(error_msg), error_type);
}

void Engine::update_dynamic_chartbox(
    quint64 chartbox_id,
    quint64 time_to)
{
    // Get time into Timestamp
    backend::Timestamp time_to_timestamp_ = backend::Timestamp(std::chrono::milliseconds(time_to));

    /////
    // Get the parameters to get data
    // time_from, data_kind, source_ids, target_ids, statistics_kinds
    const UpdateParameters parameters = dynamic_statistics_data_->get_update_parameters(chartbox_id);

    /////
    // Collect the data for each series and store it in a point vector
    std::map<quint64, QVector<QPointF>> new_series_points;
    for (auto id : parameters.series_ids)
    {
        new_series_points[id] = QVector<QPointF>();
    }

    // Check that source target and kinds has same size
    if (parameters.source_ids.size() != parameters.target_ids.size() ||
        parameters.source_ids.size() != parameters.statistics_kinds.size())
    {
        // BAD PARAMETERS
        qCritical() << "Bad parameters in function update_dynamic_chartbox."
                    << "sources: " << parameters.source_ids.size()
                    << "targets: " << parameters.target_ids.size()
                    << "statistics kind: " << parameters.statistics_kinds.size();

        // Update the model with an empty vector so the time saves coherence in chart
        dynamic_statistics_data_->update(chartbox_id, new_series_points, time_to);
        return;
    }

    for (std::size_t i = 0; i < parameters.series_ids.size(); i++)
    {
        backend::StatisticKind statistics_kind_ = backend::string_to_statistic_kind(parameters.statistics_kinds[i]);

        eprosima::statistics_backend::Timestamp time_from_ =
            backend::Timestamp(std::chrono::milliseconds(parameters.time_from));

        // If statistics_kind is NONE, then the number of bins is 0 to retrieve all the data available
        // Otherwise the bins is 1 so only one data is updated
        uint16_t bins_ = (statistics_kind_ == backend::StatisticKind::NONE) ? 0 : 1;

        std::vector<backend::StatisticsData> new_points;

        if ((statistics_kind_ == backend::StatisticKind::NONE) || !parameters.cumulative[i])
        {
            new_points = backend_connection_.get_data(
                backend::string_to_data_kind(parameters.data_kind),
                backend::models_id_to_backend_id(parameters.source_ids[i]),
                backend::models_id_to_backend_id(parameters.target_ids[i]),
                bins_, // 0 when NONE , 1 otherwise
                statistics_kind_,
                time_from_,          // New limit value
                time_to_timestamp_); // Last time value taken in last call
        }
        else
        {
            if (backend_connection_.data_available(
                    backend::string_to_data_kind(parameters.data_kind),
                    backend::models_id_to_backend_id(parameters.source_ids[i]),
                    backend::models_id_to_backend_id(parameters.target_ids[i]),
                    time_from_,
                    time_to_timestamp_))
            {
                if (parameters.cumulative_interval[i] > 0)
                {
                    time_from_ = time_to_timestamp_ -
                                 std::chrono::seconds(parameters.cumulative_interval[i]);
                }
                else
                {
                    time_from_ = initial_time_;
                }

                new_points = backend_connection_.get_data(
                    backend::string_to_data_kind(parameters.data_kind),
                    backend::models_id_to_backend_id(parameters.source_ids[i]),
                    backend::models_id_to_backend_id(parameters.target_ids[i]),
                    bins_, // 0 when NONE , 1 otherwise
                    statistics_kind_,
                    time_from_,
                    time_to_timestamp_); // Last time value taken in last call
            }
        }

        // Check that get_data call has not failed
        if (new_points.empty())
        {
            qDebug() << "get_data call has not retrieved datapoints in series: " << parameters.series_ids[i];
        }
        else
        {
            for (auto point : new_points)
            {
                if (!std::isnan(point.second))
                {
                    // Add points to list of new points
                    new_series_points[parameters.series_ids[i]].push_back(QPointF(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            point.first.time_since_epoch())
                            .count(),
                        point.second));
                }
            }
        }
    }

    /////
    // Update series with data AND now value
    dynamic_statistics_data_->update(chartbox_id, new_series_points, time_to);
}

void Engine::set_alias(
    const backend::EntityId &entity_id,
    const std::string &new_alias,
    const backend::EntityKind &entity_kind)
{
    backend_connection_.set_alias(entity_id, new_alias);

    if (last_entities_clicked_.dds.id == entity_id)
    {
        EntityInfo entity_info = backend_connection_.get_info(entity_id);
        std::string app_id = "UNKNOWN_APP";
        if (entity_info.contains("app_id"))
        {
            app_id = entity_info["app_id"];
        }
        info_model_->update_selected_entity(
            utils::to_QString(entity_info["kind"]),
            utils::to_QString(new_alias),
            utils::to_QString(app_id));
    }

    // Refresh specific model
    switch (entity_kind)
    {
    case backend::EntityKind::HOST:
        update_entity(entity_id, &Engine::update_host, false);
        break;

    case backend::EntityKind::USER:
        update_entity(entity_id, &Engine::update_user, false);
        break;

    case backend::EntityKind::PROCESS:
        update_entity(entity_id, &Engine::update_process, false);
        break;

    case backend::EntityKind::DOMAIN:
        update_entity(entity_id, &Engine::update_domain, false);
        break;

    case backend::EntityKind::TOPIC:
        update_entity(entity_id, &Engine::update_topic, false);
        break;

    case backend::EntityKind::PARTICIPANT:
        update_entity(entity_id, &Engine::update_participant, false);
        break;

    case backend::EntityKind::DATAWRITER:
        update_entity(entity_id, &Engine::update_datawriter, false);
        break;

    case backend::EntityKind::DATAREADER:
        update_entity(entity_id, &Engine::update_datareader, false);
        break;

    case backend::EntityKind::LOCATOR:
        update_entity(entity_id, &Engine::update_locator, false);
        break;

    default:
        qWarning() << "Updated alias of an unknown EntityKind";
        break;
    }
}

bool Engine::update_entity(
    const backend::EntityId &entity_updated,
    bool (Engine::*update_function)(const backend::EntityId &, bool, bool),
    bool new_entity, /* true */
    bool last_clicked /* false */)
{
    bool res = false;

    // Only update the info if the updated entity is the one that is being shown
    if (last_clicked)
    {
        res = fill_entity_info_(last_entities_clicked_.dds.id) || res;
        res = fill_summary_(last_entities_clicked_.dds.id) || res;
    }

    res = (this->*update_function)(entity_updated, new_entity, last_clicked) || res;

    return res;
}

void Engine::change_inactive_visible()
{
    inactive_visible_ = !inactive_visible_;
    fill_physical_data_();
    fill_logical_data_();
    fill_dds_data_();
    refresh_engine();
}

void Engine::change_metatraffic_visible()
{
    metatraffic_visible_ = !metatraffic_visible_;
    fill_physical_data_();
    fill_logical_data_();
    fill_dds_data_();
    refresh_engine();
}

bool Engine::inactive_visible() const
{
    return inactive_visible_;
}

bool Engine::metatraffic_visible() const
{
    return metatraffic_visible_;
}

std::string Engine::get_data_kind_units(
    const QString &data_kind)
{
    return backend_connection_.get_data_kind_units(backend::string_to_data_kind(data_kind));
}

void Engine::change_max_points(
    quint64 chartbox_id,
    quint64 series_id,
    quint64 new_max_point)
{
    return dynamic_statistics_data_->set_max_points(chartbox_id, series_id, new_max_point);
}

void Engine::save_csv(
    const QString &file_name,
    const QList<quint64> &chartbox_ids,
    const QList<quint64> &series_indexes,
    const QStringList &data_kinds,
    const QStringList &chartbox_names,
    const QStringList &label_names)
{
    // Check input vectors are same size
    int size = chartbox_ids.size();
    assert(series_indexes.size() == size);
    assert(data_kinds.size() == size);
    assert(chartbox_names.size() == size);
    assert(label_names.size() == size);

    // get all data
    std::vector<QVector<QPointF>> datas(size);

    for (int i = 0; i < size; ++i)
    {
        // It looks up if it belongs to dynamic or historic before getting its data
        if (historic_statistics_data_->contains_chartbox(chartbox_ids[i]))
        {
            datas[i] = historic_statistics_data_->get_data(chartbox_ids[i], series_indexes[i]);
        }
        else if (dynamic_statistics_data_->contains_chartbox(chartbox_ids[i]))
        {
            datas[i] = dynamic_statistics_data_->get_data(chartbox_ids[i], series_indexes[i]);
        }
        else
        {
            // Chartbox does not exist
            qCritical() << "Chartbox with id " << chartbox_ids[i] << " does not exist.";
            assert(false);
        }
    }

    // get units
    std::vector<std::string> data_units(size);
    for (int i = 0; i < size; ++i)
    {
        data_units[i] = get_data_kind_units(data_kinds[i]);
    }

    // call csv write
    if (io::HandlerCSV::write_series_to_csv(
            file_name,
            datas,
            data_kinds,
            chartbox_names,
            label_names,
            data_units))
    {
        qDebug() << "CSV file written successfully to file " << file_name;
    }
    else
    {
        qWarning() << "Error writing CSV file " << file_name;
    }
}

void Engine::dump(
    const QString &file_name,
    bool clear)
{
    std::string file = utils::erase_file_substr(file_name); // file_name without file://
    backend_connection_.dump(file, clear);
}

std::vector<std::string> Engine::ds_supported_transports()
{
    return backend_connection_.ds_supported_transports();
}

std::vector<std::string> Engine::get_statistic_kinds()
{
    return backend_connection_.get_statistic_kinds();
}

std::vector<std::string> Engine::get_data_kinds()
{
    return backend_connection_.get_data_kinds();
}

bool Engine::data_kind_has_target(
    const QString &data_kind)
{
    return backend_connection_.data_kind_has_target(backend::string_to_data_kind(data_kind));
}

backend::Graph Engine::get_domain_view_graph(
    const backend::EntityId &domain_id)
{
    return backend_connection_.get_domain_view_graph(domain_id);
}

bool EntityClicked::is_set() const
{
    return kind != backend::EntityKind::INVALID;
}

EntityClicked EntityClicked::reset()
{
    return set(backend::ID_ALL, backend::EntityKind::INVALID);
}

bool EntitiesClicked::is_dds_clicked() const
{
    return dds.is_set();
}

bool EntitiesClicked::is_physical_logical_clicked() const
{
    return physical_logical.is_set();
}

EntityClicked EntityClicked::set(
    backend::EntityId clicked_entity,
    backend::EntityKind clicked_kind)
{
    EntityClicked old = *this;
    id = clicked_entity;
    kind = clicked_kind;
    return old;
}

std::tuple<
    EntitiesClicked::EntityKindClicked,
    EntityClicked,
    EntityClicked>
EntitiesClicked::click(
    backend::EntityId clicked_entity,
    backend::EntityKind clicked_kind)
{
    std::tuple<EntitiesClicked::EntityKindClicked, EntityClicked, EntityClicked> result;

    switch (clicked_kind)
    {
    case backend::EntityKind::HOST:
    case backend::EntityKind::USER:
    case backend::EntityKind::PROCESS:
    case backend::EntityKind::DOMAIN:
    case backend::EntityKind::TOPIC:
        std::get<2>(result) = unclick_dds();
        std::get<1>(result) = physical_logical.set(clicked_entity, clicked_kind);
        std::get<0>(result) = EntitiesClicked::EntityKindClicked::logical_physical;
        break;

    case backend::EntityKind::PARTICIPANT:
    case backend::EntityKind::DATAWRITER:
    case backend::EntityKind::DATAREADER:
    case backend::EntityKind::LOCATOR:
        std::get<2>(result) = dds.set(clicked_entity, clicked_kind);
        std::get<0>(result) = EntitiesClicked::EntityKindClicked::dds;
        break;

    default:
        std::get<2>(result) = unclick_dds();
        std::get<1>(result) = physical_logical.reset();
        std::get<0>(result) = EntitiesClicked::EntityKindClicked::all;
        break;
    }

    return result;
}

std::pair<EntityClicked, EntityClicked> EntitiesClicked::unclick()
{
    return {unclick_dds(), physical_logical.reset()};
}

EntityClicked EntitiesClicked::unclick_dds()
{
    return dds.reset();
}

//==============================================================================
// SYSTEM OVERVIEW PANEL IMPLEMENTATION
//==============================================================================

void Engine::setSystemOverviewModel(SystemOverviewModel* model)
{
    system_overview_model_ = model;
    std::cout << "[Engine] ========== SYSTEM OVERVIEW MODEL CONNECTED ==========" << std::endl;
    std::cout << "[Engine] Model pointer: " << model << std::endl;
    
    if (system_overview_model_)
    {
        // Initialize with existing entities
        updateSystemOverviewPanel();
    }
}


void Engine::updateSystemOverviewPanel()
{
    if (!system_overview_model_)
    {
        std::cout << "[Engine] System Overview Model not set, skipping update" << std::endl;
        return;
    }
    
    std::cout << "[Engine] ========== UPDATING SYSTEM OVERVIEW PANEL ==========" << std::endl;
    
    // Clear existing data
    system_overview_model_->clearAll();
    
    // Get all hosts
    std::vector<backend::EntityId> hosts = backend_connection_.get_entities(
        backend::EntityKind::HOST,
        backend::ID_ALL
    );
    
    std::cout << "[Engine] Found " << hosts.size() << " hosts" << std::endl;
    
    for (const auto& host_id : hosts)
    {
        std::string host_name = backend_connection_.get_name(host_id);
        
        // FIX 1: backend_id_to_models_id() returns QString directly, no need for fromStdString
        QString qhost_id = backend::backend_id_to_models_id(host_id);
        QString qhost_name = QString::fromStdString(host_name);
        
        std::cout << "[Engine]   Adding host: " << host_name << std::endl;
        system_overview_model_->addHost(qhost_name, qhost_id);
        
        // Get participants in this host
        std::vector<backend::EntityId> participants = backend_connection_.get_entities(
            backend::EntityKind::PARTICIPANT,
            host_id
        );
        
        std::cout << "[Engine]   Found " << participants.size() << " participants" << std::endl;
        
        for (const auto& participant_id : participants)
        {
            std::string participant_name = backend_connection_.get_name(participant_id);
            backend::EntityInfo participant_info = backend_connection_.get_info(participant_id);
            
            int domain_id = 0;
            int process_id = 0;
            
            // FIX 2: Extract domain ID with proper string conversion
            if (participant_info.contains("domain_id"))
            {
                try {
                    // Convert json to string first, then to int
                    std::string domain_id_str = participant_info["domain_id"].get<std::string>();
                    domain_id = std::stoi(domain_id_str);
                } catch (...) {
                    std::cout << "[Engine] Warning: Could not parse domain_id" << std::endl;
                }
            }
            
            // FIX 3: Extract process ID with proper string conversion
            if (participant_info.contains("process_id"))
            {
                try {
                    // Convert json to string first, then to int
                    std::string process_id_str = participant_info["process_id"].get<std::string>();
                    process_id = std::stoi(process_id_str);
                } catch (...) {
                    std::cout << "[Engine] Warning: Could not parse process_id" << std::endl;
                }
            }
            
            QString qparticipant_name = QString::fromStdString(participant_name);
            system_overview_model_->addDomainParticipant(qhost_id, qparticipant_name, domain_id, process_id);
            
            // FIX 4: backend_id_to_models_id() returns QString directly
            QString qdp_id = backend::backend_id_to_models_id(participant_id);
            
            // Get DataWriters
            std::vector<backend::EntityId> datawriters = backend_connection_.get_entities(
                backend::EntityKind::DATAWRITER,
                participant_id
            );
            
            QMap<QString, QString> pub_map;
            
            for (const auto& dw_id : datawriters)
            {
                backend::EntityInfo dw_info = backend_connection_.get_info(dw_id);
                
                std::string topic_name = "Unknown";
                if (dw_info.contains("topic"))
                {
                    topic_name = dw_info["topic"].get<std::string>();
                }
                
                // FIX 5: backend_id_to_models_id() returns QString directly
                QString qdw_id = backend::backend_id_to_models_id(dw_id);
                QString qtopic_name = QString::fromStdString(topic_name);
                QString qpub_id = qdp_id + "_PUB_" + qtopic_name;
                
                if (!pub_map.contains(qpub_id))
                {
                    system_overview_model_->addPublisher(qdp_id, qpub_id, "Publisher");
                    pub_map[qpub_id] = qtopic_name;
                }
                
                system_overview_model_->addDataWriter(qpub_id, qdw_id, qtopic_name);
            }
            
            // Get DataReaders
            std::vector<backend::EntityId> datareaders = backend_connection_.get_entities(
                backend::EntityKind::DATAREADER,
                participant_id
            );
            
            QMap<QString, QString> sub_map;
            
            for (const auto& dr_id : datareaders)
            {
                backend::EntityInfo dr_info = backend_connection_.get_info(dr_id);
                
                std::string topic_name = "Unknown";
                if (dr_info.contains("topic"))
                {
                    topic_name = dr_info["topic"].get<std::string>();
                }
                
                // FIX 6: backend_id_to_models_id() returns QString directly
                QString qdr_id = backend::backend_id_to_models_id(dr_id);
                QString qtopic_name = QString::fromStdString(topic_name);
                QString qsub_id = qdp_id + "_SUB_" + qtopic_name;
                
                if (!sub_map.contains(qsub_id))
                {
                    system_overview_model_->addSubscriber(qdp_id, qsub_id, "Subscriber");
                    sub_map[qsub_id] = qtopic_name;
                }
                
                system_overview_model_->addDataReader(qsub_id, qdr_id, qtopic_name);
            }
            
            // Get Topics
            std::vector<backend::EntityId> topics = backend_connection_.get_entities(
                backend::EntityKind::TOPIC,
                participant_id
            );
            
            for (const auto& topic_id : topics)
            {
                std::string topic_name = backend_connection_.get_name(topic_id);
                
                // FIX 7: backend_id_to_models_id() returns QString directly
                QString qtopic_id = backend::backend_id_to_models_id(topic_id);
                QString qtopic_name = QString::fromStdString(topic_name);
                
                system_overview_model_->addTopic(qdp_id, qtopic_id, qtopic_name);
            }
        }
    }
    
    std::cout << "[Engine] ✓✓✓ System Overview Panel updated successfully ✓✓✓" << std::endl;
}



void Engine::notifySystemOverviewNewEntity(
    [[maybe_unused]] const backend::EntityId& entityId,
    [[maybe_unused]] backend::EntityKind entityKind)
{
    if (!system_overview_model_)
    {
        return;
    }

    system_overview_update_pending_.store(true);
    system_overview_update_timer_.start();  // debounce
}

void Engine::onEntityUpdateReady(const backend::EntityUpdatePacket& packet)
{
    qDebug() << "[Engine] Received entity update from worker:" 
             << backend::backend_id_to_models_id(packet.entity_id)
             << "Kind:" << backend::entity_kind_to_QString(packet.entity_kind);
    
    // Update entity on UI thread (safe - we're on UI thread now!)
    update_entity_generic(
        packet.entity_id,
        packet.entity_kind,
        packet.is_new_entity,
        false  // is_last_clicked
    );
    
    qDebug() << "[Engine] Entity update completed on UI thread";
}

void Engine::onStatusUpdateReady(
    const backend::EntityId& entity_id,
    const backend::StatusKind& status_kind)
{
    qDebug() << "[Engine] Received status update from worker:" 
             << backend::backend_id_to_models_id(entity_id);
    
    // Update status on UI thread (safe - we're on UI thread now!)
    update_entity_status(entity_id, status_kind);
    
    qDebug() << "[Engine] Status update completed on UI thread";
}

void Engine::onWorkerCallbacksBatchDone()
{
    callback_drain_scheduled_.store(false);

    // If more callbacks arrived while worker was processing, schedule again.
    if (are_callbacks_to_process_())
    {
        emit triggerCallbackProcessing();
    }
}

void Engine::onWorkerStatusBatchDone()
{
    status_drain_scheduled_.store(false);

    if (are_status_callbacks_to_process_())
    {
        emit triggerStatusCallbackProcessing();
    }
}
