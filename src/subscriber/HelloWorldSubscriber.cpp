#include <indedds-monitor/subscriber/HelloWorldSubscriber.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <iomanip>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>

using namespace eprosima::fastdds::dds;
using eprosima::fastrtps::types::ReturnCode_t;

// Define the global variable here
std::string global_topic_name;

HelloWorldSubscriber::HelloWorldSubscriber()
    : mp_participant(nullptr), mp_subscriber(nullptr), m_listener(this)
{
}

bool HelloWorldSubscriber::init(
    const std::string &topic_name,
    TopicIDLStruct *topicIDLModel,
    int domain_id)
{
    std::cout << "[DEBUG] Entering HelloWorldSubscriber::init()" << std::endl;
    std::cout << "[DEBUG] Input topic_name = " << topic_name << std::endl;
    std::cout << "[DEBUG] topicIDLModel pointer = " << topicIDLModel << std::endl;
    std::cout << "[DEBUG] Requested discovery domain_id = " << domain_id << std::endl;

    global_topic_name = topic_name;
    m_targetTopic = topic_name;
    this->topicIDLModel = topicIDLModel;
    topicIDLModel->setTopicName(QString::fromStdString(topic_name));
    std::cout << "[DEBUG] Topic name set in model = " << topicIDLModel->topicName().toStdString() << std::endl;

    // Access the model values using getter methods
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
    pqos.name("Participant_sub");
    std::cout << "[DEBUG] Participant name set = Participant_sub" << std::endl;

    if (durability == "Transient")
    {
        std::cout << "[DEBUG] Durability is Transient -> Adding persistence plugin property" << std::endl;
        pqos.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    }

    StatusMask par_mask = StatusMask::subscription_matched() << StatusMask::data_available();
    std::cout << "[DEBUG] Creating DomainParticipant on domain " << domain_id
              << " with status mask (subscription_matched | data_available)" << std::endl;

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(
        domain_id, pqos, &m_listener, par_mask);

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

    std::cout << "Reliability from subscriber local variable : " << reliability.toStdString() << std::endl;
    std::cout << "durability from subscriber local variable : " << durability.toStdString() << std::endl;
    std::cout << "ownership from subscriber local variable : " << ownership.toStdString() << std::endl;

    bool isQoSSet = !topicIDLModel->reliability().isEmpty() ||
                    !topicIDLModel->durability().isEmpty() ||
                    !topicIDLModel->ownership().isEmpty();

    std::cout << "[DEBUG] isQoSSet = " << (isQoSSet ? "true" : "false") << std::endl;

    if (!isQoSSet)
    {
        // Use default QoS if none of the properties are set
        qos_ = DATAREADER_QOS_DEFAULT;
        std::cout << "Using default QoS settings: DATAREADER_QOS_DEFAULT" << std::endl;
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

    std::cout << "[DEBUG] Exiting HelloWorldSubscriber::init() with return true" << std::endl;
    return true;
}


HelloWorldSubscriber::~HelloWorldSubscriber()
{
    std::cout << "[DEBUG] Entering ~HelloWorldSubscriber destructor" << std::endl;

    std::cout << "[DEBUG] Cleaning up topics, total topics_: " << topics_.size() << std::endl;
    for (const auto &it : topics_)
    {
        std::cout << "[DEBUG] Deleting DataReader for topic: " << it.second->get_name() << std::endl;
        mp_subscriber->delete_datareader(it.first);

        std::cout << "[DEBUG] Deleting Topic: " << it.second->get_name() << std::endl;
        mp_participant->delete_topic(it.second);
    }

    if (mp_subscriber != nullptr)
    {
        std::cout << "[DEBUG] Deleting Subscriber" << std::endl;
        mp_participant->delete_subscriber(mp_subscriber);
    }
    else
    {
        std::cout << "[DEBUG] mp_subscriber is nullptr, skipping delete" << std::endl;
    }

    std::cout << "[DEBUG] Deleting Participant" << std::endl;
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);

    std::cout << "[DEBUG] Clearing containers: topics_, readers_, datas_" << std::endl;
    topics_.clear();
    readers_.clear();
    datas_.clear();

    std::cout << "[DEBUG] Exiting ~HelloWorldSubscriber destructor" << std::endl;
}

HelloWorldSubscriber::SubListener::SubListener(HelloWorldSubscriber *sub)
    : n_matched(0), n_samples(0), subscriber_(sub)
{
    std::cout << "[DEBUG] SubListener constructed, n_matched = " << n_matched
              << ", n_samples = " << n_samples
              << ", subscriber_ = " << (subscriber_ ? "valid" : "nullptr") << std::endl;
}

HelloWorldSubscriber::SubListener::~SubListener()
{
    std::cout << "[DEBUG] SubListener destructor called" << std::endl;
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
    DataReader *reader,
    const SubscriptionMatchedStatus &info)
{
    std::cout << "[DEBUG] on_subscription_matched triggered" << std::endl;
    std::cout << "[DEBUG] DataReader pointer: " << reader << std::endl;
    std::cout << "[DEBUG] SubscriptionMatchedStatus: "
              << "current_count_change=" << info.current_count_change
              << ", total_count=" << info.total_count
              << ", total_count_change=" << info.total_count_change << std::endl;

    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "[DEBUG] Subscriber matched, n_matched updated to " << n_matched << std::endl;
        std::cout << "Subscriber matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "[DEBUG] Subscriber unmatched, n_matched updated to " << n_matched << std::endl;
        std::cout << "Subscriber unmatched" << std::endl;
    }
    else
    {
        std::cout << "[DEBUG] Unexpected current_count_change value: " << info.current_count_change << std::endl;
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}


