// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <sqlite3.h>
#include <iomanip>
#include <sstream>
#include <limits.h>
#include <string>
#include <unistd.h>
#include <libgen.h>
//#include <filesystem>

#include "dloggerSubscriber.h"
#include "dloggerPubSubTypes.h"

//namespace fs = std::filesystem;

using namespace eprosima::fastdds::dds;

namespace
{
    std::string kindToString(int kindValue)
    {
        switch (kindValue)
        {
        case 0:
            return "Error";
        case 1:
            return "Warning";
        case 2:
            return "Info";
        case 3:
            return "Notice";
        case 4:
            return "Debug";
        case 5:
            return "DL_Error";
        case 6:
            return "DL_Warning";
        case 7:
            return "DL_Info";
        case 8:
            return "DL_Notice";
        case 9:
            return "DL_Debug";
        default:
            return "UNKNOWN";
        }
    }
}

dloggerSubscriber::dloggerSubscriber()
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new LoggingModule::DL_EntryPubSubType())
{
}

dloggerSubscriber::~dloggerSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool dloggerSubscriber::initialize_database(sqlite3** db)
{
    int rc = sqlite3_open("/home/cdac/indedds/monitor/src/DataBase/logs.db", db);
    if (rc)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }

    std::string create_table_query =
        "CREATE TABLE IF NOT EXISTS Logs ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "HostID TEXT, "
        "Process TEXT, "  
        "Message TEXT, "
        "Timestamp TEXT, " 
        "Filename TEXT, "
        "Line INTEGER, "
        "Function TEXT, "
        "Category TEXT, "
        "Kind TEXT);";  

    rc = sqlite3_exec(*db, create_table_query.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }

    return true;
}

bool dloggerSubscriber::init(
    int domain_id)
{
    std::cout << "[DEBUG] Entering dloggerSubscriber::init()" << std::endl;

    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    std::cout << "[DEBUG] DomainParticipantQos name set to: " << pqos.name() << std::endl;

    std::cout << "[DEBUG] Creating DomainParticipant with domain ID = " << domain_id << std::endl;
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain_id, pqos);
    if (participant_ == nullptr)
    {
        std::cerr << "[ERROR] Failed to create DomainParticipant." << std::endl;
        std::cout << "[DEBUG] Exiting dloggerSubscriber::init() with return = false" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] DomainParticipant created successfully." << std::endl;

    std::cout << "[DEBUG] Registering type with participant." << std::endl;
    type_.register_type(participant_);
    std::cout << "[DEBUG] Type registered: " << type_.get_type_name() << std::endl;

    std::cout << "[DEBUG] Creating Subscriber with default QoS." << std::endl;
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        std::cerr << "[ERROR] Failed to create Subscriber." << std::endl;
        std::cout << "[DEBUG] Exiting dloggerSubscriber::init() with return = false" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Subscriber created successfully." << std::endl;

    std::cout << "[DEBUG] Creating Topic with name = 'dloggerTopic', type = " << type_.get_type_name() << std::endl;
    topic_ = participant_->create_topic(
        "dloggerTopic",
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cerr << "[ERROR] Failed to create Topic." << std::endl;
        std::cout << "[DEBUG] Exiting dloggerSubscriber::init() with return = false" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Topic created successfully." << std::endl;

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    std::cout << "[DEBUG] DataReaderQos set with Reliability kind = RELIABLE" << std::endl;

    std::cout << "[DEBUG] Creating DataReader for topic 'dloggerTopic'." << std::endl;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
    if (reader_ == nullptr)
    {
        std::cerr << "[ERROR] Failed to create DataReader." << std::endl;
        std::cout << "[DEBUG] Exiting dloggerSubscriber::init() with return = false" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] DataReader created successfully." << std::endl;

    std::cout << "[DEBUG] Exiting dloggerSubscriber::init() with return = true" << std::endl;
    return true;
}

