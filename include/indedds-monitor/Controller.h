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
// along with eProsima Fast DDS Monitor. If not, see <http://www.gnu.org/licenses/>.

/**
 * @file Controller.h
 */

#ifndef _EPROSIMA_FASTDDS_MONITOR_CONTROLLER_H
#define _EPROSIMA_FASTDDS_MONITOR_CONTROLLER_H

#include <QString>
#include <QDebug>
#include <QObject>
#include <QtCharts/QVXYModelMapper>
#include <QSet>
#include <QPair>
#include <QMap>
#include <QVariantMap>

#include "indedds-monitor/backend/backend_types.h"
#include "indedds-monitor/publisher/PublicationManager.h"
#include "indedds-monitor/publisher/HelloWorldPublisher.h"
#include "indedds-monitor/topic_idl_struct.h"

class Engine;
class HelloWorldSubscriber; // Forward declaration
class MatchAnalysesModel; // Forward declaration

enum class ErrorType : int
{
    GENERIC = 0,        //! Generic error, just show the message
    INIT_MONITOR = 1,   //! Error in @c init_monitor. Reopen the @c init dds monitor dialog
    INIT_DS_MONITOR = 2 //! Error in @c init discovery server monitor dialog
};

/**
 * Class to connect the QML js view with the main Engine class.
 * All the methods in the class will be called by the interaction with the view and will call methods in the Engine.
 */
class Controller : public QObject
{
    Q_OBJECT
    
    // Match Analyses Model
    Q_PROPERTY(QObject* matchAnalysesModel READ get_match_analyses_model NOTIFY matchAnalysesModelChanged)

public:
    
    // -----------------------------
    // Admin Tool Features (Qt5/Linux)
    // -----------------------------
    Q_INVOKABLE QString launch_rnr_controller();

    // Database Integration Service (start/launch/stop)
    // Returns "" on success (terminal launched), otherwise error string.
    Q_INVOKABLE QString db_start_database_service();
    Q_INVOKABLE QString db_launch_database_service();
    Q_INVOKABLE QString db_stop_database_service();

    //! Standard QObject constructor with a reference to the Engine object
    Controller(
        Engine* engine,
        QObject* parent = nullptr);
    
    //! Destructor - cleans up publishers
    ~Controller();

    //! Returns the last error logged
    void send_error(
        QString error_msg,
        ErrorType error_type = ErrorType::GENERIC);

    //! Status counters displayed in the QML
    struct StatusCounters
    {
        std::map<backend::EntityId, uint32_t> errors;
        std::map<backend::EntityId, uint32_t> warnings;
        int32_t total_errors = 0;
        int32_t total_warnings = 0;
    }
    status_counters;

    // Setter for topicIDLModel
    void setTopicIDLModel(TopicIDLStruct* model)
    {
        topicIDLModel_ = model;
        qDebug() << "[Controller] topicIDLModel set to:" << topicIDLModel_;
    }

public slots:

    // Methods to be called from QML

    //! Slot called by init a monitor with a domain number
    void init_monitor(
        int domain);

    //! Slot called when initializing a monitor for a Discovery Server network
    void init_monitor(
        QString discovery_server_guid_prefix,
        QString discovery_server_locators);

    //! Slot called when a Host entity is pressed
    void host_click(
        QString id);

    //! Slot called when a User entity is pressed
    void user_click(
        QString id);

    //! Slot called when a Process entity is pressed
    void process_click(
        QString id);

    //create a dynamic subscriber for selected topic
    void startDynamicSubscriber(const QString& topicName);

    //create a dynamic publisher for selected topic
    void startDynamicPublisher(const QString& topicName);

    void startDynamicDLSubscriber();

    //! Slot called when a Domain entity is pressed
    void domain_click(
        QString id);

    //! Slot called when a Topic entity is pressed
    void topic_click(
        QString id);

    //! Slot called when a Participant entity is pressed
    void participant_click(
        QString id);

    //! Slot called when a Endpoint entity is pressed
    void endpoint_click(
        QString id);

    //! Slot called when a Locator entity is pressed
    void locator_click(
        QString id);

    //! Slot called when refresh button is pressed
    void refresh_click();

    //! Slot to remove all inactive entities from database.
    void clear_entities();

    /**
     * @brief Check if a topic is currently subscribed
     * @param domainId The DDS domain ID
     * @param topicName The name of the topic
     * @return true if subscribed, false otherwise
     */
    Q_INVOKABLE bool isTopicSubscribed(int domainId, const QString& topicName) const;