void HelloWorldSubscriber::SubListener::on_data_available(DataReader *reader)
{
    std::cout << "[DEBUG] Entered on_data_available()" << std::endl;

    std::cout << "[DEBUG] Looking up DataReader in datas_ map..." << std::endl;
    auto dit = subscriber_->datas_.find(reader);

    if (dit != subscriber_->datas_.end())
    {
        std::cout << "[DEBUG] DataReader found in datas_ map." << std::endl;
        eprosima::fastrtps::types::DynamicData_ptr data = dit->second;
        SampleInfo info;

        std::cout << "[DEBUG] Attempting to take next sample..." << std::endl;
        if (reader->take_next_sample(data.get(), &info) == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[DEBUG] take_next_sample() returned RETCODE_OK." << std::endl;

            if (info.instance_state == ALIVE_INSTANCE_STATE)
            {
                std::cout << "[DEBUG] Instance state is ALIVE_INSTANCE_STATE." << std::endl;

                eprosima::fastrtps::types::DynamicType_ptr type = subscriber_->readers_[reader];
                this->n_samples++;
                std::cout << "[DEBUG] Incremented sample count: " << this->n_samples << std::endl;
                std::cout << "Received data of type " << type->get_name() << std::endl;

                // Create a dynamic data stream
                std::stringstream dynamicDataStream;
                std::cout << "[DEBUG] Calling print_dynamic_data..." << std::endl;
                print_dynamic_data(data, 0, dynamicDataStream);
                std::cout << "[DEBUG] Finished print_dynamic_data call." << std::endl;

                std::cout << "print_dynamic_data nantar  " << std::endl;

                // Extract selected fields
                std::stringstream selectedFieldsStream;
                std::cout << "[DEBUG] Extracting selected fields (currently commented out)." << std::endl;
                // extractSelectedFields(data, selectedFieldsStream);

                std::cout << "extractSelectedFields nantar  " << std::endl;

                // Generate and append the timestamp to the dynamic data stream
                std::time_t now = std::time(nullptr);
                std::tm *local_tm = std::localtime(&now);
                std::cout << "[DEBUG] Current time generated for timestamp." << std::endl;

                dynamicDataStream << "Timestamp: "
                                  << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S ")
                                  << now // Unix timestamp
                                  << " IST" << std::endl;

                // Store the dynamic data in the model
                QString dynamicDataStr = QString::fromStdString(dynamicDataStream.str());
                QString selectedFieldsStr = QString::fromStdString(selectedFieldsStream.str());

                std::cout << "[DEBUG] Converted dynamic and selected field streams to QString." << std::endl;

                // REPLACE the current block that fills ss with this:

std::stringstream ss;

ss << "--- SampleInfo Details ---" << std::endl;

// 1) valid_data first (RTI style)
ss << "valid_data: " << (info.valid_data ? "true" : "false") << std::endl;

// 2) Timestamps and handles
ss << "source_timestamp: " << info.source_timestamp.seconds() << "."
   << std::setfill('0') << std::setw(9) << info.source_timestamp.nanosec() << std::endl;

ss << "reception_timestamp: " << info.reception_timestamp.seconds() << "."
   << std::setfill('0') << std::setw(9) << info.reception_timestamp.nanosec() << std::endl;

ss << "instance_handle: " << info.instance_handle << std::endl;
ss << "publication_handle: " << info.publication_handle << std::endl;

// 3) State & rank
ss << "instance_state: " << info.instance_state << std::endl;
ss << "sample_rank: " << info.sample_rank << std::endl;
ss << "generation_rank: " << info.generation_rank << std::endl;
ss << "absolute_generation_rank: " << info.absolute_generation_rank << std::endl;
ss << "disposed_generation_count: " << info.disposed_generation_count << std::endl;
ss << "no_writers_generation_count: " << info.no_writers_generation_count << std::endl;
ss << "sample_state: " << info.sample_state << std::endl;
ss << "view_state: " << info.view_state << std::endl;

// 4) Identity fields (Fast DDS provides these)
ss << "sample_identity: " << info.sample_identity << std::endl;
ss << "related_sample_identity: " << info.related_sample_identity << std::endl;

// NOTE: RTI-specific sequence / GUID / flag fields are NOT present in
// eprosima::fastdds::dds::SampleInfo for your version, so they cannot
// be accessed here without compile errors.

ss << "-------------------------" << std::endl << std::endl;



                QString sampleInfoStr = QString::fromStdString(ss.str());
                std::cout << "[DEBUG] SampleInfo string prepared." << std::endl;

                // Use the model's addSample method to store both dynamic data and sample info
                QMap<QString, QVariant> sampleData;
                sampleData["dynamicData"] = dynamicDataStr;
                sampleData["selectedFields"] = selectedFieldsStr;
                sampleData["sampleInfo"] = sampleInfoStr;

                std::cout << "[DEBUG] Calling topicIDLModel->addSample with sample index "
                          << (this->n_samples - 1) << std::endl;
                subscriber_->topicIDLModel->addSample(this->n_samples - 1, sampleData);
                std::cout << "[DEBUG] Sample added to topicIDLModel successfully." << std::endl;

                std::cout << "Timestamp: "
                          << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S ")
                          << now // Unix timestamp
                          << " IST" << std::endl;

                // Print the sample info (optional, for debugging)
                std::cout << "[DEBUG] Printing SampleInfo details..." << std::endl;
                std::cout << sampleInfoStr.toStdString();
                std::cout << "[DEBUG] Finished printing SampleInfo details." << std::endl;
            }
            else
            {
                std::cout << "[DEBUG] Instance state is NOT ALIVE_INSTANCE_STATE. Ignored sample." << std::endl;
            }
        }
        else
        {
            std::cout << "[DEBUG] take_next_sample() did not return RETCODE_OK. No sample taken." << std::endl;
        }
    }
    else
    {
        std::cout << "[DEBUG] DataReader NOT found in datas_ map. Ignoring callback." << std::endl;
    }

    std::cout << "[DEBUG] Exiting on_data_available()" << std::endl;
}


