#include <indedds-monitor/publisher/HelloWorldPublisher.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <QDebug>
#include <QString>
#include <QRegularExpression>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>

using namespace eprosima::fastdds::dds;
using eprosima::fastrtps::types::ReturnCode_t;

// Define the global variable here
std::string global_publisher_topic_name;

namespace
{

class SafeDynamicPubSubType : public eprosima::fastrtps::types::DynamicPubSubType
{
public:
    explicit SafeDynamicPubSubType(
            const eprosima::fastrtps::types::DynamicType_ptr& pType)
        : eprosima::fastrtps::types::DynamicPubSubType(pType)
    {
        // DynamicType size estimation can be too small for some discovered
        // dynamic types (especially with strings), causing DataWriter::write()
        // to fail with RETCODE_ERROR at runtime. Force a safe lower bound.
        if (m_typeSize < 65536)
        {
            m_typeSize = 65536;
        }
    }
};

} // namespace

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , m_participant_listener(this)  // ADD THIS LINE - Initialize participant listener
    , m_listener(this)
{
}

bool HelloWorldPublisher::init(const std::string &topic_name, int domain_id, TopicIDLStruct *topicIDLModel)
{
    std::cout << "[DEBUG] Entering HelloWorldPublisher::init()" << std::endl;
    std::cout << "[DEBUG] Input topic_name = " << topic_name << std::endl;
    std::cout << "[DEBUG] topicIDLModel pointer = " << topicIDLModel << std::endl;
    std::cout << "[DEBUG] domain_id = " << domain_id << std::endl;
    m_domainId = domain_id;
    m_topicNameStr = topic_name;
    global_publisher_topic_name = topic_name;
    this->topicIDLModel = topicIDLModel;
    topicIDLModel->setTopicName(QString::fromStdString(topic_name));
    std::cout << "[DEBUG] Topic name set in model = " << topicIDLModel->topicName().toStdString() << std::endl;

    // Access the model values using getter methods for QoS
    QString reliability = topicIDLModel->reliability();
    QString durability = topicIDLModel->durability();
    QString ownership = topicIDLModel->ownership();
    std::cout << "[DEBUG] Retrieved model QoS values -> Reliability: " << reliability.toStdString()
              << ", Durability: " << durability.toStdString()
              << ", Ownership: " << ownership.toStdString() << std::endl;

    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = true;
    std::cout << "[DEBUG] Setting factory_qos.entity_factory().autoenable_created_entities = "
              << factory_qos.entity_factory().autoenable_created_entities << std::endl;

    DomainParticipantFactory::get_instance()->set_qos(factory_qos);
    std::cout << "[DEBUG] DomainParticipantFactory QoS set successfully" << std::endl;

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    std::cout << "[DEBUG] Participant name set = Participant_pub" << std::endl;

    // Keep publisher participant TypeLookup disabled.
    // Runtime publish flow resolves type via the temporary discovery subscriber
    // and then initializes publisher entities from that discovered DynamicType
    // (or parsed IDL fallback). Enabling TypeLookup client here can race with
    // that flow and trigger register_remote_type() for the same type name in
    // this participant, causing duplicate/competing type registration states.
    pqos.wire_protocol().builtin.typelookup_config.use_client = false;
    pqos.wire_protocol().builtin.typelookup_config.use_server = false;
    std::cout << "[DEBUG] TypeLookup client disabled for publisher participant" << std::endl;

    if (durability == "Transient")
    {
        std::cout << "[DEBUG] Durability is Transient -> Adding persistence plugin property" << std::endl;
        pqos.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    }

    // Publication matching happens on DataWriter listener.
    // No participant-level callbacks are required for publisher init path.
    StatusMask par_mask = StatusMask::none();
    std::cout << "[DEBUG] Creating DomainParticipant on domain " << domain_id
              << " with StatusMask::none()" << std::endl;

    // Keep participant listener detached in init to avoid participant-level
    // type registration side effects; type bootstrap is managed explicitly by
    // Controller (discovered DynamicType / IDL fallback).
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(
        domain_id, pqos, nullptr, par_mask);

    if (mp_participant == nullptr)
    {
        std::cout << "[ERROR] Failed to create DomainParticipant" << std::endl;
        return false;
    }
    else
    {
        std::cout << "[DEBUG] DomainParticipant created successfully at " << mp_participant << std::endl;
    }

    if (mp_participant->enable() != ReturnCode_t::RETCODE_OK)
    {
        std::cout << "[ERROR] Failed to enable DomainParticipant, deleting..." << std::endl;
        DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
        return false;
    }
    else
    {
        std::cout << "[DEBUG] DomainParticipant enabled successfully" << std::endl;
    }

    std::cout << "Reliability from publisher local variable : " << reliability.toStdString() << std::endl;
    std::cout << "Durability from publisher local variable : " << durability.toStdString() << std::endl;
    std::cout << "Ownership from publisher local variable : " << ownership.toStdString() << std::endl;

    bool isQoSSet = !topicIDLModel->reliability().isEmpty() ||
                    !topicIDLModel->durability().isEmpty() ||
                    !topicIDLModel->ownership().isEmpty();

    std::cout << "[DEBUG] isQoSSet = " << (isQoSSet ? "true" : "false") << std::endl;

    if (!isQoSSet)
    {
        // Use broadly-compatible defaults so dynamic subscribers with default
        // settings can always match this writer.
        qos_ = DATAWRITER_QOS_DEFAULT;
        // Reliable writer is compatible with both reliable and best-effort readers,
        // while best-effort writer is NOT compatible with reliable readers.
        qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
        qos_.durability().kind = VOLATILE_DURABILITY_QOS;
        qos_.ownership().kind = SHARED_OWNERSHIP_QOS;
        std::cout << "Using compatible default QoS settings: "
                  << "RELIABLE + VOLATILE + SHARED" << std::endl;
    }
    else
    {
        std::cout << "[DEBUG] Custom QoS settings will be applied" << std::endl;

        // Set QoS based on model properties
        if (!reliability.isEmpty())
        {
            if (reliability == "Reliable")
            {
                qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
                std::cout << "[DEBUG] QoS Reliability set to RELIABLE" << std::endl;
            }
            else
            {
                qos_.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
                std::cout << "[DEBUG] QoS Reliability set to BEST_EFFORT" << std::endl;
            }
        }

        if (!durability.isEmpty())
        {
            if (durability == "Volatile")
            {
                qos_.durability().kind = VOLATILE_DURABILITY_QOS;
                std::cout << "[DEBUG] QoS Durability set to VOLATILE" << std::endl;
            }
            else if (durability == "Transient")
            {
                qos_.durability().kind = TRANSIENT_DURABILITY_QOS;
                qos_.properties().properties().emplace_back("dds.persistence.guid", "78.72.64.74.25.72.5f.70.65.72.79.5f|67.75.69.64");
                std::cout << "[DEBUG] QoS Durability set to TRANSIENT + persistence.guid property applied" << std::endl;
            }
            else if (durability == "Transient Local")
            {
                qos_.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
                std::cout << "[DEBUG] QoS Durability set to TRANSIENT_LOCAL" << std::endl;
            }
            else if (durability == "Persistent")
            {
                qos_.durability().kind = PERSISTENT_DURABILITY_QOS;
                std::cout << "[DEBUG] QoS Durability set to PERSISTENT" << std::endl;
            }
        }

        if (!ownership.isEmpty())
        {
            qos_.ownership().kind = (ownership == "Exclusive")
                                        ? EXCLUSIVE_OWNERSHIP_QOS
                                        : SHARED_OWNERSHIP_QOS;

            std::cout << "[DEBUG] QoS Ownership set to "
                      << ((ownership == "Exclusive") ? "EXCLUSIVE" : "SHARED") << std::endl;
        }
    }

    std::cout << "[DEBUG] Exiting HelloWorldPublisher::init() with return true" << std::endl;
    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    std::cout << "[DEBUG] Entering ~HelloWorldPublisher destructor" << std::endl;

    std::cout << "[DEBUG] Cleaning up topics, total topics_: " << topics_.size() << std::endl;
    for (const auto &it : topics_)
    {
        std::cout << "[DEBUG] Deleting DataWriter for topic: " << it.second->get_name() << std::endl;
        if (mp_publisher != nullptr)
        {
            mp_publisher->delete_datawriter(it.first);
        }

        std::cout << "[DEBUG] Deleting Topic: " << it.second->get_name() << std::endl;
        if (mp_participant != nullptr)
        {
            mp_participant->delete_topic(it.second);
        }
    }

    if (mp_publisher != nullptr)
    {
        std::cout << "[DEBUG] Deleting Publisher" << std::endl;
        if (mp_participant != nullptr)
        {
            mp_participant->delete_publisher(mp_publisher);
        }
    }
    else
    {
        std::cout << "[DEBUG] mp_publisher is nullptr, skipping delete" << std::endl;
    }

    std::cout << "[DEBUG] Deleting Participant" << std::endl;
    if (mp_participant != nullptr)
    {
        if (m_current_topic != nullptr && topics_.empty())
        {
            std::cout << "[DEBUG] Deleting leaked Topic (DataWriter was never created)" << std::endl;
            mp_participant->delete_topic(m_current_topic);
            m_current_topic = nullptr;
        }
        DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
        mp_participant = nullptr;
    }

    std::cout << "[DEBUG] Clearing containers: topics_, writers_, datas_" << std::endl;
    topics_.clear();
    writers_.clear();
    datas_.clear();
    m_type_support_.reset();
    m_type_registered_ = false;

    std::cout << "[DEBUG] Exiting ~HelloWorldPublisher destructor" << std::endl;
}