    /**
     * @brief Unsubscribe from a topic
     * @param domainId The DDS domain ID
     * @param topicName The name of the topic to unsubscribe from
     */
    Q_INVOKABLE void unsubscribeFromTopic(int domainId, const QString& topicName);

    //! Clear all statistics data of all entities received before a time given.
    void clear_statistics_data(
        quint64 time = 0);

    //! Slot to clear the monitor logging information
    void clear_log();

    //! Slot to clear the issues list
    void clear_issues();

    //! Slot called when chart is to be created and ids mst be updated by an entity kind
    void update_available_entity_ids(
        QString entity_kind,
        QString entity_model_id);

    //! Get max qreal possible number
    static qreal get_max_real();

    //! Get max qreal possible number
    static qreal get_min_real();

    //! Get max quint64 possible number
    static quint64 get_max_uint();

    //! Get max quint64 possible number
    static quint64 get_min_uint();

    //! Slot called when chart is to be built
    QtCharts::QVXYModelMapper* add_statistics_data(
        quint64 chartbox_id,
        QString data_kind,
        QString source_entity_id,
        QString target_entity_id,
        quint16 bins,
        quint64 startTime,
        bool startTimeDefault,
        quint64 endTime,
        bool endTimeDefault,
        QString statisticKind);

    //! Returns the eProsima Fast DDS version used to compile de Monitor
    QString fastdds_version();

    //! Returns the eProsima Fast DDS Statistics Backend version used to compile de Monitor
    QString fastdds_statistics_backend_version();

    //! Returns the Qt version used to compile de Monitor
    QString qt_version();

    //! Returns the eProsima Fast DDS Monitor version used to compile de Monitor
    QString fastdds_monitor_version();

    //! Returns the system information for which Fast DDS is built
    QString system_info();

    //! Returns the date on which Fast DDS Monitor was built
    QString build_date();

    //! Returns the Fast DDS Monitor GitHub commit built
    QString git_commit();

    //! Whether the inactive entities must be shown or hidden
    bool inactive_visible();

    //! Change \c inactive_visible status
    void change_inactive_visible();

    //! Whether metatraffic must be shown or hidden
    bool metatraffic_visible();

    //! Change \c metatraffic_visible status
    void change_metatraffic_visible();

    //! Call engine to refresh summary
    void refresh_summary();

    //! Call engine to update a dynamic chartbox
    void update_dynamic_chartbox(
        quint64 chartbox_id,
        quint64 time_to);

    //! Change alias
    void set_alias(
        QString entity_id,
        QString new_alias,
        QString entity_kind);

    //! Give a string with the name of the unit magnitud in which each DataKind is measured
    QString get_data_kind_units(
        QString data_kind);

    /**
     * @brief Export the series given to a new csv file
     *
     * Export one or multiple series to a new csv file.
     * Each series to export is given in a vector as chartobox id and series index to get the data from the models.
     * Each series to export is given with its headers in order to save them in the csv and can import the file.
     *
     * @param file_name path and name to the new csv file
     * @param chartbox_ids ids of the chartboxes of each series
     * @param series_indexes indexes of the serioes inside each chartbox
     * @param data_kinds DataKind that refers to the each series
     * @param chartbox_names Title of the chartbox this series belongs
     * @param label_names Label of each series
     */
    void save_csv(
        QString file_name,
        QList<quint64> chartbox_ids,
        QList<quint64> series_indexes,
        QStringList data_kinds,
        QStringList chartbox_names,
        QStringList label_names);

    /**
     * @brief Dump Fast DDS Statistics Backend's database to a file.
     *
     * @param file_name The name of the file where the database is dumped.
     * @param clear If true, clear all the statistics data of all entities.
     */
    void dump(
        QString file_name,
        bool clear);

    //! Retrive a string list containing the transport protocols supported by the Statistics Backend Discovery Server.
    QStringList ds_supported_transports();

    //! Retrive a string list containing the available statistic kinds.
    QStringList get_statistic_kinds();

    //! Retrive a string list containing the available data kinds.
    QStringList get_data_kinds();

    //! Returns whether the data kind entered requires a target entity to be defined.
    bool data_kind_has_target(
        const QString& data_kind);

