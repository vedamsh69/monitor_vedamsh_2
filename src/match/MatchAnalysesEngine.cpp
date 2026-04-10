// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "indedds-monitor/match/MatchAnalysesEngine.h"
#include "indedds-monitor/backend/SyncBackendConnection.h"
#include "indedds-monitor/backend/backend_utils.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

MatchAnalysesEngine::MatchAnalysesEngine(backend::SyncBackendConnection* backend)
    : backend_(backend)
{
}

QList<QoSComparisonResult> MatchAnalysesEngine::analyze_all()
{
    QList<QoSComparisonResult> results;
    
    if (!backend_) {
        return results;
    }
    
    auto pairs = get_writer_reader_pairs();
    
    for (auto it = pairs.begin(); it != pairs.end(); ++it) {
        backend::EntityId writer_id = it->first;
        for (backend::EntityId reader_id : it->second) {
            auto pair_results = analyze_pair(writer_id, reader_id);
            results.append(pair_results);
        }
    }
    
    return results;
}

QList<QoSComparisonResult> MatchAnalysesEngine::analyze_pair(backend::EntityId writer_id, backend::EntityId reader_id)
{
    QList<QoSComparisonResult> results;
    
    if (!backend_) {
        return results;
    }
    
    // Get QoS info for writer and reader
    auto writer_info = backend_->get_info(writer_id);
    auto reader_info = backend_->get_info(reader_id);
    
    // Compare all 9 QoS policies
    results.append(compare_data_representation(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_deadline(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_durability(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_ownership(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_liveliness(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_partition(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_presentation(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_reliability(writer_info, reader_info, writer_id, reader_id));
    results.append(compare_data_type(writer_info, reader_info, writer_id, reader_id));
    
    return results;
}

std::vector<std::pair<backend::EntityId, std::vector<backend::EntityId>>> MatchAnalysesEngine::get_writer_reader_pairs()
{
    std::vector<std::pair<backend::EntityId, std::vector<backend::EntityId>>> pairs;
    
    if (!backend_) {
        return pairs;
    }
    
    // Get all DataWriter and DataReader entities
    auto writers = backend_->get_entities(backend::EntityKind::DATAWRITER);
    auto readers = backend_->get_entities(backend::EntityKind::DATAREADER);
    
    // Group by topic name
    std::map<std::string, std::vector<backend::EntityId>> writer_topics;  // topic_name -> [writer_ids]
    std::map<std::string, std::vector<backend::EntityId>> reader_topics;  // topic_name -> [reader_ids]
    
    // Collect writers by topic
    for (backend::EntityId writer_id : writers) {
        auto writer_info = backend_->get_info(writer_id);
        std::string topic_name = backend::get_info_value(writer_info, "topic_name");
        if (!topic_name.empty()) {
            writer_topics[topic_name].push_back(writer_id);
        }
    }
    
    // Collect readers by topic
    for (backend::EntityId reader_id : readers) {
        auto reader_info = backend_->get_info(reader_id);
        std::string topic_name = backend::get_info_value(reader_info, "topic_name");
        if (!topic_name.empty()) {
            reader_topics[topic_name].push_back(reader_id);
        }
    }
    
    // Match writers and readers on same topics
    for (auto& topic_pair : writer_topics) {
        std::string topic_name = topic_pair.first;
        if (reader_topics.find(topic_name) != reader_topics.end()) {
            for (backend::EntityId writer_id : topic_pair.second) {
                std::vector<backend::EntityId> matching_readers = reader_topics[topic_name];
                pairs.push_back({writer_id, matching_readers});
            }
        }
    }
    
    return pairs;
}

// ========================
// QoS Comparison Functions
// ========================

QoSComparisonResult MatchAnalysesEngine::compare_data_representation(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "DataRepresentation";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_rep = backend::get_info_value(writer_info, "qos/representation/kind");
    std::string reader_rep = backend::get_info_value(reader_info, "qos/representation/kind");

    result.offered = QString::fromStdString(writer_rep.empty() ? "DEFAULT" : writer_rep);
    result.requested = QString::fromStdString(reader_rep.empty() ? "DEFAULT" : reader_rep);

    // Reader must support writer's representation or be XCDR/XCDR2
    result.is_compatible = (writer_rep == reader_rep || reader_rep == "XCDR" || reader_rep == "XCDR2");
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_deadline(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Deadline.period";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_deadline_str = backend::get_info_value(writer_info, "qos/deadline/period");
    std::string reader_deadline_str = backend::get_info_value(reader_info, "qos/deadline/period");

    result.offered = QString::fromStdString(writer_deadline_str.empty() ? "INFINITE" : writer_deadline_str);
    result.requested = QString::fromStdString(reader_deadline_str.empty() ? "INFINITE" : reader_deadline_str);

    // Compare durations: reader deadline >= writer deadline
    long long writer_ns = 0, reader_ns = 0;
    parse_duration(result.offered, writer_ns);
    parse_duration(result.requested, reader_ns);
    
    result.is_compatible = (reader_ns >= writer_ns || reader_ns == 0);  // 0 = INFINITE
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_durability(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Durability.kind";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_dur = backend::get_info_value(writer_info, "qos/durability/kind");
    std::string reader_dur = backend::get_info_value(reader_info, "qos/durability/kind");

    result.offered = QString::fromStdString(writer_dur.empty() ? "VOLATILE" : writer_dur);
    result.requested = QString::fromStdString(reader_dur.empty() ? "VOLATILE" : reader_dur);

    // Durability hierarchy: VOLATILE(0) < TRANSIENT_LOCAL(1) < TRANSIENT(2) < PERSISTENT(3)
    std::map<std::string, int> durability_levels{
        {"VOLATILE", 0}, {"TRANSIENT_LOCAL", 1}, {"TRANSIENT", 2}, {"PERSISTENT", 3}
    };
    
    int writer_level = durability_levels.count(writer_dur) ? durability_levels[writer_dur] : 0;
    int reader_level = durability_levels.count(reader_dur) ? durability_levels[reader_dur] : 0;
    
    result.is_compatible = (reader_level >= writer_level);
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_ownership(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Ownership.kind";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_own = backend::get_info_value(writer_info, "qos/ownership/kind");
    std::string reader_own = backend::get_info_value(reader_info, "qos/ownership/kind");

    result.offered = QString::fromStdString(writer_own.empty() ? "SHARED" : writer_own);
    result.requested = QString::fromStdString(reader_own.empty() ? "SHARED" : reader_own);

    // Must match exactly
    result.is_compatible = (writer_own == reader_own);
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_liveliness(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Liveliness.kind";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_live = backend::get_info_value(writer_info, "qos/liveliness/kind");
    std::string reader_live = backend::get_info_value(reader_info, "qos/liveliness/kind");
    
    std::string writer_lease = backend::get_info_value(writer_info, "qos/liveliness/lease_duration");
    std::string reader_lease = backend::get_info_value(reader_info, "qos/liveliness/lease_duration");

    result.offered = QString::fromStdString(writer_live.empty() ? "AUTOMATIC" : writer_live);
    result.requested = QString::fromStdString(reader_live.empty() ? "AUTOMATIC" : reader_live);

    // Kind must match
    bool kind_matches = (writer_live == reader_live);
    
    // Reader lease >= writer lease
    long long writer_lease_ns = 0, reader_lease_ns = 0;
    parse_duration(QString::fromStdString(writer_lease), writer_lease_ns);
    parse_duration(QString::fromStdString(reader_lease), reader_lease_ns);
    
    result.is_compatible = kind_matches && (reader_lease_ns >= writer_lease_ns || reader_lease_ns == 0);
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_partition(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Partition.name";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_part = backend::get_info_value(writer_info, "qos/partition/name");
    std::string reader_part = backend::get_info_value(reader_info, "qos/partition/name");

    result.offered = QString::fromStdString(writer_part.empty() ? "" : writer_part);
    result.requested = QString::fromStdString(reader_part.empty() ? "" : reader_part);

    // Partition set intersection (default partition "*" matches all)
    result.is_compatible = (writer_part == reader_part) || (writer_part.empty() && reader_part.empty()) ||
                          (writer_part == "*") || (reader_part == "*");
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_presentation(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Presentation.access_scope";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_pres = backend::get_info_value(writer_info, "qos/presentation/access_scope");
    std::string reader_pres = backend::get_info_value(reader_info, "qos/presentation/access_scope");

    result.offered = QString::fromStdString(writer_pres.empty() ? "INSTANCE" : writer_pres);
    result.requested = QString::fromStdString(reader_pres.empty() ? "INSTANCE" : reader_pres);

    // Must match exactly
    result.is_compatible = (writer_pres == reader_pres);
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_reliability(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Reliability.kind";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_rel = backend::get_info_value(writer_info, "qos/reliability/kind");
    std::string reader_rel = backend::get_info_value(reader_info, "qos/reliability/kind");

    result.offered = QString::fromStdString(writer_rel.empty() ? "BEST_EFFORT" : writer_rel);
    result.requested = QString::fromStdString(reader_rel.empty() ? "BEST_EFFORT" : reader_rel);

    // Reliability hierarchy: BEST_EFFORT(0) < RELIABLE(1)
    // Reader reliability >= writer reliability
    std::map<std::string, int> reliability_levels{
        {"BEST_EFFORT", 0}, {"RELIABLE", 1}
    };
    
    int writer_level = reliability_levels.count(writer_rel) ? reliability_levels[writer_rel] : 0;
    int reader_level = reliability_levels.count(reader_rel) ? reliability_levels[reader_rel] : 0;
    
    result.is_compatible = (reader_level >= writer_level);
    
    return result;
}

QoSComparisonResult MatchAnalysesEngine::compare_data_type(
    const backend::EntityInfo& writer_info,
    const backend::EntityInfo& reader_info,
    backend::EntityId writer_id,
    backend::EntityId reader_id)
{
    QoSComparisonResult result;
    result.policy_name = "Data Type";
    result.entity_id = static_cast<int>(writer_id.value());
    result.matching_reader_id = static_cast<int>(reader_id.value());

    std::string writer_type = backend::get_info_value(writer_info, "type_name");
    std::string reader_type = backend::get_info_value(reader_info, "type_name");

    result.offered = QString::fromStdString(writer_type);
    result.requested = QString::fromStdString(reader_type);

    // Type names must match exactly
    result.is_compatible = (writer_type == reader_type);
    
    return result;
}

// Helper: Parse ISO 8601 duration or nanosecond integer to nanoseconds
void MatchAnalysesEngine::parse_duration(const QString& duration_str, long long& nanoseconds)
{
    if (duration_str.isEmpty() || duration_str == "INFINITE") {
        nanoseconds = 0;  // INFINITE
        return;
    }
    
    try {
        // Try to parse as integer nanoseconds
        bool ok = false;
        long long value = duration_str.toLongLong(&ok);
        if (ok) {
            nanoseconds = value;
            return;
        }
    } catch (...) {}
    
    // Try to parse as ISO 8601 duration (e.g., "PT1S", "PT500MS")
    QString dur = duration_str.trimmed();
    if (dur.startsWith("PT")) {
        dur = dur.mid(2);  // Remove "PT"
        
        long long total_ns = 0;
        
        // Simple parsing: look for H, M, S, MS, NS suffixes
        int i = 0;
        while (i < dur.length()) {
            int start = i;
            while (i < dur.length() && (dur[i].isDigit() || dur[i] == '.')) {
                i++;
            }
            
            if (i > start && i < dur.length()) {
                double value = dur.mid(start, i - start).toDouble();
                char unit = dur[i].toLatin1();
                i++;
                
                // Check for multi-char units like MS, NS
                char unit2 = 0;
                if (i < dur.length()) {
                    unit2 = dur[i].toLatin1();
                    if ((unit == 'M' && unit2 == 'S') || (unit == 'N' && unit2 == 'S')) {
                        i++;
                    } else {
                        unit2 = 0;
                    }
                }
                
                if (unit == 'H') {
                    total_ns += (long long)(value * 3600000000000LL);  // Hours to ns
                } else if (unit == 'M' && unit2 != 'S') {
                    total_ns += (long long)(value * 60000000000LL);  // Minutes to ns
                } else if (unit == 'S') {
                    total_ns += (long long)(value * 1000000000LL);  // Seconds to ns
                } else if (unit == 'M' && unit2 == 'S') {
                    total_ns += (long long)(value * 1000000LL);  // Milliseconds to ns
                } else if (unit == 'N') {
                    total_ns += (long long)value;  // Nanoseconds
                }
            }
        }
        
        nanoseconds = total_ns;
    }
}