HelloWorldPublisher::PubListener::PubListener(HelloWorldPublisher *pub)
    : n_matched(0), n_samples(0), publisher_(pub)
{
    std::cout << "[DEBUG] PubListener constructed, n_matched = " << n_matched
              << ", n_samples = " << n_samples
              << ", publisher_ = " << (publisher_ ? "valid" : "nullptr") << std::endl;
}

HelloWorldPublisher::PubListener::~PubListener()
{
    std::cout << "[DEBUG] PubListener destructor called" << std::endl;
}

void HelloWorldPublisher::PubListener::on_publication_matched(
    DataWriter *writer,
    const PublicationMatchedStatus &info)
{
    std::cout << "[DEBUG] on_publication_matched triggered" << std::endl;
    std::cout << "[DEBUG] DataWriter pointer: " << writer << std::endl;
    std::cout << "[DEBUG] PublicationMatchedStatus: "
              << "current_count_change=" << info.current_count_change
              << ", total_count=" << info.total_count
              << ", total_count_change=" << info.total_count_change << std::endl;

    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "[DEBUG] Publisher matched, n_matched updated to " << n_matched << std::endl;
        std::cout << "Publisher matched with " << n_matched << " subscriber(s)" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "[DEBUG] Publisher unmatched, n_matched updated to " << n_matched << std::endl;
        std::cout << "Publisher unmatched. Current matched subscribers: " << n_matched << std::endl;
    }
    else
    {
        std::cout << "[DEBUG] Publisher match status changed." << std::endl;
    }
}

