#include "TemperaturePublisher.hpp"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

static const char *PERSISTENCE_GUID = "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64";

TemperaturePublisher::TemperaturePublisher()
    : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new TemperatureReadingPubSubType())
{
}

TemperaturePublisher::~TemperaturePublisher()
{
    if (writer_ != nullptr)
        publisher_->delete_datawriter(writer_);
    if (publisher_ != nullptr)
        participant_->delete_publisher(publisher_);
    if (topic_ != nullptr)
        participant_->delete_topic(topic_);
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool TemperaturePublisher::init()
{
    // Create DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("TemperaturePublisher");

    // Enable propagation of optional QoS (Property QoS) in discovery
    pqos.properties().properties().emplace_back("fastdds.serialize_optional_qos", "true");

    // Optional: process-level persistence plugin config
    pqos.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    pqos.properties().properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");

    // (Participant-level dds.persistence.guid is not required; GUID is endpoint-scoped)
    participant_ = DomainParticipantFactory::get_instance()->create_participant(1, pqos);

    if (participant_ == nullptr)
        return false;

    // Register type support
    type_.get()->auto_fill_type_information(false);
    type_.get()->auto_fill_type_object(true);
    type_.register_type(participant_);

    // Create Topic
    topic_ = participant_->create_topic(
        "TemperatureDataTopic",
        "TemperatureReading",
        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
        return false;

    // Create Publisher
    publisher_ = participant_->create_publisher(
        PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
        return false;

    // Create DataWriter with listener

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.durability().kind = TRANSIENT_DURABILITY_QOS;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    writer_qos.reliability().max_blocking_time = eprosima::fastrtps::Duration_t{1, 0};
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.history().depth = 10000; // or appropriate depth

    writer_qos.resource_limits().max_samples = -1; // unlimited
    writer_qos.resource_limits().max_instances = -1;
    writer_qos.resource_limits().max_samples_per_instance = -1;

    // Always set persistence properties for DataWriter
    writer_qos.properties().properties().emplace_back("dds.persistence.guid", PERSISTENCE_GUID);
    writer_qos.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    writer_qos.properties().properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");

    writer_ = publisher_->create_datawriter(
        topic_, writer_qos, &listener_);
    if (writer_ == nullptr)
        return false;

    return true;
}

bool TemperaturePublisher::publish()
{
    if (listener_.matched_ > 0)
    {
        // 1) Populate fields
        data_.temperature_value(20.0 + count_ * 0.1);
        data_.measurement_unit("Celsius");

        // ISO-8601 timestamp
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
        data_.recorded_time(oss.str());

        // 2) Write and 3) Log
        writer_->write(&data_);
        std::cout << "Sent data: "
                  << data_.temperature_value() << " "
                  << data_.measurement_unit() << " at "
                  << data_.recorded_time() << std::endl;

        ++count_;
        return true;
    }
    return false;
}

void TemperaturePublisher::run()
{
    while (true)
    {
        publish();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    std::cout << "Starting Temperature Publisher." << std::endl;
    TemperaturePublisher publisher;
    if (publisher.init())
    {
        publisher.run();
    }
    return 0;
}
