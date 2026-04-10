#ifndef HELLOWORLDPUBLISHER_H
#define HELLOWORLDPUBLISHER_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <indedds-monitor/topic_idl_struct.h>
#include <QVariantMap>
#include <QVariant>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <string>
#include <functional>  // ADD THIS

// Declare the global variable for the topic name as extern
extern std::string global_publisher_topic_name;

class HelloWorldPublisher
{
public:
    HelloWorldPublisher();
    virtual ~HelloWorldPublisher();

    bool init(const std::string &topic_name, int domain_id, TopicIDLStruct *topicIDLModel);

    void run();
    void run(uint32_t number);
    
    // NEW: Non-blocking run with callback when type is discovered
    void runWithCallback(std::function<void(bool)> callback);
    
    // NEW: Check if type discovery is complete
    bool isTypeDiscovered() const { return m_listener.reception_flag_.load(); }

    bool isReady() const { return !datas_.empty() && mp_publisher != nullptr; }

    /*!
     * @brief Wait until type is discovered and entities are initialized
     * @param timeout_ms Maximum time to wait in milliseconds
     * @return true if ready, false if timeout
     */
    bool ensureInitialized(int timeout_ms = 5000);

    bool initializeFromIDLText(const QString &idlText);

    /*!
     * @brief Write a single sample using QVariantMap data
     * @param sampleData Map of field names to values
     * @return true if published successfully, false otherwise
     */
    bool writeSample(const QVariantMap& sampleData);

private:
    eprosima::fastdds::dds::DomainParticipant* mp_participant;
    eprosima::fastdds::dds::Publisher* mp_publisher;
    int m_domainId = 0;

    /*!
     * @brief Set a field value in DynamicData from QVariant
     * @param data The DynamicData instance
     * @param fieldName The field name
     * @param value The value to set
     * @return true if successful
     */
    bool setDynamicDataField(
        eprosima::fastrtps::types::DynamicData* data,
        const std::string& fieldName,
        const QVariant& value
    );
    
    // Fixed PartListener: now implements on_type_information_received so the
    // TypeLookup Service can populate received_type_ and unblock ensureInitialized()
    class PartListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:
        explicit PartListener(HelloWorldPublisher *pub) : publisher_(pub) {}
        ~PartListener() override = default;

        void on_type_information_received(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::string_255 topic_name,
            const eprosima::fastrtps::string_255 type_name,
            const eprosima::fastrtps::types::TypeInformation& type_information) override;

    private:
        HelloWorldPublisher *publisher_;
    } m_participant_listener;

    // DataWriter listener (DataWriterListener)
    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:
        explicit PubListener(HelloWorldPublisher *pub);
        ~PubListener() override;

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        int n_matched;
        uint32_t n_samples;
        eprosima::fastrtps::types::DynamicType_ptr received_type_;
        std::atomic<bool> reception_flag_{false};
        std::condition_variable types_cv_;
        std::mutex types_mx_;

    private:
        HelloWorldPublisher *publisher_;
    } m_listener;

    eprosima::fastdds::dds::DataWriterQos qos_;
    eprosima::fastdds::dds::TypeSupport m_type_support_;
    bool m_type_registered_ = false;
    
    std::map<eprosima::fastdds::dds::DataWriter *, eprosima::fastdds::dds::Topic *> topics_;
    std::map<eprosima::fastdds::dds::DataWriter *, eprosima::fastrtps::types::DynamicType_ptr> writers_;
    std::map<eprosima::fastdds::dds::DataWriter *, eprosima::fastrtps::types::DynamicData_ptr> datas_;

    TopicIDLStruct *topicIDLModel;

    // Per-instance topic name — replaces global_publisher_topic_name in all
    // internal methods so multiple concurrent publishers never interfere.
    std::string m_topicNameStr;

    // Tracks the Topic* created inside initialize_entities() so that:
    // (a) on retry the same Topic object is reused instead of creating a duplicate
    // (b) the destructor can clean up if DataWriter creation failed and the topic
    //     was never stored in the topics_ map.
    eprosima::fastdds::dds::Topic* m_current_topic = nullptr;

    void initialize_entities();
    void publish_sample(const eprosima::fastrtps::types::DynamicData_ptr &data);
    void print_type_structure(const eprosima::fastrtps::types::DynamicType_ptr &type, int indent = 0);
    std::string get_type_name(eprosima::fastrtps::types::TypeKind kind);
    eprosima::fastrtps::types::DynamicType_ptr buildTypeFromIDL(const QString &idlText);
};

#endif // HELLOWORLDPUBLISHER_H