void HelloWorldPublisher::initialize_entities()
{
    std::lock_guard<std::mutex> entities_lock(m_entities_mutex_);
    auto type = m_listener.received_type_;
    std::cout << "[DEBUG] Entering initialize_entities()" << std::endl;

    if (!type)
    {
        std::cout << "[ERROR] No type received in listener. Cannot initialize entities." << std::endl;
        return;
    }

    std::cout << "[DEBUG] Initializing DDS entities for type: " << type->get_name() << std::endl;

    if (!m_type_support_)
    {
        m_type_support_ = TypeSupport(new SafeDynamicPubSubType(type));
    }

    if (!m_type_support_)
    {
        std::cout << "[ERROR] Failed to create DynamicPubSubType for type: " << type->get_name() << std::endl;
        return;
    }

    if (!m_type_registered_)
    {
        std::cout << "[DEBUG] Registering type with participant..." << std::endl;
        auto reg_ret = m_type_support_.register_type(mp_participant);
        if (reg_ret != ReturnCode_t::RETCODE_OK &&
                reg_ret != ReturnCode_t::RETCODE_PRECONDITION_NOT_MET)
        {
            std::cout << "[ERROR] Failed to register type, code: " << static_cast<int>(reg_ret()) << std::endl;
            return;
        }
        if (reg_ret == ReturnCode_t::RETCODE_PRECONDITION_NOT_MET)
        {
            std::cout << "[DEBUG] Type name already registered in participant. "
                         "Reusing participant registration for topic creation." << std::endl;
        }
        m_type_registered_ = true;
        std::cout << "[DEBUG] Type registered (or already registered)." << std::endl;
    }

    if (mp_publisher == nullptr)
    {
        std::cout << "[DEBUG] Creating publisher..." << std::endl;
        mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        if (mp_publisher == nullptr)
        {
            std::cout << "[ERROR] Failed to create Publisher" << std::endl;
            return;
        }
        else
        {
            std::cout << "[DEBUG] Publisher created successfully" << std::endl;
        }
    }
    else
    {
        std::cout << "[DEBUG] Publisher already exists, skipping creation." << std::endl;
    }

    if (m_current_topic == nullptr)
    {
        std::cout << "[DEBUG] Creating topic with name: " << m_topicNameStr
                  << " and type: " << m_type_support_.get_type_name() << std::endl;

        m_current_topic = mp_participant->create_topic(
            m_topicNameStr,
            m_type_support_.get_type_name(),
            TOPIC_QOS_DEFAULT);

        if (m_current_topic == nullptr)
        {
            std::cout << "[ERROR] Failed to create Topic: " << m_topicNameStr << std::endl;
            return;
        }
        std::cout << "[DEBUG] Topic created successfully: " << m_topicNameStr << std::endl;
    }
    else
    {
        std::cout << "[DEBUG] Reusing existing Topic: " << m_topicNameStr << std::endl;
    }

    Topic* topic = m_current_topic;

    // QoS debug information
    switch (qos_.durability().kind)
    {
    case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to VOLATILE_DURABILITY_QOS" << std::endl;
        break;
    case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to TRANSIENT_LOCAL_DURABILITY_QOS" << std::endl;
        break;
    case eprosima::fastrtps::TRANSIENT_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to TRANSIENT_DURABILITY_QOS" << std::endl;
        break;
    case eprosima::fastrtps::PERSISTENT_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to PERSISTENT_DURABILITY_QOS" << std::endl;
        break;
    default:
        std::cout << "[WARN] Unknown durability QoS setting." << std::endl;
        break;
    }

    switch (qos_.reliability().kind)
    {
    case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
        std::cout << "[DEBUG] Reliability set to RELIABLE_RELIABILITY_QOS" << std::endl;
        break;
    case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
        std::cout << "[DEBUG] Reliability set to BEST_EFFORT_RELIABILITY_QOS" << std::endl;
        break;
    default:
        std::cout << "[WARN] Unknown reliability QoS setting." << std::endl;
        break;
    }

    std::cout << "[DEBUG] Creating DataWriter..." << std::endl;
    StatusMask pub_mask = StatusMask::publication_matched();
    DataWriter *writer = mp_publisher->create_datawriter(
        topic,
        qos_,
        &m_listener,
        pub_mask);

    if (!writer)
    {
        std::cout << "[ERROR] Failed to create DataWriter for topic: " << m_topicNameStr << std::endl;
        return;
    }
    else
    {
        std::cout << "[DEBUG] DataWriter created successfully for topic: " << m_topicNameStr << std::endl;
    }

    topics_[writer] = topic;
    writers_[writer] = type;

    std::cout << "[DEBUG] Creating DynamicData for type: " << type->get_name() << std::endl;
    eprosima::fastrtps::types::DynamicData_ptr data(
        eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(type));

    if (!data)
    {
        std::cout << "[ERROR] Failed to create DynamicData instance for type: " << type->get_name() << std::endl;
        return;
    }

    datas_[writer] = data;
    std::cout << "[DEBUG] DynamicData created and stored successfully." << std::endl;

    std::cout << "[DEBUG] Exiting initialize_entities()" << std::endl;
}

void HelloWorldPublisher::publish_sample(const eprosima::fastrtps::types::DynamicData_ptr &data)
{
    std::cout << "[DEBUG] Entering publish_sample()" << std::endl;

    for (auto &it : datas_)
    {
        DataWriter *writer = it.first;
        std::cout << "[DEBUG] Publishing sample using DataWriter at " << writer << std::endl;

        if (writer->write(data.get()) == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[DEBUG] Sample published successfully" << std::endl;
            std::cout << "Sample published" << std::endl;
        }
        else
        {
            std::cout << "[ERROR] Failed to publish sample" << std::endl;
        }
    }

    std::cout << "[DEBUG] Exiting publish_sample()" << std::endl;
}

void HelloWorldPublisher::print_type_structure(const eprosima::fastrtps::types::DynamicType_ptr &type, int indent)
{
    std::cout << "[DEBUG] Entering print_type_structure() with type: " << type->get_name()
              << ", indent: " << indent << std::endl;

    static std::stringstream ss;
    if (indent == 0)
    {
        std::cout << "[DEBUG] Top-level call detected, clearing static stringstream" << std::endl;
        ss.str("");
        ss.clear();
    }

    std::string indent_str(indent * 2, ' ');
    ss << indent_str << "struct " << type->get_name() << " {" << std::endl;

    std::cout << "[DEBUG] Fetching all members of type: " << type->get_name() << std::endl;
    std::map<eprosima::fastrtps::types::MemberId, eprosima::fastrtps::types::DynamicTypeMember *> members_map;
    type->get_all_members(members_map);

    std::cout << "[DEBUG] Found " << members_map.size() << " members in type: " << type->get_name() << std::endl;
    std::vector<std::pair<eprosima::fastrtps::types::MemberId, eprosima::fastrtps::types::DynamicTypeMember *>> members_vec(members_map.begin(), members_map.end());

    std::sort(members_vec.begin(), members_vec.end(), [](const auto &a, const auto &b)
              { return a.first < b.first; });

    for (const auto &pair : members_vec)
    {
        const auto &member = pair.second;
        std::cout << "[DEBUG] Processing member: " << member->get_name() << std::endl;

        ss << indent_str << "  "
           << get_type_name(member->get_descriptor()->get_kind())
           << " " << member->get_name();

        if (member->get_descriptor()->get_kind() == eprosima::fastrtps::types::TK_STRUCTURE)
        {
            ss << " {}" << std::endl;
        }
        else
        {
            ss << ";" << std::endl;
        }
    }

    ss << indent_str << "};" << std::endl;

    if (indent == 0)
    {
        std::cout << "[DEBUG] Top-level completed, setting TopicIDLStruct with accumulated structure" << std::endl;
        QString newData = QString::fromStdString(ss.str());
        topicIDLModel->setTextData(newData);

        std::string data = std::string(topicIDLModel->textData().toStdString());
        std::cout << "[DEBUG] Final generated structure:\n" << data << std::endl;

        std::cout << "[DEBUG] Exiting print_type_structure()" << std::endl;
    }
}

