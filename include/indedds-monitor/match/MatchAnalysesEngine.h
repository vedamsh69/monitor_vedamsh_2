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

#ifndef _INDEDDS_MONITOR_MATCH_MATCHANALYSESENGINE_H_
#define _INDEDDS_MONITOR_MATCH_MATCHANALYSESENGINE_H_

#include <QString>
#include <QList>
#include <vector>

#include "indedds-monitor/backend/backend_types.h"

namespace backend {
    class SyncBackendConnection;
}

/**
 * @brief Structure holding QoS comparison result for a single policy
 */
struct QoSComparisonResult
{
    QString policy_name;
    QString offered;      // DataWriter value
    QString requested;    // DataReader value
    bool is_compatible;
    backend::EntityId entity_id;        // DataWriter ID
    backend::EntityId matching_reader_id;
};

/**
 * @brief Main engine for analyzing QoS compatibility between DataWriter and DataReader
 *
 * This engine compares 9 key QoS policies and data type compatibility:
 * 1. DataRepresentation
 * 2. Deadline
 * 3. Durability
 * 4. Ownership
 * 5. Liveliness
 * 6. Partition
 * 7. Presentation
 * 8. Reliability
 * 9. Data Type
 */
class MatchAnalysesEngine
{
public:

    explicit MatchAnalysesEngine(backend::SyncBackendConnection* backend);

    ~MatchAnalysesEngine() = default;

    /**
     * @brief Analyze compatibility between all DataWriters and DataReaders
     * @return List of analysis results
     */
    QList<QoSComparisonResult> analyze_all();

    /**
     * @brief Analyze compatibility between specific DataWriter and DataReader
     * @param writer_id DataWriter entity ID
     * @param reader_id DataReader entity ID
     * @return List of comparison results for all 9 policies
     */
    QList<QoSComparisonResult> analyze_pair(backend::EntityId writer_id, backend::EntityId reader_id);

    /**
     * @brief Get all DataWriter and DataReader pairs by topic
     * @return Vector of (writer_id, [reader_ids]) pairs for same topic
     */
    std::vector<std::pair<backend::EntityId, std::vector<backend::EntityId>>> get_writer_reader_pairs();

private:

    backend::SyncBackendConnection* backend_;

    // Individual policy comparison functions
    QoSComparisonResult compare_data_representation(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_deadline(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_durability(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_ownership(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_liveliness(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_partition(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_presentation(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_reliability(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);
    QoSComparisonResult compare_data_type(
        const backend::EntityInfo& writer_info,
        const backend::EntityInfo& reader_info,
        backend::EntityId writer_id,
        backend::EntityId reader_id);

    // Helper function for duration parsing
    void parse_duration(const QString& duration_str, long long& nanoseconds);
};

#endif // _INDEDDS_MONITOR_MATCH_MATCHANALYSESENGINE_H_