void HelloWorldSubscriber::SubListener::print_sample_info(const eprosima::fastdds::dds::SampleInfo &info)
{
    std::cout << "[DEBUG] Entering print_sample_info()" << std::endl;

    std::stringstream ss;
    ss << "\n--- SampleInfo Details ---" << std::endl;
    ss << "sample_state: " << info.sample_state << std::endl;
    ss << "view_state: " << info.view_state << std::endl;
    ss << "instance_state: " << info.instance_state << std::endl;
    ss << "disposed_generation_count: " << info.disposed_generation_count << std::endl; 
    ss << "no_writers_generation_count: " << info.no_writers_generation_count << std::endl;
    ss << "sample_rank: " << info.sample_rank << std::endl;
    ss << "generation_rank: " << info.generation_rank << std::endl;
    ss << "absolute_generation_rank: " << info.absolute_generation_rank << std::endl;

    std::cout << "[DEBUG] Adding timestamps to SampleInfo output" << std::endl;
    ss << "source_timestamp: " << info.source_timestamp.seconds() << "."
       << std::setfill('0') << std::setw(9) << info.source_timestamp.nanosec() << std::endl;
    ss << "reception_timestamp: " << info.reception_timestamp.seconds() << "."
       << std::setfill('0') << std::setw(9) << info.reception_timestamp.nanosec() << std::endl;

    ss << "instance_handle: " << info.instance_handle << std::endl;
    ss << "publication_handle: " << info.publication_handle << std::endl;
    ss << "valid_data: " << (info.valid_data ? "true" : "false") << std::endl;
    ss << "sample_identity: " << info.sample_identity << std::endl;
    ss << "related_sample_identity: " << info.related_sample_identity << std::endl;
    ss << "-------------------------\n"
       << std::endl;

    std::string info_str = ss.str();
    std::cout << "[DEBUG] Generated SampleInfo string:\n" << info_str << std::endl;

    std::cout << "[DEBUG] Setting SampleInfo into topicIDLModel" << std::endl;
    subscriber_->topicIDLModel->setSampleInfo(QString::fromStdString(info_str));

    std::cout << "[DEBUG] Printing SampleInfo to console" << std::endl;
    std::cout << info_str;

    std::cout << "[DEBUG] Exiting print_sample_info()" << std::endl;
}

void HelloWorldSubscriber::print_type_structure(const eprosima::fastrtps::types::DynamicType_ptr &type, int indent)
{
    std::cout << "[DEBUG] Entering print_type_structure() with type: " << type->get_name()
              << ", indent: " << indent << std::endl;

    static std::stringstream ss; // Static to accumulate all data across recursive calls
    if (indent == 0)
    {
        std::cout << "[DEBUG] Top-level call detected, clearing static stringstream" << std::endl;
        ss.str(""); // Clear the stringstream for a new top-level call
        ss.clear();
    }

    std::string indent_str(indent * 2, ' ');
    ss << indent_str << "struct " << type->get_name() << " {" << std::endl;

    std::cout << "[DEBUG] Fetching all members of type: " << type->get_name() << std::endl;
    std::map<eprosima::fastrtps::types::MemberId, eprosima::fastrtps::types::DynamicTypeMember *> members_map;
    type->get_all_members(members_map);

    std::cout << "[DEBUG] Found " << members_map.size() << " members in type: " << type->get_name() << std::endl;
    std::vector<std::pair<eprosima::fastrtps::types::MemberId, eprosima::fastrtps::types::DynamicTypeMember *>> members_vec(members_map.begin(), members_map.end());

    // Sort members by their ID to maintain the original order
    std::cout << "[DEBUG] Sorting members by ID" << std::endl;
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
            std::cout << "[DEBUG] Member " << member->get_name() << " is a nested structure, recursing..." << std::endl;
            ss << std::endl;
            // Recursive call for nested structures
            print_type_structure(member->get_descriptor()->get_type(), indent + 1);
        }
        else if (member->get_descriptor()->get_kind() == eprosima::fastrtps::types::TK_SEQUENCE)
        {
            std::cout << "[DEBUG] Member " << member->get_name() << " is a sequence" << std::endl;
            ss << "[" << member->get_descriptor()->get_type()->get_bounds() << "]";
            ss << ";" << std::endl;
        }
        else
        {
            ss << ";" << std::endl;
        }

        ss << indent_str << std::endl;
    }

    ss << indent_str << "};" << std::endl;

    // Only set the data and print when we're back at the top level (indent == 0)
    if (indent == 0)
    {
        std::cout << "[DEBUG] Top-level completed, setting TopicIDLStruct with accumulated structure" << std::endl;
        
        QString newData = QString::fromStdString(ss.str());
        topicIDLModel->setTextData(newData);
        
        // **THIS IS THE KEY FIX** - Call parseIDLToTree to populate the tree view!
        std::cout << "[DEBUG] ★★★ Calling parseIDLToTree() to populate tree view ★★★" << std::endl;
        topicIDLModel->parseIDLToTree();
        
        std::string data = std::string(topicIDLModel->textData().toStdString());
        std::cout << "[DEBUG] Final generated structure:\n" << data << std::endl;
    }

    std::cout << "[DEBUG] Exiting print_type_structure()" << std::endl;
}