std::string HelloWorldPublisher::get_type_name(eprosima::fastrtps::types::TypeKind kind)
{
    std::cout << "[DEBUG] Entering get_type_name() with TypeKind value: " << kind << std::endl;

    switch (kind)
    {
    case eprosima::fastrtps::types::TK_BOOLEAN:
        return "boolean";
    case eprosima::fastrtps::types::TK_BYTE:
        return "octet";
    case eprosima::fastrtps::types::TK_INT16:
        return "short";
    case eprosima::fastrtps::types::TK_INT32:
        return "long";
    case eprosima::fastrtps::types::TK_INT64:
        return "long long";
    case eprosima::fastrtps::types::TK_UINT16:
        return "unsigned short";
    case eprosima::fastrtps::types::TK_UINT32:
        return "unsigned long";
    case eprosima::fastrtps::types::TK_UINT64:
        return "unsigned long long";
    case eprosima::fastrtps::types::TK_FLOAT32:
        return "float";
    case eprosima::fastrtps::types::TK_FLOAT64:
        return "double";
    case eprosima::fastrtps::types::TK_STRING8:
        return "string";
    case eprosima::fastrtps::types::TK_STRING16:
        return "wstring";
    case eprosima::fastrtps::types::TK_SEQUENCE:
        return "sequence";
    case eprosima::fastrtps::types::TK_ARRAY:
        return "array";
    case eprosima::fastrtps::types::TK_STRUCTURE:
        return "struct";
    default:
        return "unknown";
    }
}

void HelloWorldPublisher::run()
{
    std::cout << "[DEBUG] Entering run()" << std::endl;

    std::cout << "Publisher running. Please press enter to stop the Publisher" << std::endl;

    std::cout << "[DEBUG] Acquiring lock on types_mx_" << std::endl;
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    std::cout << "[DEBUG] Waiting on types_cv_ condition variable..." << std::endl;

    m_listener.types_cv_.wait(lock, [&]()
                              {
                                  std::cout << "[DEBUG] Condition variable triggered, checking reception_flag_" << std::endl;
                                  bool result = m_listener.reception_flag_.exchange(false);
                                  std::cout << "[DEBUG] reception_flag_ value after exchange = " << result << std::endl;
                                  return result;
                              });

    std::cout << "[DEBUG] Condition satisfied, proceeding to initialize_entities()" << std::endl;

    initialize_entities();
    std::cout << "[DEBUG] initialize_entities() completed" << std::endl;

    std::cout << "[DEBUG] Waiting for user input to stop publisher" << std::endl;
    std::cin.ignore();

    std::cout << "[DEBUG] Exiting run()" << std::endl;
}

// NEW METHOD: Non-blocking run with callback
void HelloWorldPublisher::runWithCallback(std::function<void(bool)> callback)
{
    std::cout << "[DEBUG] Entering runWithCallback()" << std::endl;
    std::cout << "Publisher starting discovery process..." << std::endl;

    // Start a thread that waits for type discovery
    std::thread discovery_thread([this, callback]() {
        std::cout << "[DEBUG] Discovery thread started" << std::endl;
        
        std::unique_lock<std::mutex> lock(m_listener.types_mx_);
        std::cout << "[DEBUG] Waiting on types_cv_ with 5 second timeout..." << std::endl;
        
        // Wait with timeout (5 seconds)
        bool discovered = m_listener.types_cv_.wait_for(
            lock,
            std::chrono::seconds(5),
            [&]() {
                bool result = m_listener.reception_flag_.exchange(false);
                std::cout << "[DEBUG] Condition check: reception_flag_ = " << result << std::endl;
                return result;
            }
        );
        
        if (discovered) {
            std::cout << "[DEBUG] Type discovered successfully!" << std::endl;
            initialize_entities();
            std::cout << "[DEBUG] Entities initialized, notifying callback with success" << std::endl;
            callback(true);  // Success
        } else {
            std::cout << "[ERROR] Type discovery TIMEOUT after 5 seconds" << std::endl;
            callback(false);  // Failure
        }
        
        std::cout << "[DEBUG] Discovery thread exiting" << std::endl;
    });
    
    discovery_thread.detach();  // Let it run independently
    std::cout << "[DEBUG] Exiting runWithCallback(), discovery thread detached" << std::endl;
}