void dloggerSubscriber::SubListener::on_data_available(
    DataReader *reader)
{
    std::cout << "[DEBUG] Entering SubListener::on_data_available()" << std::endl;

    LoggingModule::DL_Entry st;
    SampleInfo info;

    std::cout << "[DEBUG] Calling reader->take_next_sample()" << std::endl;
    if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << "[DEBUG] take_next_sample() returned RETCODE_OK" << std::endl;

        if (info.valid_data)
        {
            std::cout << "[DEBUG] Received valid data sample." << std::endl;
            ++samples;
            std::cout << "[DEBUG] Total samples received so far: " << samples << std::endl;

            sqlite3 *db;
            std::cout << "[DEBUG] Initializing database connection..." << std::endl;
            if (!dloggerSubscriber::initialize_database(&db)) // Static function call
            {
                std::cerr << "[ERROR] Failed to initialize the database." << std::endl;
                std::cout << "[DEBUG] Exiting SubListener::on_data_available() early due to DB init failure." << std::endl;
                return;
            }
            std::cout << "[DEBUG] Database connection established successfully." << std::endl;

            std::string insert_query =
                "INSERT INTO Logs (HostID, Process, Message, Timestamp, Filename, Line, Function, Category, Kind) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
            std::cout << "[DEBUG] Preparing SQL insert query: " << insert_query << std::endl;

            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(db, insert_query.c_str(), -1, &stmt, nullptr);
            std::cout << "[DEBUG] sqlite3_prepare_v2() returned rc = " << rc << std::endl;

            if (rc != SQLITE_OK)
            {
                std::cerr << "[ERROR] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                std::cout << "[DEBUG] Database connection closed after statement preparation failure." << std::endl;
                return;
            }
            std::cout << "[DEBUG] SQL statement prepared successfully." << std::endl;

            // Bind values to the prepared statement
            std::cout << "[DEBUG] Binding values to SQL statement..." << std::endl;
            std::cout << "   HostID = " << st.host_id() << std::endl;
            std::cout << "   Process = Distributed Logger Example" << std::endl;
            std::cout << "   Message = " << st.message() << std::endl;

            sqlite3_bind_text(stmt, 1, st.host_id().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, "Distributed Logger Example", -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, st.message().c_str(), -1, SQLITE_STATIC);

            // Convert timestamp to desired string format
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&in_time_t), "%I:%M:%S")
                << ':' << std::setw(3) << std::setfill('0') << milliseconds.count()
                << ' ' << std::put_time(std::localtime(&in_time_t), "%p")
                << ' ' << std::put_time(std::localtime(&in_time_t), "%A")
                << ' ' << std::put_time(std::localtime(&in_time_t), "%m/%d/%Y");
            std::string timestamp_str = oss.str();

            std::cout << "   Timestamp = " << timestamp_str << std::endl;
            sqlite3_bind_text(stmt, 4, timestamp_str.c_str(), -1, SQLITE_STATIC);

            std::cout << "   Filename = " << st.log_context().filename() << std::endl;
            std::cout << "   Line = " << st.log_context().line() << std::endl;
            std::cout << "   Function = " << st.log_context().function() << std::endl;
            std::cout << "   Category = " << st.log_context().category() << std::endl;
            std::cout << "   Kind = " << kindToString(static_cast<int>(st.kind())) << std::endl;

            sqlite3_bind_text(stmt, 5, st.log_context().filename().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 6, st.log_context().line());
            sqlite3_bind_text(stmt, 7, st.log_context().function().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 8, st.log_context().category().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 9, kindToString(static_cast<int>(st.kind())).c_str(), -1, SQLITE_STATIC);

            std::cout << "[DEBUG] Executing SQL statement..." << std::endl;
            rc = sqlite3_step(stmt);
            std::cout << "[DEBUG] sqlite3_step() returned rc = " << rc << std::endl;

            if (rc != SQLITE_DONE)
            {
                std::cerr << "[ERROR] Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
            }
            else
            {
                std::cout << "[DEBUG] SQL statement executed successfully." << std::endl;
            }

            sqlite3_finalize(stmt);
            std::cout << "[DEBUG] SQL statement finalized." << std::endl;

            sqlite3_close(db);
            std::cout << "[DEBUG] Database connection closed." << std::endl;

            std::cout << "Log successfully stored in database." << std::endl;
        }
        else
        {
            std::cout << "[DEBUG] Received sample is not valid_data, ignoring." << std::endl;
        }
    }
    else
    {
        std::cout << "[DEBUG] take_next_sample() did not return RETCODE_OK." << std::endl;
    }

    std::cout << "[DEBUG] Exiting SubListener::on_data_available()" << std::endl;
}

void dloggerSubscriber::SubListener::on_subscription_matched(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::SubscriptionMatchedStatus &info)
{
    std::cout << "[DEBUG] Entering SubListener::on_subscription_matched()" << std::endl;
    std::cout << "[DEBUG] DataReader pointer = " << reader << std::endl;
    std::cout << "[DEBUG] SubscriptionMatchedStatus details:" << std::endl;
    std::cout << "    total_count = " << info.total_count << std::endl;
    std::cout << "    total_count_change = " << info.total_count_change << std::endl;
    std::cout << "    current_count = " << info.current_count << std::endl;
    std::cout << "    current_count_change = " << info.current_count_change << std::endl;
    std::cout << "    last_publication_handle = " << info.last_publication_handle << std::endl;

    (void)reader; // Suppress unused parameter warning

    if (info.current_count_change == 1)
    {
        std::cout << "[INFO] Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "[INFO] Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << "[INFO] Subscriber match status changed." << std::endl;
    }

    std::cout << "[DEBUG] Exiting SubListener::on_subscription_matched()" << std::endl;
}

void dloggerSubscriber::run()
{
    std::cout << "[DEBUG] Entering dloggerSubscriber::run()" << std::endl;
    std::cout << "[DEBUG] Thread ID = " << std::this_thread::get_id() << std::endl;

    std::cout << "Waiting for Data, press Enter to stop the DataReader. " << std::endl;
    std::cout << "[DEBUG] Blocking on std::cin.ignore() (waiting for user input)." << std::endl;

    std::cin.ignore();

    std::cout << "[DEBUG] User pressed Enter, continuing execution." << std::endl;
    std::cout << "Shutting down the Subscriber." << std::endl;

    std::cout << "[DEBUG] Exiting dloggerSubscriber::run()" << std::endl;
}