std::string HelloWorldSubscriber::get_type_name(eprosima::fastrtps::types::TypeKind kind)
{
    std::cout << "[DEBUG] Entering get_type_name() with TypeKind value: " << kind << std::endl;

    switch (kind)
    {
    case eprosima::fastrtps::types::TK_BOOLEAN:
        std::cout << "[DEBUG] Matched TK_BOOLEAN" << std::endl;
        return "boolean";
    case eprosima::fastrtps::types::TK_BYTE:
        std::cout << "[DEBUG] Matched TK_BYTE" << std::endl;
        return "octet";
    case eprosima::fastrtps::types::TK_INT16:
        std::cout << "[DEBUG] Matched TK_INT16" << std::endl;
        return "short";
    case eprosima::fastrtps::types::TK_INT32:
        std::cout << "[DEBUG] Matched TK_INT32" << std::endl;
        return "long";
    case eprosima::fastrtps::types::TK_INT64:
        std::cout << "[DEBUG] Matched TK_INT64" << std::endl;
        return "long long";
    case eprosima::fastrtps::types::TK_UINT16:
        std::cout << "[DEBUG] Matched TK_UINT16" << std::endl;
        return "unsigned short";
    case eprosima::fastrtps::types::TK_UINT32:
        std::cout << "[DEBUG] Matched TK_UINT32" << std::endl;
        return "unsigned long";
    case eprosima::fastrtps::types::TK_UINT64:
        std::cout << "[DEBUG] Matched TK_UINT64" << std::endl;
        return "unsigned long long";
    case eprosima::fastrtps::types::TK_FLOAT32:
        std::cout << "[DEBUG] Matched TK_FLOAT32" << std::endl;
        return "float";
    case eprosima::fastrtps::types::TK_FLOAT64:
        std::cout << "[DEBUG] Matched TK_FLOAT64" << std::endl;
        return "double";
    case eprosima::fastrtps::types::TK_STRING8:
        std::cout << "[DEBUG] Matched TK_STRING8" << std::endl;
        return "string";
    case eprosima::fastrtps::types::TK_STRING16:
        std::cout << "[DEBUG] Matched TK_STRING16" << std::endl;
        return "wstring";
    case eprosima::fastrtps::types::TK_SEQUENCE:
        std::cout << "[DEBUG] Matched TK_SEQUENCE" << std::endl;
        return "sequence";
    case eprosima::fastrtps::types::TK_ARRAY:
        std::cout << "[DEBUG] Matched TK_ARRAY" << std::endl;
        return "array";
    case eprosima::fastrtps::types::TK_STRUCTURE:
        std::cout << "[DEBUG] Matched TK_STRUCTURE" << std::endl;
        return "struct";
    default:
        std::cout << "[DEBUG] Unknown TypeKind: " << kind << std::endl;
        return "unknown";
    }
}

void HelloWorldSubscriber::SubListener::on_type_discovery(
    DomainParticipant *participant,
    const eprosima::fastrtps::rtps::SampleIdentity &sample_id,
    const eprosima::fastrtps::string_255 &topic_name,
    const eprosima::fastrtps::types::TypeIdentifier *type_id,
    const eprosima::fastrtps::types::TypeObject *type_obj,
    eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    std::cout << "[DEBUG] Entering on_type_discovery()" << std::endl;
    std::cout << "[DEBUG] Participant: " << participant << std::endl;
    std::cout << "[DEBUG] SampleIdentity writer_guid: " << sample_id.writer_guid() << std::endl;
    std::cout << "[DEBUG] Topic name received: " << topic_name << std::endl;
    std::cout << "[DEBUG] TypeIdentifier pointer: " << type_id << std::endl;
    std::cout << "[DEBUG] TypeObject pointer: " << type_obj << std::endl;
    std::cout << "[DEBUG] DynamicType pointer: " << dyn_type << std::endl;

    const std::string received_topic = topic_name.c_str();
    if (received_topic != subscriber_->targetTopic())
    {
        std::cout << "[DEBUG] Ignoring discovered type for unrelated topic: "
                  << received_topic << " (expected " << subscriber_->targetTopic() << ")" << std::endl;
        return;
    }

    if (dyn_type)
    {
        std::cout << "[DEBUG] Discovered type name: " << dyn_type->get_name() << std::endl;
        std::cout << "Discovered type: " << dyn_type->get_name()
                  << " from topic " << subscriber_->targetTopic() << std::endl;

        std::cout << "[DEBUG] Printing IDL structure of type: " << dyn_type->get_name() << std::endl;
        std::cout << "IDL structure:" << std::endl;

        subscriber_->print_type_structure(dyn_type);

        std::cout << "[DEBUG] Storing discovered DynamicType in received_type_" << std::endl;
        received_type_ = dyn_type;

        std::cout << "[DEBUG] Setting reception_flag_ to true" << std::endl;
        reception_flag_.store(true);

        std::cout << "[DEBUG] Notifying waiting threads using condition variable" << std::endl;
        types_cv_.notify_one();
    }
    else
    {
        std::cout << "[DEBUG][WARNING] dyn_type is NULL. Skipping processing." << std::endl;
    }

    std::cout << "[DEBUG] Exiting on_type_discovery()" << std::endl;
}


