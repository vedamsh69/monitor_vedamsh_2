// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.

#include "indedds-monitor/match/MatchAnalysesModel.h"
#include "indedds-monitor/match/MatchAnalysesEngine.h"
#include "indedds-monitor/backend/SyncBackendConnection.h"
#include <QDebug>

MatchAnalysesModel::MatchAnalysesModel(backend::SyncBackendConnection* backend, QObject* parent)
    : QAbstractItemModel(parent),
      engine_(std::make_unique<MatchAnalysesEngine>(backend)),
      root_node_(nullptr),
      filter_mismatches_only_(false)
{
    root_node_ = new TreeNode("Root", true);
    build_tree();
}

MatchAnalysesModel::~MatchAnalysesModel()
{
    clear_tree();
    if (root_node_) {
        delete root_node_;
    }
}

QModelIndex MatchAnalysesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeNode* parent_node;
    if (!parent.isValid()) {
        parent_node = root_node_;
    } else {
        parent_node = static_cast<TreeNode*>(parent.internalPointer());
    }

    if (row < parent_node->children.size()) {
        TreeNode* child_node = parent_node->children.at(row);
        return createIndex(row, column, child_node);
    }

    return QModelIndex();
}

QModelIndex MatchAnalysesModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeNode* child_node = static_cast<TreeNode*>(index.internalPointer());
    TreeNode* parent_node = child_node->parent_node;

    if (parent_node == root_node_ || !parent_node) {
        return QModelIndex();
    }

    // Find parent's row
    if (parent_node->parent_node) {
        int row = parent_node->parent_node->children.indexOf(parent_node);
        return createIndex(row, 0, parent_node);
    }

    return QModelIndex();
}

int MatchAnalysesModel::rowCount(const QModelIndex& parent) const
{
    TreeNode* parent_node;
    if (!parent.isValid()) {
        parent_node = root_node_;
    } else {
        parent_node = static_cast<TreeNode*>(parent.internalPointer());
    }

    return parent_node->children.size();
}

int MatchAnalysesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;  // All data returned via roles
}

QVariant MatchAnalysesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());

    switch (role) {
        case NameRole:
            return node->name;
        case OfferedRole:
            return node->offered;
        case RequestedRole:
            return node->requested;
        case NotesRole:
            return node->notes;
        case IsMatchedRole:
            return node->is_matched;
        case IsParentRole:
            return node->is_parent;
        case Qt::DisplayRole:
            return node->name;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> MatchAnalysesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[OfferedRole] = "offered";
    roles[RequestedRole] = "requested";
    roles[NotesRole] = "notes";
    roles[IsMatchedRole] = "is_matched";
    roles[IsParentRole] = "is_parent";
    return roles;
}

void MatchAnalysesModel::refresh()
{
    beginResetModel();
    build_tree();
    endResetModel();
}

void MatchAnalysesModel::set_filter_mismatches_only(bool filter)
{
    if (filter_mismatches_only_ != filter) {
        filter_mismatches_only_ = filter;
        beginResetModel();
        build_tree();
        endResetModel();
    }
}

bool MatchAnalysesModel::all_pairs_match() const
{
    for (const TreeNode* node : all_nodes_flat_) {
        if (node->is_parent && !node->is_matched) {
            return false;
        }
    }
    return true;
}

void MatchAnalysesModel::build_tree()
{
    clear_tree();
    all_nodes_flat_.clear();

    if (!engine_) {
        return;
    }

    // Get all writer-reader pairs
    auto pairs = engine_->get_writer_reader_pairs();

    for (auto& pair : pairs) {
        backend::EntityId writer_id = pair.first;
        for (backend::EntityId reader_id : pair.second) {
            // Convert EntityId to int for storage
            int writer_id_int = static_cast<int>(writer_id.value());
            int reader_id_int = static_cast<int>(reader_id.value());
            
            // Analyze this pair
            auto results = engine_->analyze_pair(writer_id, reader_id);

            // Check if all policies match
            bool pair_matches = true;
            for (const auto& result : results) {
                if (!result.is_compatible) {
                    pair_matches = false;
                    break;
                }
            }

            // Skip if filtering mismatches and this pair matches
            if (filter_mismatches_only_ && pair_matches) {
                continue;
            }

            // Create parent node for this pair
            QString pair_name = QString("DataWriter_%1 <-> DataReader_%2").arg(writer_id_int).arg(reader_id_int);
            TreeNode* pair_node = new TreeNode(pair_name, true, writer_id_int, reader_id_int);
            pair_node->parent_node = root_node_;
            pair_node->is_matched = pair_matches;
            pair_node->notes = pair_matches ? "✓ All match" : "✗ Mismatches found";

            // Add child nodes for each QoS policy
            for (const auto& result : results) {
                TreeNode* policy_node = new TreeNode(result.policy_name, false, writer_id_int, reader_id_int);
                policy_node->parent_node = pair_node;
                policy_node->offered = result.offered;
                policy_node->requested = result.requested;
                policy_node->is_matched = result.is_compatible;
                policy_node->notes = result.is_compatible ? "✓ OK" : "✗ Mismatch";

                pair_node->children.append(policy_node);
                all_nodes_flat_.append(policy_node);
            }

            root_node_->children.append(pair_node);
            all_nodes_flat_.append(pair_node);
        }
    }
}

void MatchAnalysesModel::clear_tree()
{
    qDeleteAll(root_node_->children);
    root_node_->children.clear();
}

void MatchAnalysesModel::add_nodes_to_flat_list(TreeNode* node)
{
    if (!node) {
        return;
    }

    all_nodes_flat_.append(node);
    for (TreeNode* child : node->children) {
        add_nodes_to_flat_list(child);
    }
}
