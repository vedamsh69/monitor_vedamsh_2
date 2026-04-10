#ifndef HELLOWORLDSUBSCRIBER_H_
#define HELLOWORLDSUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/rtps/common/Types.h>

#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>

#include <fastrtps/attributes/SubscriberAttributes.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <string>

#include <indedds-monitor/topic_idl_struct.h>

// Declare the global variable for the topic name as extern
extern std::string global_topic_name;

class HelloWorldSubscriber
{
public:
    HelloWorldSubscriber();
    virtual ~HelloWorldSubscriber();
    void print_type_structure(const eprosima::fastrtps::types::DynamicType_ptr &type, int indent = 0);

    // bool init(const std::string& topic_name, std::vector<std::string> submodel);
    bool init(const std::string &topic_name, TopicIDLStruct *topicIDLModel);
    void run();
    void run(uint32_t number);
    void runWithTimeout(uint32_t timeout_seconds);
    void initialize_entities();

    // ========== NEW METHODS ==========
    /**
     * @brief Signal the subscriber to stop gracefully
     */
    void stop();
    
    /**
     * @brief Check if subscriber is running
     */
    bool isRunning() const { return m_running; }
    const std::string& targetTopic() const { return m_targetTopic; }
    // ========== END NEW METHODS ==========


private:
    eprosima::fastdds::dds::DomainParticipant *mp_participant;
    eprosima::fastdds::dds::Subscriber *mp_subscriber;
    std::string get_type_name(eprosima::fastrtps::types::TypeKind kind);

    std::map<eprosima::fastdds::dds::DataReader *, eprosima::fastdds::dds::Topic *> topics_;
    std::map<eprosima::fastdds::dds::DataReader *, eprosima::fastrtps::types::DynamicType_ptr> readers_;
    std::map<eprosima::fastdds::dds::DataReader *, eprosima::fastrtps::types::DynamicData_ptr> datas_;

    eprosima::fastrtps::SubscriberAttributes att_;
    eprosima::fastdds::dds::DataReaderQos qos_;

    //  std::vector<std::string> submodel;
    TopicIDLStruct *topicIDLModel;
    std::string m_targetTopic;

    // ========== NEW MEMBER ==========
    std::atomic<bool> m_running{false};  // Track if subscriber is running
    // ========== END NEW MEMBER ==========

public:
    class SubListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:
        SubListener(HelloWorldSubscriber *sub);
        ~SubListener() override;

        void on_data_available(eprosima::fastdds::dds::DataReader *reader) override;
        void on_subscription_matched(eprosima::fastdds::dds::DataReader *reader, const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) override;
        void on_type_discovery(eprosima::fastdds::dds::DomainParticipant *participant, const eprosima::fastrtps::rtps::SampleIdentity &request_sample_id, const eprosima::fastrtps::string_255 &topic, const eprosima::fastrtps::types::TypeIdentifier *identifier, const eprosima::fastrtps::types::TypeObject *object, eprosima::fastrtps::types::DynamicType_ptr dyn_type) override;
        void on_requested_incompatible_qos(eprosima::fastdds::dds::DataReader *reader,const eprosima::fastdds::dds::RequestedIncompatibleQosStatus &info) override;

        void print_sample_info(const eprosima::fastdds::dds::SampleInfo &info);
        void print_dynamic_data(const eprosima::fastrtps::types::DynamicData_ptr &data, int indent, std::stringstream &dynamicDataStream);
        //void extractSelectedFields(const eprosima::fastrtps::types::DynamicData_ptr &data, std::stringstream &selectedFieldsStream);
        int n_matched;
        uint32_t n_samples;

        std::mutex types_mx_;
        std::condition_variable types_cv_;

        eprosima::fastrtps::types::DynamicType_ptr received_type_;
        std::atomic<bool> reception_flag_{false};

        HelloWorldSubscriber *subscriber_;
    } m_listener;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
