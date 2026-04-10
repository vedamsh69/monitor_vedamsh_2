#include "TemperatureSubscriber.hpp"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <chrono>
#include <thread>

static const char *PERSISTENCE_GUID = "77.72.69.75.65.72.5f.70.65.72.73.5f|67.75.69.64";

TemperatureSubscriber::TemperatureSubscriber()
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new TemperatureReadingPubSubType())
{
    // Empty
}

TemperatureSubscriber::~TemperatureSubscriber()
{
    // Clean up DDS entities
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    // Delete the participant (removes all contained entities)
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool TemperatureSubscriber::init()
{
    // Create Participant with a name
    DomainParticipantQos pqos;
    pqos.name("TemperatureSubscriber");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(1, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the type
    type_.register_type(participant_);

    // Create Topic (using default QoS)
    topic_ = participant_->create_topic("TemperatureDataTopic",
                                        "TemperatureReading",
                                        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        return false;
    }

    // Create Subscriber (default QoS)
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    // Create DataReader with our listener
    reader_ = subscriber_->create_datareader(topic_,
                                             rqos,
                                             &listener_);
    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

void TemperatureSubscriber::run()
{
    // Run indefinitely; received data is handled in the listener
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char **argv)
{
    std::cout << "Starting Temperature Subscriber." << std::endl;
    TemperatureSubscriber sub;
    if (sub.init())
    {
        sub.run(); // Runs indefinitely
    }
    return 0;
}