void HelloWorldSubscriber::initialize_entities()
{
    auto type = m_listener.received_type_;
    std::cout << "[DEBUG] Entering initialize_entities()" << std::endl;

    if (!type)
    {
        std::cout << "[ERROR] No type received in listener. Cannot initialize entities." << std::endl;
        return;
    }

    std::cout << "[DEBUG] Initializing DDS entities for type: " << type->get_name() << std::endl;

    TypeSupport m_type(new eprosima::fastrtps::types::DynamicPubSubType(type));
    if (!m_type)
    {
        std::cout << "[ERROR] Failed to create DynamicPubSubType for type: " << type->get_name() << std::endl;
        return;
    }

    std::cout << "[DEBUG] Registering type with participant..." << std::endl;
    m_type.register_type(mp_participant);
    std::cout << "[DEBUG] Type registered successfully." << std::endl;

    if (mp_subscriber == nullptr)
    {
        std::cout << "[DEBUG] Creating subscriber..." << std::endl;
        mp_subscriber = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (mp_subscriber == nullptr)
        {
            std::cout << "[ERROR] Failed to create Subscriber." << std::endl;
            return;
        }
        else
        {
            std::cout << "[DEBUG] Subscriber created successfully." << std::endl;
        }
    }
    else
    {
        std::cout << "[DEBUG] Subscriber already exists, skipping creation." << std::endl;
    }

    std::cout << "[DEBUG] Creating topic with name: " << m_targetTopic
              << " and type: " << m_type->getName() << std::endl;

    Topic *topic = mp_participant->create_topic(
        m_targetTopic,
        m_type->getName(),
        TOPIC_QOS_DEFAULT);

    if (topic == nullptr)
    {
        std::cout << "[ERROR] Failed to create Topic: " << m_targetTopic << std::endl;
        return;
    }
    else
    {
        std::cout << "[DEBUG] Topic created successfully: " << m_targetTopic << std::endl;
    }

    // Durability QoS debug
    switch (qos_.durability().kind)
    {
    case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to VOLATILE_DURABILITY_QOS " << std::endl;
        break;
    case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to TRANSIENT_LOCAL_DURABILITY_QOS " << std::endl;
        break;
    case eprosima::fastrtps::TRANSIENT_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to TRANSIENT_DURABILITY_QOS " << std::endl;
        break;
    case eprosima::fastrtps::PERSISTENT_DURABILITY_QOS:
        std::cout << "[DEBUG] Durability set to PERSISTENT_DURABILITY_QOS " << std::endl;
        break;
    default:
        std::cout << "[WARN] Unknown durability QoS setting." << std::endl;
        break;
    }

    // Ownership QoS debug
    switch (qos_.ownership().kind)
    {
    case eprosima::fastrtps::SHARED_OWNERSHIP_QOS:
        std::cout << "[DEBUG] Ownership set to SHARED_OWNERSHIP_QOS " << std::endl;
        break;
    case eprosima::fastrtps::EXCLUSIVE_OWNERSHIP_QOS:
        std::cout << "[DEBUG] Ownership set to EXCLUSIVE_OWNERSHIP_QOS " << std::endl;
        break;
    default:
        std::cout << "[WARN] Unknown ownership QoS setting." << std::endl;
        break;
    }

    // Reliability QoS debug
    switch (qos_.reliability().kind)
    {
    case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
        std::cout << "[DEBUG] Reliability set to RELIABLE_RELIABILITY_QOS " << std::endl;
        break;
    case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
        std::cout << "[DEBUG] Reliability set to BEST_EFFORT_RELIABILITY_QOS " << std::endl;
        break;
    default:
        std::cout << "[WARN] Unknown reliability QoS setting." << std::endl;
        break;
    }

    std::cout << "[DEBUG] Creating DataReader..." << std::endl;
    StatusMask sub_mask = StatusMask::subscription_matched() << StatusMask::data_available();
    DataReader *reader = mp_subscriber->create_datareader(
        topic,
        qos_,
        &m_listener,
        sub_mask);

    if (!reader)
    {
        std::cout << "[ERROR] Failed to create DataReader for topic: " << m_targetTopic << std::endl;
        return;
    }
    else
    {
        std::cout << "[DEBUG] DataReader created successfully for topic: " << m_targetTopic << std::endl;
    }

    topics_[reader] = topic;
    readers_[reader] = type;

    std::cout << "[DEBUG] Creating DynamicData for type: " << type->get_name() << std::endl;
    eprosima::fastrtps::types::DynamicData_ptr data(
        eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(type));

    if (!data)
    {
        std::cout << "[ERROR] Failed to create DynamicData instance for type: " << type->get_name() << std::endl;
        return;
    }

    datas_[reader] = data;
    std::cout << "[DEBUG] DynamicData created and stored successfully." << std::endl;

    std::cout << "[DEBUG] Exiting initialize_entities()" << std::endl;
}