void HelloWorldPublisher::run(uint32_t number)
{
    std::cout << "[DEBUG] Entering run(number)" << std::endl;
    std::cout << "Publisher running, will send " << number << " samples" << std::endl;

    std::cout << "[DEBUG] Acquiring lock on types_mx_" << std::endl;
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    std::cout << "[DEBUG] Waiting on types_cv_ condition variable..." << std::endl;

    m_listener.types_cv_.wait(lock, [&]()
                              {
                                  std::cout << "[DEBUG] Condition variable triggered, checking reception_flag_" << std::endl;
                                  bool result = m_listener.reception_flag_.exchange(false);
                                  std::cout << "[DEBUG] reception_flag_ value after exchange = " << result << std::endl;
                                  return result;
                              });

    std::cout << "[DEBUG] Condition satisfied, proceeding to initialize_entities()" << std::endl;

    initialize_entities();
    std::cout << "[DEBUG] initialize_entities() completed" << std::endl;

    std::cout << "[DEBUG] Entering sample publishing loop for " << number << " samples" << std::endl;
    for (uint32_t i = 0; i < number; ++i)
    {
        std::cout << "[DEBUG] Publishing sample " << (i + 1) << " of " << number << std::endl;
        
        for (auto &it : datas_)
        {
            publish_sample(it.second);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "[DEBUG] All samples published, exiting run(number)" << std::endl;
}

// ========================================
// Ensure entities are initialized
// ========================================
bool HelloWorldPublisher::ensureInitialized(int timeout_ms)
{

    // Fast path: if entities are already created (e.g. previous call succeeded,
    // or initializeFromIDLText already ran), return immediately without locking.
    if (isReady())
    {
        std::cout << "[ensureInitialized] Already initialized, returning immediately" << std::endl;
        return true;
    }
    std::cout << "[HelloWorldPublisher::ensureInitialized] Waiting for type discovery..." << std::endl;
    
    // Wait for type discovery with timeout
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);  //  FIXED: types_mx_
    bool discovered = m_listener.types_cv_.wait_for(          //  FIXED: types_cv_
        lock,
        std::chrono::milliseconds(timeout_ms),
        [this]() {
            bool result = m_listener.reception_flag_.exchange(false);  //  FIXED: reception_flag_
            std::cout << "[HelloWorldPublisher] Checking reception_flag_: " << result << std::endl;
            return result;
        }
    );

    if (!discovered)
    {
        std::cerr << "[HelloWorldPublisher::ensureInitialized] ✗ Type discovery timeout!" << std::endl;
        // Race-safe fallback: callback may have arrived right after wait_for timeout.
        if (m_listener.received_type_)
        {
            std::cout << "[HelloWorldPublisher::ensureInitialized] "
                         "Late type discovered after timeout window, proceeding to initialize entities."
                      << std::endl;
        }
        else
        {
            return false;
        }
    }

    std::cout << "[HelloWorldPublisher::ensureInitialized] ✓ Type discovered, initializing entities..." << std::endl;

    // Initialize DDS entities
    initialize_entities();  //  FIXED: initialize_entities (with underscore)

    // Check if DataWriter was created
    if (datas_.empty() || !mp_publisher)  //  FIXED: datas_ and mp_publisher
    {
        std::cerr << "[HelloWorldPublisher::ensureInitialized] ✗ Entities not created!" << std::endl;
        return false;
    }

    std::cout << "[HelloWorldPublisher::ensureInitialized] ✓ ✓ ✓ All entities ready!" << std::endl;
    return true;
}


// ========================================
// Write a single sample from QVariantMap
// ========================================
bool HelloWorldPublisher::writeSample(const QVariantMap& sampleData)
{
    std::lock_guard<std::mutex> entities_lock(m_entities_mutex_);
    std::cout << "========================================" << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] ========================================" << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] writeSample() CALLED" << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] Sample has " << sampleData.size() << " fields" << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] Fields:" << std::endl;
    for (auto it = sampleData.constBegin(); it != sampleData.constEnd(); ++it) {
        std::cout << "[HelloWorldPublisher::writeSample]   - " << it.key().toStdString() << ", ";
        std::cout << "[HelloWorldPublisher::writeSample]   - " << it.value().toString().toStdString() << std::endl;
    }

    // Check if entities are initialized
    if (datas_.empty())  //  FIXED: datas_
    {
        std::cerr << "[HelloWorldPublisher::writeSample] ✗ CRITICAL: No DynamicData instances! Call ensureInitialized first." << std::endl;
        return false;
    }

    std::cout << "[HelloWorldPublisher::writeSample] ✓ DynamicData instances found:" << datas_.size() << std::endl;

    // Get the first DataWriter and its DynamicData
    auto it = datas_.begin();  //  FIXED: datas_
    eprosima::fastrtps::types::DynamicData* dynamicData = it->second.get();
    eprosima::fastdds::dds::DataWriter* writer = it->first;

    if (!dynamicData || !writer)
    {
        std::cerr << "[HelloWorldPublisher::writeSample] ✗ CRITICAL: NULL DataWriter or DynamicData!" << std::endl;
        if (!writer) std::cerr << "[HelloWorldPublisher::writeSample] DataWriter is NULL" << std::endl;
        if (!dynamicData) std::cerr << "[HelloWorldPublisher::writeSample] DynamicData is NULL" << std::endl;
        return false;
    }

    std::cout << "[HelloWorldPublisher::writeSample] ✓ Valid DataWriter at " << writer << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] ✓ Valid DynamicData at " << dynamicData << std::endl;

    eprosima::fastdds::dds::PublicationMatchedStatus match_status;
    if (writer->get_publication_matched_status(match_status) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << "[HelloWorldPublisher::writeSample] Matched readers current_count="
                  << match_status.current_count << " total_count=" << match_status.total_count << std::endl;
    }

    // Set each field from the QVariantMap
    bool allFieldsSet = true;
    int successCount = 0;
    for (auto fieldIt = sampleData.constBegin(); fieldIt != sampleData.constEnd(); ++fieldIt)
    {
        QString fieldName = fieldIt.key();
        QVariant value = fieldIt.value();
        
        std::cout << "[HelloWorldPublisher::writeSample] Setting field: "
                  << fieldName.toStdString() << " = ";
        
        // Print value based on type
        if (value.type() == QVariant::String)
            std::cout << "\"" << value.toString().toStdString() << "\" (string)";
        else if (value.type() == QVariant::Int || value.type() == QVariant::LongLong)
            std::cout << value.toLongLong() << " (int)";
        else if (value.type() == QVariant::Double)
            std::cout << value.toDouble() << " (double)";
        else if (value.type() == QVariant::Bool)
            std::cout << (value.toBool() ? "true" : "false") << " (bool)";
        else
            std::cout << "(unknown type: " << value.type() << ")";
        
        std::cout << std::endl;

        // Set the field
        bool success = setDynamicDataField(dynamicData, fieldName.toStdString(), value);
        if (!success)
        {
            std::cerr << "[HelloWorldPublisher::writeSample] ✗ FAILED to set field: "
                      << fieldName.toStdString() << std::endl;
            allFieldsSet = false;
        } else {
            std::cout << "[HelloWorldPublisher::writeSample] ✓ Field set successfully" << std::endl;
            successCount++;
        }
    }

    std::cout << "[HelloWorldPublisher::writeSample] Summary: " << successCount << "/" << sampleData.size() << " fields set" << std::endl;

    if (!allFieldsSet)
    {
        std::cerr << "[HelloWorldPublisher::writeSample] ⚠ WARNING: Some fields could not be set!" << std::endl;
    }

    // Write the sample to DDS
    std::cout << "[HelloWorldPublisher::writeSample] ========================================" << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] Writing sample to DDS..." << std::endl;
    std::cout << "[HelloWorldPublisher::writeSample] Topic name: " << global_publisher_topic_name << std::endl;
    
    eprosima::fastrtps::types::ReturnCode_t ret = writer->write(dynamicData);

    if (ret == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
    {
        std::cout << "[HelloWorldPublisher::writeSample] ✓ ✓ ✓ SUCCESS: SAMPLE PUBLISHED TO DDS!" << std::endl;
        std::cout << "[HelloWorldPublisher::writeSample] ========================================" << std::endl;
        std::cout << "========================================" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "[HelloWorldPublisher::writeSample] ✗ ✗ ✗ CRITICAL: DDS write() FAILED!" << std::endl;
        int rc = static_cast<int>(ret());
        std::cerr << "[HelloWorldPublisher::writeSample] Write operation returned code: "
                  << rc << std::endl;
        switch (rc)
        {
            case 1: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_ERROR (generic serialization/type/write failure)" << std::endl; break;
            case 4: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_PRECONDITION_NOT_MET" << std::endl; break;
            case 5: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_OUT_OF_RESOURCES" << std::endl; break;
            case 6: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_NOT_ENABLED" << std::endl; break;
            case 7: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_IMMUTABLE_POLICY" << std::endl; break;
            case 8: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_INCONSISTENT_POLICY" << std::endl; break;
            case 9: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_ALREADY_DELETED" << std::endl; break;
            case 10: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_TIMEOUT" << std::endl; break;
            case 11: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_NO_DATA" << std::endl; break;
            case 12: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_ILLEGAL_OPERATION" << std::endl; break;
            case 13: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl; break;
            case 14: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_BAD_PARAMETER" << std::endl; break;
            case 15: std::cerr << "[HelloWorldPublisher::writeSample] RETCODE_UNSUPPORTED" << std::endl; break;
            default: break;
        }

        std::cout << "[HelloWorldPublisher::writeSample] ========================================" << std::endl;
        std::cout << "========================================" << std::endl;
        return false;
    }
}