    /**
     * @brief Export the series given to a new csv file
     *
     * Export one or multiple series to a new csv file.
     * Each series to export is given in a vector as chartobox id and series index to get the data from the models.
     * Each series to export is given with its headers in order to save them in the csv and can import the file.
     *
     * @param series_id path and name to the new csv file
     * @param series_id ids of the chartboxes of each series
     * @param series_indexes indexes of the serioes inside each chartbox
     * @param data_kinds DataKind that refers to the each series
     * @param chartbox_names Title of the chartbox this series belongs
     * @param label_names Label of each series
     */
    void change_max_points(
        quint64 chartbox_id,
        quint64 series_id,
        quint64 new_max_point);

    //! Request to backend the latest domain view JSON to build the graph
    QString get_domain_view_graph (
        QString domain_id);

    // EXISTING: Publication Manager access
    Q_INVOKABLE PublicationManager* createPublicationManager();
    Q_INVOKABLE void destroyPublicationManager(PublicationManager* manager);

    // ========== NEW METHODS ==========
    
    /**
     * @brief Start publisher with type discovery and callback
     *
     * This method creates a HelloWorldPublisher, initializes it, and waits for type discovery.
     * When type is discovered (or timeout occurs), it emits appropriate signals.
     *
     * @param topicName The name of the topic to publish
     * @param domainId The DDS domain ID
     */
    Q_INVOKABLE void startPublisherWithDiscovery(const QString& topicName, int domainId);

    /*!
     * @brief Publish a single sample to a topic
     * @param topicName The topic name
     * @param domainId The DDS domain ID
     * @param sampleData The sample data as QVariantMap (field -> value)
     * @return true if published successfully, false otherwise
     */
    Q_INVOKABLE bool publishOneSample(const QString& topicName, int domainId, const QVariantMap& sampleData);

    /**
     * @brief Get the Match Analyses Model
     * @return Pointer to MatchAnalysesModel
     */
    QObject* get_match_analyses_model();

signals:

    //! Signal to show the Error Dialog
    void error(
        QString error_msg,
        int error_type);

    //! Signal to inform qml that a new monitor has been initialized
    void monitorInitialized();

    //! Signal to notify status counters have been updated
    void update_status_counters(
        QString errors,
        QString warnings);

    //! Signal to notify Match Analyses Model has changed
    void matchAnalysesModelChanged();

    void publicationCreated(PublicationManager* manager);
    void publicationClosed();

    // ========== NEW SIGNALS ==========
    
    /**
     * @brief Signal emitted when publisher type is successfully discovered
     * @param topicName The name of the topic whose type was discovered
     */
    void publisherTypeDiscovered(const QString& topicName);

    /**
     * @brief Signal emitted when publisher type discovery fails
     * @param topicName The name of the topic
     * @param reason The reason for failure
     */
    void publisherDiscoveryFailed(const QString& topicName, const QString& reason);

protected:

    //! Reference to \c Engine object
    Engine* engine_;

private:

    static void * create_subscriber(void *arg);
    pthread_t subThread;

    // ========== MODIFY THREAD MANAGEMENT ==========
    // OLD: pthread_t subThread; // DON'T USE SINGLE THREAD
    // NEW: Store thread per subscription
    QMap<QPair<int, QString>, pthread_t> subscriberThreads_;
    // ========== END MODIFICATION ==========

    static void * create_publisher(void *arg);
    pthread_t pubThread;

    static void* create_dl_subscriber(void* arg);
    pthread_t dlSubThread;

    QList<PublicationManager*> m_publicationManagers;

    // ========== NEW MEMBERS ==========
    
    //! Pointer to the shared TopicIDLStruct model (set by main.cpp)
    TopicIDLStruct* topicIDLModel_;

    //! Pointer to the Match Analyses Model
    MatchAnalysesModel* match_analyses_model_;

    //! Map of publishers: key = (domainId, topicName), value = publisher instance
    QMap<QPair<int, QString>, HelloWorldPublisher*> m_publisherMap;
    QMap<QPair<int, QString>, QString> m_topicIdlCache_;
    QSet<QPair<int, QString>> m_pendingDiscovery_;
    // Track active subscriptions: QPair<domainId, topicName>
    QSet<QPair<int, QString>> activeSubscriptions_;

    // Map to store subscriber instances for cleanup: Key = <domainId, topicName>, Value = subscriber pointer
    QMap<QPair<int, QString>, HelloWorldSubscriber*> subscriberMap_;
    QMap<QPair<int, QString>, HelloWorldSubscriber*> m_discoverySubscriberMap_;
};

#endif // _EPROSIMA_FASTDDS_MONITOR_CONTROLLER_H