void HelloWorldSubscriber::SubListener::print_dynamic_data(
    const eprosima::fastrtps::types::DynamicData_ptr& data,
    int indent,
    std::stringstream& dynamicDataStream)
{
    std::cout << "[DEBUG] Entering print_dynamic_data with indent=" << indent 
              << ", itemcount=" << data->get_item_count() << std::endl;

    std::string indent_str(indent * 2, ' ');

    for (uint32_t i = 0; i < data->get_item_count(); ++i)
    {
        std::cout << "[DEBUG] Processing item index " << i << std::endl;

        eprosima::fastrtps::types::MemberId member_id = data->get_member_id_at_index(i);
        std::cout << "[DEBUG] Got member_id=" << member_id << std::endl;

        eprosima::fastrtps::types::MemberDescriptor descriptor;
        if (data->get_descriptor(descriptor, member_id) != eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[DEBUG/ERROR] Failed to get descriptor for member_id=" << member_id << std::endl;
            continue;
        }

        std::string member_name = descriptor.get_name();
        std::cout << "[DEBUG] Processing member: " << member_name << std::endl;

        std::cout << indent_str << member_name << ": ";
        dynamicDataStream << indent_str << member_name << ": ";

        auto member_kind = descriptor.get_kind();
        // Print as INTEGER to see actual TypeKind value
        std::cout << "[DEBUG] Member kind=" << static_cast<uint32_t>(member_kind) << std::endl;

        try {
            switch (member_kind)
            {
                case eprosima::fastrtps::types::TK_BOOLEAN:
                {
                    bool val = data->get_bool_value(member_id);
                    std::cout << "[DEBUG] Boolean value: " << val << std::endl;
                    std::cout << (val ? "true" : "false");
                    dynamicDataStream << (val ? "true" : "false");
                    break;
                }

                case eprosima::fastrtps::types::TK_BYTE:  // octet (unsigned byte) - TypeKind = 10
                    {
                    std::cout << "[DEBUG] Handling TK_BYTE/TK_INT8" << std::endl;
                    
                    // Try int8 first (signed)
                    int8_t sval;
                    eprosima::fastrtps::types::ReturnCode_t ret = data->get_int8_value(sval, member_id);
                    
                    if (ret == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Int8 value: " << static_cast<int>(sval) << std::endl;
                        std::cout << static_cast<int>(sval);
                        dynamicDataStream << static_cast<int>(sval);
                    }
                    else
                    {
                        // Try octet (unsigned)
                        eprosima::fastrtps::types::octet uval;
                        ret = data->get_byte_value(uval, member_id);
                        
                        if (ret == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                        {
                            std::cout << "[DEBUG] Byte/Octet value: " << static_cast<int>(uval) << std::endl;
                            std::cout << static_cast<int>(uval);
                            dynamicDataStream << static_cast<int>(uval);
                        }
                        else
                        {
                            // Try uint8
                            uint8_t u8val;
                            ret = data->get_uint8_value(u8val, member_id);
                            if (ret == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                            {
                                std::cout << "[DEBUG] UInt8 value: " << static_cast<int>(u8val) << std::endl;
                                std::cout << static_cast<int>(u8val);
                                dynamicDataStream << static_cast<int>(u8val);
                            }
                            else
                            {
                                std::cout << "[ERROR] All byte-type getters failed" << std::endl;
                                std::cout << "N/A";
                                dynamicDataStream << "N/A";
                            }
                        }
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_INT16:
                {
                    int16_t val;
                    if (data->get_int16_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Int16 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get int16 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_INT32:
                {
                    int32_t val;
                    if (data->get_int32_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Int32 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get int32 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_INT64:
                {
                    int64_t val;
                    if (data->get_int64_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Int64 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get int64 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_UINT16:
                {
                    uint16_t val;
                    if (data->get_uint16_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] UInt16 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get uint16 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_UINT32:
                {
                    uint32_t val;
                    if (data->get_uint32_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] UInt32 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get uint32 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_UINT64:
                {
                    uint64_t val;
                    if (data->get_uint64_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] UInt64 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get uint64 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_FLOAT32:
                {
                    float val;
                    if (data->get_float32_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Float32 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get float32 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_FLOAT64:
                {
                    double val;
                    if (data->get_float64_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Float64 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get float64 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_CHAR8:
                {
                    char val;
                    if (data->get_char8_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Char8 value: " << val << std::endl;
                        std::cout << val;
                        dynamicDataStream << val;
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get char8 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_CHAR16:  // wchar
                {
                    wchar_t val;
                    if (data->get_char16_value(val, member_id) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
                    {
                        std::cout << "[DEBUG] Char16/wchar value: " << static_cast<int>(val) << std::endl;
                        std::cout << static_cast<int>(val);
                        dynamicDataStream << static_cast<int>(val);
                    }
                    else
                    {
                        std::cout << "[ERROR] Failed to get char16 value" << std::endl;
                        std::cout << "N/A";
                        dynamicDataStream << "N/A";
                    }
                    break;
                }

                case eprosima::fastrtps::types::TK_STRING8:
                {
                    std::string val = data->get_string_value(member_id);
                    std::cout << "[DEBUG] String value: " << val << std::endl;
                    std::cout << "\"" << val << "\"";
                    dynamicDataStream << "\"" << val << "\"";
                    break;
                }

                case eprosima::fastrtps::types::TK_STRING16:
                {
                    std::wstring val = data->get_wstring_value(member_id);
                    std::cout << "[DEBUG] WString length: " << val.length() << std::endl;
                    std::wcout << L"\"" << val << L"\"";
                    dynamicDataStream << "\"<wstring>\"";
                    break;
                }

                case eprosima::fastrtps::types::TK_STRUCTURE:
                {
                    std::cout << "[DEBUG] Entering STRUCTURE member: " << member_name << std::endl;
                    std::cout << std::endl;
                    dynamicDataStream << std::endl;

                    eprosima::fastrtps::types::DynamicData* inner_data = data->loan_value(member_id);
                    std::cout << "[DEBUG] Loaned STRUCTURE value for member_id=" << member_id << std::endl;

                    eprosima::fastrtps::types::DynamicData_ptr inner_ptr(inner_data);
                    print_dynamic_data(inner_ptr, indent + 1, dynamicDataStream);

                    data->return_loaned_value(inner_data);
                    std::cout << "[DEBUG] Returned STRUCTURE loaned value for member_id=" << member_id << std::endl;
                    break;
                }

                case eprosima::fastrtps::types::TK_SEQUENCE:
                {
                    std::cout << "[DEBUG] Entering SEQUENCE member: " << member_name << std::endl;
                    std::cout << "[";
                    dynamicDataStream << "[";

                    eprosima::fastrtps::types::DynamicData* sequence = data->loan_value(member_id);
                    std::cout << "[DEBUG] Loaned SEQUENCE with itemcount=" << sequence->get_item_count() << std::endl;

                    for (uint32_t j = 0; j < sequence->get_item_count(); ++j)
                    {
                        std::cout << "[DEBUG] Processing SEQUENCE index " << j << std::endl;
                        if (j > 0)
                        {
                            std::cout << ", ";
                            dynamicDataStream << ", ";
                        }

                        eprosima::fastrtps::types::DynamicData* item = sequence->loan_value(j);
                        std::cout << "[DEBUG] Loaned SEQUENCE item at index " << j << std::endl;

                        eprosima::fastrtps::types::DynamicData_ptr item_ptr(item);
                        print_dynamic_data(item_ptr, 0, dynamicDataStream);

                        sequence->return_loaned_value(item);
                        std::cout << "[DEBUG] Returned SEQUENCE item at index " << j << std::endl;
                    }

                    data->return_loaned_value(sequence);
                    std::cout << "[DEBUG] Returned SEQUENCE loaned value for member_id=" << member_id << std::endl;
                    std::cout << "]";
                    dynamicDataStream << "]";
                    break;
                }

                default:
                {
                    // Handle UNKNOWN types gracefully - don't throw!
                    std::cout << "[WARNING] Unsupported/Unknown TypeKind: " << static_cast<uint32_t>(member_kind) 
                              << " for member: " << member_name << std::endl;
                    std::cout << "<Unknown type>";
                    dynamicDataStream << "<Unknown type>";
                }
            }
        }
        catch (const eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue& e)
        {
            std::cout << "[ERROR] Exception caught while reading member '" << member_name 
                      << "': ReturnCode exception" << std::endl;
            std::cout << "<Error>";
            dynamicDataStream << "<Error>";
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] std::exception caught while reading member '" << member_name 
                      << "': " << e.what() << std::endl;
            std::cout << "<Exception>";
            dynamicDataStream << "<Exception>";
        }
        catch (...)
        {
            std::cout << "[ERROR] Unknown exception caught while reading member '" << member_name << "'" << std::endl;
            std::cout << "<Unknown Error>";
            dynamicDataStream << "<Unknown Error>";
        }

        std::cout << std::endl;
        dynamicDataStream << std::endl;
        std::cout << "[DEBUG] Finished processing member: " << member_name << std::endl;
    }

    std::cout << "[DEBUG] Exiting print_dynamic_data with indent=" << indent << std::endl;
}



// void HelloWorldSubscriber::SubListener::extractSelectedFields(
//     const eprosima::fastrtps::types::DynamicData_ptr &data,
//     std::stringstream &selectedFieldsStream)
// {
//     std::cout << "extractSelectedFields cha atmadhe " << std::endl;

//     bool first = true;
//     for (uint32_t i = 0; i < data->get_item_count(); ++i)
//     {
//         std::cout << "extractSelectedFields loop cha atmadhe " << std::endl;
//         eprosima::fastrtps::types::MemberId member_id = data->get_member_id_at_index(i);
//         eprosima::fastrtps::types::MemberDescriptor descriptor;
//         data->get_descriptor(descriptor, member_id);
//         std::string member_name = descriptor.get_name();

//         auto member_kind = descriptor.get_kind();
//         switch (member_kind)
//         {
//             case eprosima::fastrtps::types::TK_FLOAT32:
//             case eprosima::fastrtps::types::TK_FLOAT64:
//             case eprosima::fastrtps::types::TK_INT16:
//             case eprosima::fastrtps::types::TK_INT32:
//             case eprosima::fastrtps::types::TK_INT64:
//             case eprosima::fastrtps::types::TK_UINT16:
//             case eprosima::fastrtps::types::TK_UINT32:
//             case eprosima::fastrtps::types::TK_UINT64:
//             case eprosima::fastrtps::types::TK_STRING8:
//                 {
//                     std::string value;
//                     if (member_kind == eprosima::fastrtps::types::TK_STRING8)
//                         value = data->get_string_value(member_id);
//                     else if (member_kind == eprosima::fastrtps::types::TK_FLOAT32 || member_kind == eprosima::fastrtps::types::TK_FLOAT64)
//                         value = std::to_string(data->get_float64_value(member_id));
//                     else
//                         value = std::to_string(data->get_uint64_value(member_id));

//                     if (!first) selectedFieldsStream << ", ";
//                     selectedFieldsStream << member_name << ": " << value;
//                     first = false;
//                 }
//                 break;
//             case eprosima::fastrtps::types::TK_STRUCTURE:
//                 {
//                     if (!first) selectedFieldsStream << ", ";
//                     selectedFieldsStream << member_name << ": ";
//                     eprosima::fastrtps::types::DynamicData *inner_data = data->loan_value(member_id);
//                     extractSelectedFields(eprosima::fastrtps::types::DynamicData_ptr(inner_data), selectedFieldsStream);
//                     data->return_loaned_value(inner_data);
//                     first = false;
//                 }
//                 break;
//                 std::cout << "extractSelectedFields last cha atmadhe " << std::endl;
//         }
//     }
// }

void HelloWorldSubscriber::SubListener::on_requested_incompatible_qos(
    DataReader *reader,
    const RequestedIncompatibleQosStatus &info)
{
    std::cout << "[DEBUG] Entering on_requested_incompatible_qos()" << std::endl;
    (void)reader; // Suppress unused parameter warning

    std::cout << "[DEBUG] QoS Incompatibility detected" << std::endl;
    std::cout << "Found a remote Topic with incompatible QoS (QoS ID: "
              << info.last_policy_id << ")" << std::endl;

    std::cout << "[DEBUG] Exiting on_requested_incompatible_qos()" << std::endl;
}

void HelloWorldSubscriber::run()
{
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] HelloWorldSubscriber::run() STARTING" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] Thread ID: " << std::this_thread::get_id() << std::endl;
    std::cout << "[DEBUG] Subscriber object at: " << this << std::endl;
    
    // ========== SET RUNNING FLAG ==========
    m_running = true;
    std::cout << "[DEBUG] ✓ Set m_running = TRUE" << std::endl;
    std::cout << "[DEBUG] Current m_running state: " << (m_running ? "RUNNING" : "STOPPED") << std::endl;
    
    std::cout << "[DEBUG] Acquiring lock on types_mx_" << std::endl;
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    
    std::cout << "[DEBUG] Waiting on types_cv_ condition variable for type discovery..." << std::endl;
    m_listener.types_cv_.wait(lock, [&]()
    {
        std::cout << "[DEBUG] Condition check: reception_flag_ = " 
                  << m_listener.reception_flag_.load() << std::endl;
        bool result = m_listener.reception_flag_.exchange(false);
        return result;
    });
    
    std::cout << "[DEBUG] ✓ Type discovered! Proceeding to initialize_entities()" << std::endl;
    
    // Initialize DDS entities
    initialize_entities();
    
    std::cout << "[DEBUG] ✓ initialize_entities() completed" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] ENTERING MAIN SUBSCRIBER LOOP" << std::endl;
    std::cout << "[DEBUG] Will check m_running flag every 100ms" << std::endl;
    std::cout << "[DEBUG] Call stop() to exit this loop" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
    
    // ========== MAIN LOOP - CHECK m_running FLAG ==========
    int loop_count = 0;
    while (m_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Print status every 10 seconds (100 iterations * 100ms)
        if (++loop_count % 100 == 0)
        {
            std::cout << "[DEBUG] Subscriber still running... (loop " << loop_count << ")" << std::endl;
        }
    }
    
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] m_running = FALSE detected!" << std::endl;
    std::cout << "[DEBUG] Exiting main subscriber loop" << std::endl;
    std::cout << "[DEBUG] Subscriber stopping gracefully..." << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] HelloWorldSubscriber::run() EXITING" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
}



void HelloWorldSubscriber::run(uint32_t number)
{
    std::cout << "[DEBUG] Entering run(number)" << std::endl;
    std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;

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

    // Call initialize_entities with the global topic name
    initialize_entities();
    std::cout << "[DEBUG] initialize_entities() completed" << std::endl;

    std::cout << "[DEBUG] Entering sample reception loop until " << number << " samples received" << std::endl;
    while (number > this->m_listener.n_samples)
    {
        std::cout << "[DEBUG] Current n_samples = " << this->m_listener.n_samples
                  << ", target = " << number << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "[DEBUG] Target sample count reached, exiting run(number)" << std::endl;
}

// NEW: Timeout version for discovery (non-blocking)
void HelloWorldSubscriber::runWithTimeout(uint32_t timeout_seconds)
{
    std::cout << "[DEBUG] Entering runWithTimeout() with timeout: " << timeout_seconds << " seconds" << std::endl;
    
    std::unique_lock<std::mutex> lock(m_listener.types_mx_);
    std::cout << "[DEBUG] Waiting for type discovery with " << timeout_seconds << "s timeout..." << std::endl;
    
    // Wait with timeout
    bool discovered = m_listener.types_cv_.wait_for(
        lock,
        std::chrono::seconds(timeout_seconds),
        [&]() {
            bool result = m_listener.reception_flag_.exchange(false);
            std::cout << "[DEBUG] Condition check: reception_flag_ = " << result << std::endl;
            return result;
        }
    );
    
    if (discovered) {
        std::cout << "[DEBUG] ✓ Type discovered within " << timeout_seconds << "s!" << std::endl;
        initialize_entities();
        print_type_structure(m_listener.received_type_);  // Generate IDL
        std::cout << "[DEBUG] Entities initialized and IDL generated" << std::endl;
    } else {
        std::cout << "[ERROR] ✗ Type discovery TIMEOUT after " << timeout_seconds << " seconds" << std::endl;
    }
    
    std::cout << "[DEBUG] Exiting runWithTimeout()" << std::endl;
}


void HelloWorldSubscriber::stop()
{
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] HelloWorldSubscriber::stop() CALLED" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
    std::cout << "[DEBUG] Subscriber object at: " << this << std::endl;
    std::cout << "[DEBUG] Current m_running state BEFORE: " << (m_running ? "RUNNING" : "STOPPED") << std::endl;
    
    if (!m_running)
    {
        std::cout << "[DEBUG] ⚠ WARNING: Subscriber was already stopped!" << std::endl;
        std::cout << "[DEBUG] This might indicate the wrong subscriber object was stopped" << std::endl;
        std::cout << "[DEBUG] ========================================" << std::endl;
        return;
    }
    
    std::cout << "[DEBUG] Setting m_running = FALSE" << std::endl;
    m_running = false;
    
    std::cout << "[DEBUG] Current m_running state AFTER: " << (m_running ? "RUNNING" : "STOPPED") << std::endl;
    std::cout << "[DEBUG] Notifying condition variable (in case waiting)" << std::endl;
    m_listener.types_cv_.notify_all();
    
    std::cout << "[DEBUG] ✓ Stop signal sent successfully" << std::endl;
    std::cout << "[DEBUG] Thread will exit run() loop on next iteration" << std::endl;
    std::cout << "[DEBUG] ========================================" << std::endl;
}


// ========== END NEW FUNCTION ==========