// ========================================
// Helper: Set a single field in DynamicData
// ========================================
bool HelloWorldPublisher::setDynamicDataField(
    eprosima::fastrtps::types::DynamicData* data,
    const std::string& fieldName,
    const QVariant& value)
{
    using namespace eprosima::fastrtps::types;

    std::cout << "[setDynamicDataField] ========================================" << std::endl;
    std::cout << "[setDynamicDataField] Setting field: " << fieldName << std::endl;
    std::cout << "[setDynamicDataField] QVariant type: " << value.typeName() << std::endl;

    // Get member ID by name
    MemberId memberId = data->get_member_id_by_name(fieldName);
    if (memberId == MEMBER_ID_INVALID)
    {
        std::cerr << "[setDynamicDataField] ✗ Field not found: " << fieldName << std::endl;
        return false;
    }
    std::cout << "[setDynamicDataField] ✓ Field found, member ID:" << memberId << std::endl;

    ReturnCode_t ret = ReturnCode_t::RETCODE_ERROR;

    // ── String path ────────────────────────────────────────────────────────
    // Strings and chars are never numeric — handle first and return directly.
    if (value.type() == QVariant::String)
    {
        std::string strVal = value.toString().toStdString();
        std::cout << "[setDynamicDataField] Trying string: \"" << strVal << "\"" << std::endl;
        ret = data->set_string_value(strVal, memberId);
        if (ret != ReturnCode_t::RETCODE_OK)
        {
            // Attempt wstring as fallback for string fields
            std::cout << "[setDynamicDataField] string failed, trying wstring..." << std::endl;
            ret = data->set_wstring_value(std::wstring(strVal.begin(), strVal.end()), memberId);
        }
        if (ret != ReturnCode_t::RETCODE_OK)
        {
            std::cerr << "[setDynamicDataField] ✗ FAILED: " << fieldName << std::endl;
            return false;
        }
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }

    if (value.type() == QVariant::Char)
    {
        ret = data->set_char8_value(value.toChar().toLatin1(), memberId);
        if (ret != ReturnCode_t::RETCODE_OK)
        {
            std::cerr << "[setDynamicDataField] ✗ FAILED char: " << fieldName << std::endl;
            return false;
        }
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }

    if (value.type() == QVariant::Bool)
    {
        ret = data->set_bool_value(value.toBool(), memberId);
        if (ret != ReturnCode_t::RETCODE_OK)
        {
            std::cerr << "[setDynamicDataField] ✗ FAILED bool: " << fieldName << std::endl;
            return false;
        }
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }

    // ── Universal numeric probe ────────────────────────────────────────────
    // The root cause of the original failure: setDynamicDataField only tried
    // integer DDS setters when QVariant was Int, but the actual DDS field
    // type may be float/double (e.g. temperature_value = 10 stored as float).
    //
    // Fix: for ANY numeric QVariant (Int, UInt, LongLong, ULongLong, Double)
    // probe ALL numeric DDS types in order until one succeeds.
    // The DDS API returns RETCODE_BAD_PARAMETER immediately on type mismatch
    // (no data is written until the correct setter is called), so this is safe
    // and correct for every possible IDL numeric type.
    //
    // Probe order (most common IDL types first):
    //   float32, float64          ← temperature / sensor data
    //   int32, int64, int16       ← general signed integers
    //   uint32, uint64, uint16    ← general unsigned integers
    //   byte (uint8)              ← octet fields

    double numericVal = value.toDouble();   // lossless for all int/uint/double variants
    std::cout << "[setDynamicDataField] Numeric probe for field '" << fieldName
              << "' value=" << numericVal << " (QVariant:" << value.typeName() << ")" << std::endl;

    // float32
    ret = data->set_float32_value(static_cast<float>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as float32" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] float32 failed, trying float64..." << std::endl;

    // float64
    ret = data->set_float64_value(numericVal, memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as float64" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] float64 failed, trying int32..." << std::endl;

    // int32
    ret = data->set_int32_value(static_cast<int32_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as int32" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] int32 failed, trying int64..." << std::endl;

    // int64
    ret = data->set_int64_value(static_cast<int64_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as int64" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] int64 failed, trying int16..." << std::endl;

    // int16
    ret = data->set_int16_value(static_cast<int16_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as int16" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] int16 failed, trying uint32..." << std::endl;

    // uint32
    ret = data->set_uint32_value(static_cast<uint32_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as uint32" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] uint32 failed, trying uint64..." << std::endl;

    // uint64
    ret = data->set_uint64_value(static_cast<uint64_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as uint64" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] uint64 failed, trying uint16..." << std::endl;

    // uint16
    ret = data->set_uint16_value(static_cast<uint16_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as uint16" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }
    std::cout << "[setDynamicDataField] uint16 failed, trying byte (uint8)..." << std::endl;

    // byte (uint8)
    ret = data->set_byte_value(static_cast<uint8_t>(numericVal), memberId);
    if (ret == ReturnCode_t::RETCODE_OK) {
        std::cout << "[setDynamicDataField] ✓ Set as byte" << std::endl;
        std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
        return true;
    }

    {
        // Use integer form if value is whole (avoids "10.000000"), decimal otherwise
        std::string numAsStr;
        if (numericVal == static_cast<double>(static_cast<int64_t>(numericVal)))
            numAsStr = std::to_string(static_cast<int64_t>(numericVal));
        else
            numAsStr = std::to_string(numericVal);

        std::cout << "[setDynamicDataField] All numeric probes failed. "
                     "Trying string fallback: \"" << numAsStr << "\"" << std::endl;

        ret = data->set_string_value(numAsStr, memberId);
        if (ret == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[setDynamicDataField] ✓ Set as string (numeric→string fallback)" << std::endl;
            std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
            return true;
        }

        // wstring fallback
        std::cout << "[setDynamicDataField] string fallback failed, trying wstring..." << std::endl;
        ret = data->set_wstring_value(
            std::wstring(numAsStr.begin(), numAsStr.end()), memberId);
        if (ret == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[setDynamicDataField] ✓ Set as wstring (numeric→wstring fallback)" << std::endl;
            std::cout << "[setDynamicDataField] ✓ SUCCESS: " << fieldName << std::endl;
            return true;
        }
    }

    std::cerr << "[setDynamicDataField] ✗ FAILED: all DDS types exhausted for field '"
              << fieldName << "' value=" << numericVal << std::endl;
    std::cerr << "[setDynamicDataField]   Field is likely an enum, nested struct, or "
                 "sequence — compound types are not settable via this path." << std::endl;
    std::cout << "[setDynamicDataField] ========================================" << std::endl;
    return false;
}

// ============================================================
// PartListener::on_type_information_received
//
// This is the TYPE DISCOVERY entry point for the publisher.
// Fast DDS calls this on the participant's listener thread when
// another participant on the same domain announces a type via the
// builtin TypeLookup Service.  We request the full DynamicType_ptr
// via register_remote_type() and, once resolved, store it in
// m_listener so ensureInitialized()'s condition_variable unblocks.
// ============================================================
void HelloWorldPublisher::PartListener::on_type_information_received(
    eprosima::fastdds::dds::DomainParticipant *participant,
    const eprosima::fastrtps::string_255 topic_name,
    const eprosima::fastrtps::string_255 type_name,
    const eprosima::fastrtps::types::TypeInformation &type_information)
{
    using namespace eprosima::fastrtps::types;

    std::cout << "[PartListener::on_type_information_received] ==============================" << std::endl;
    std::cout << "[PartListener] topic_name  = " << topic_name.c_str() << std::endl;
    std::cout << "[PartListener] type_name   = " << type_name.c_str() << std::endl;
    std::cout << "[PartListener] our topic   = " << publisher_->m_topicNameStr << std::endl;

    // Filter: only process the topic this publisher was created for
    if (std::string(topic_name.c_str()) != publisher_->m_topicNameStr)
    {
        std::cout << "[PartListener] Not our topic — ignoring" << std::endl;
        return;
    }

    // Guard against re-entry if callback fires more than once for the same topic
    if (publisher_->isReady())
    {
        std::cout << "[PartListener] Publisher already initialized — ignoring duplicate callback" << std::endl;
        return;
    }

    std::cout << "[PartListener] Calling participant->register_remote_type() ..." << std::endl;

    std::function<void(const std::string&, eprosima::fastrtps::types::DynamicType_ptr)>
        type_callback = [this](const std::string& name,
                               const eprosima::fastrtps::types::DynamicType_ptr type)
    {
        std::cout << "[PartListener] register_remote_type callback: name=" << name << std::endl;

        if (!type)
        {
            std::cerr << "[PartListener] ✗ NULL DynamicType_ptr received — cannot initialize" << std::endl;
            return;
        }

        std::cout << "[PartListener] ✓ DynamicType resolved: " << type->get_name() << std::endl;

        // Store the resolved type and wake ensureInitialized()
        {
            std::lock_guard<std::mutex> lock(publisher_->m_listener.types_mx_);
            publisher_->m_listener.received_type_ = type;
            publisher_->m_listener.reception_flag_.store(true);
        }
        publisher_->m_listener.types_cv_.notify_all();

        std::cout << "[PartListener] ✓ Condition variable notified — ensureInitialized will proceed" << std::endl;
    };

    auto ret = participant->register_remote_type(
        type_information,
        std::string(type_name.c_str()),
        type_callback);

    if (ret != ReturnCode_t::RETCODE_OK)
    {
        std::cerr << "[PartListener] ✗ register_remote_type returned error code: "
                  << static_cast<int>(ret()) << std::endl;
    }

    std::cout << "[PartListener::on_type_information_received] ==============================" << std::endl;
}

// ============================================================
// buildTypeFromIDL — Fallback type builder
//
// Parses the IDL text that the subscriber-side discovery wrote
// into topicIDLModel->textData() and builds a DynamicType_ptr
// using DynamicTypeBuilderFactory.  Used when TypeLookup times
// out but IDL text is already available (e.g. no other publisher
// on the network, but a subscriber was previously discovered).
// ============================================================
eprosima::fastrtps::types::DynamicType_ptr
HelloWorldPublisher::buildTypeFromIDL(const QString &idlText)
{
    using namespace eprosima::fastrtps::types;

    std::cout << "[buildTypeFromIDL] Parsing IDL text (length=" << idlText.length() << ")" << std::endl;

    // ── 1. Extract struct name ──────────────────────────────────
    QRegularExpression nameRx(R"(struct\s+(\w+)\s*\{)");
    QRegularExpressionMatch nameMatch = nameRx.match(idlText);
    if (!nameMatch.hasMatch())
    {
        std::cerr << "[buildTypeFromIDL] ✗ No 'struct <Name> {' found in IDL" << std::endl;
        return DynamicType_ptr();
    }
    QString typeName = nameMatch.captured(1);
    std::cout << "[buildTypeFromIDL] struct name = " << typeName.toStdString() << std::endl;

    // ── 2. Extract struct body ──────────────────────────────────
    QRegularExpression bodyRx(R"(struct\s+\w+\s*\{([^}]+)\})");
    QRegularExpressionMatch bodyMatch = bodyRx.match(idlText);
    if (!bodyMatch.hasMatch())
    {
        std::cerr << "[buildTypeFromIDL] ✗ Could not extract struct body" << std::endl;
        return DynamicType_ptr();
    }
    QStringList lines = bodyMatch.captured(1).split('\n', Qt::SkipEmptyParts);

    // ── 3. Build DynamicType via builder ───────────────────────
    DynamicTypeBuilderFactory *factory = DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder *struct_builder = factory->create_struct_builder();
    if (!struct_builder)
    {
        std::cerr << "[buildTypeFromIDL] ✗ create_struct_builder() returned nullptr" << std::endl;
        return DynamicType_ptr();
    }
    struct_builder->set_name(typeName.toStdString());

    uint32_t memberId = 0;
    for (const QString &rawLine : lines)
    {
        QString line = rawLine.trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        // Strip inline comment
        int commentPos = line.indexOf("//");
        if (commentPos >= 0)
            line = line.left(commentPos).trimmed();
        line = line.remove(';').trimmed();

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;

        // Handle multi-word type keywords: "long long", "unsigned long", etc.
        QString fullType;
        QString fieldName;
        if (parts.size() >= 3 &&
            (parts[0].toLower() == "unsigned" ||
             (parts[0].toLower() == "long" && parts[1].toLower() == "long")))
        {
            fullType = parts[0] + " " + parts[1];
            fieldName = parts[2];
        }
        else
        {
            fullType = parts[0];
            fieldName = parts[1];
        }

        // Strip array brackets from field name (e.g. "data[10]" → "data")
        int bracketPos = fieldName.indexOf('[');
        if (bracketPos >= 0)
            fieldName = fieldName.left(bracketPos);

        std::string fieldNameStd = fieldName.toStdString();
        QString ft = fullType.toLower();

        DynamicTypeBuilder *member_builder = nullptr;

        if (ft == "string")
            member_builder = factory->create_string_builder();
        else if (ft == "boolean" || ft == "bool")
            member_builder = factory->create_bool_builder();
        else if (ft == "long long")
            member_builder = factory->create_int64_builder();
        else if (ft == "unsigned long long")
            member_builder = factory->create_uint64_builder();
        else if (ft == "unsigned long")
            member_builder = factory->create_uint32_builder();
        else if (ft == "long")
            member_builder = factory->create_int32_builder();
        else if (ft == "unsigned short")
            member_builder = factory->create_uint16_builder();
        else if (ft == "short")
            member_builder = factory->create_int16_builder();
        else if (ft == "double")
            member_builder = factory->create_float64_builder();
        else if (ft == "float")
            member_builder = factory->create_float32_builder();
        else if (ft == "octet" || ft == "byte")
            member_builder = factory->create_byte_builder();
        else if (ft == "char")
            member_builder = factory->create_char8_builder();
        else
        {
            std::cerr << "[buildTypeFromIDL] Unknown type '" << ft.toStdString()
                      << "' for field '" << fieldNameStd << "' — defaulting to string" << std::endl;
            member_builder = factory->create_string_builder();
        }

        if (member_builder)
        {
            ReturnCode_t addRet = struct_builder->add_member(memberId++, fieldNameStd, member_builder);
            factory->delete_builder(member_builder);
            if (addRet != ReturnCode_t::RETCODE_OK)
                std::cerr << "[buildTypeFromIDL] ✗ add_member failed for: " << fieldNameStd << std::endl;
            else
                std::cout << "[buildTypeFromIDL]   + member[" << (memberId - 1) << "] "
                          << fieldNameStd << " : " << ft.toStdString() << std::endl;
        }
    }

    DynamicType_ptr built_type = struct_builder->build();
    factory->delete_builder(struct_builder);

    if (!built_type)
    {
        std::cerr << "[buildTypeFromIDL] ✗ DynamicTypeBuilder::build() returned nullptr" << std::endl;
        return DynamicType_ptr();
    }

    std::cout << "[buildTypeFromIDL] ✓ Built DynamicType '" << built_type->get_name()
              << "' with " << memberId << " member(s)" << std::endl;
    return built_type;
}

// ============================================================
// initializeFromIDLText — Fallback initialization path
//
// Called from Controller when TypeLookup has timed out but the
// subscriber-side discovery already populated textData().
// Builds a DynamicType from the IDL text, injects it into
// m_listener so initialize_entities() behaves identically to
// the TypeLookup path, then creates all DDS entities directly.
// ============================================================
bool HelloWorldPublisher::initializeFromIDLText(const QString &idlText)
{
    std::cout << "[initializeFromIDLText] Called, IDL length=" << idlText.length() << std::endl;

    if (idlText.trimmed().isEmpty())
    {
        std::cerr << "[initializeFromIDLText] ✗ IDL text is empty — cannot build type" << std::endl;
        return false;
    }

    // Prefer remotely-discovered type if it arrived late (avoids duplicate type-name registration mismatch).
    eprosima::fastrtps::types::DynamicType_ptr type;
    {
        std::lock_guard<std::mutex> lock(m_listener.types_mx_);
        type = m_listener.received_type_;
    }
    if (type)
    {
        std::cout << "[initializeFromIDLText] Reusing already discovered remote type: "
                  << type->get_name() << std::endl;
    }
    else
    {
        type = buildTypeFromIDL(idlText);
    }
    if (!type)
    {
        std::cerr << "[initializeFromIDLText] ✗ buildTypeFromIDL() failed" << std::endl;
        return false;
    }

    // Inject into m_listener exactly as on_type_information_received would
    {
        std::lock_guard<std::mutex> lock(m_listener.types_mx_);
        m_listener.received_type_ = type;
        m_listener.reception_flag_.store(true);
    }

    // Call initialize_entities directly — no need to go through the CV
    initialize_entities();

    if (!isReady())
    {
        std::cerr << "[initializeFromIDLText] ✗ initialize_entities() produced incomplete state" << std::endl;
        return false;
    }

    std::cout << "[initializeFromIDLText] ✓ DDS entities created via IDL fallback path" << std::endl;
    return true;
}

bool HelloWorldPublisher::initializeFromDiscoveredType(
    const eprosima::fastrtps::types::DynamicType_ptr& type,
    const QString& source_tag)
{
    std::cout << "[initializeFromDiscoveredType] Called from "
              << source_tag.toStdString() << std::endl;

    if (!type)
    {
        std::cerr << "[initializeFromDiscoveredType] ✗ NULL DynamicType_ptr" << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_listener.types_mx_);
        m_listener.received_type_ = type;
        m_listener.reception_flag_.store(true);
    }

    initialize_entities();

    if (!isReady())
    {
        std::cerr << "[initializeFromDiscoveredType] ✗ initialize_entities() produced incomplete state" << std::endl;
        return false;
    }

    std::cout << "[initializeFromDiscoveredType] ✓ DDS entities created from discovered type: "
              << type->get_name() << std::endl;
    return true;
}
