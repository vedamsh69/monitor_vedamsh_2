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

#ifndef _INDEDDS_MONITOR_MATCH_MATCHANALYSESMODEL_H_
#define _INDEDDS_MONITOR_MATCH_MATCHANALYSESMODEL_H_

#include <QAbstractItemModel>
#include <QList>
#include <memory>
#include "indedds-monitor/backend/backend_types.h"

struct QoSComparisonResult;
class MatchAnalysesEngine;

namespace backend {
    class SyncBackendConnection;
}

/**
 * @brief Qt TreeModel for displaying Match Analyses results hierarchically
 *
 * Structure:
 * - Parent: DataWriter->DataReader pair (e.g., "DataWriter_1 <-> DataReader_1")
 *   - Child: QoS Policy analysis (e.g., "DataRepresentation", "Deadline", etc)
 */
class MatchAnalysesModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum ModelRole
    {
        NameRole = Qt::UserRole + 1,
        OfferedRole = Qt::UserRole + 2,
        RequestedRole = Qt::UserRole + 3,
        NotesRole = Qt::UserRole + 4,
        IsMatchedRole = Qt::UserRole + 5,
        IsParentRole = Qt::UserRole + 6
    };

    explicit MatchAnalysesModel(backend::SyncBackendConnection* backend, QObject* parent = nullptr);

    ~MatchAnalysesModel();

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Refresh model data from backend
     */
    void refresh();

    /**
     * @brief Apply filter to show only mismatched pairs
     */
    void set_filter_mismatches_only(bool filter);

    /**
     * @brief Check if all pairs match
     */
    bool all_pairs_match() const;

private:

    // Internal tree node structure
    struct TreeNode
    {
        QString name;
        QString offered;
        QString requested;
        QString notes;
        bool is_matched;
        bool is_parent;
        backend::EntityId entity_id;  // For parent nodes: DataWriter ID
        backend::EntityId reader_id;  // For parent nodes: DataReader ID
        QList<TreeNode*> children;
        TreeNode* parent_node;

        TreeNode(const QString& n, bool parent, backend::EntityId eid = backend::EntityId::invalid(), backend::EntityId rid = backend::EntityId::invalid())
            : name(n), is_matched(true), is_parent(parent), entity_id(eid),
              reader_id(rid), parent_node(nullptr)
        {
        }

        ~TreeNode()
        {
            qDeleteAll(children);
        }
    };

    std::unique_ptr<MatchAnalysesEngine> engine_;
    TreeNode* root_node_;
    QList<TreeNode*> all_nodes_flat_;
    bool filter_mismatches_only_;

    void build_tree();
    void clear_tree();
    void add_nodes_to_flat_list(TreeNode* node);
};

#endif // _INDEDDS_MONITOR_MATCH_MATCHANALYSESMODEL_H_
