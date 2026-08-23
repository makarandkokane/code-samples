#include "ItemModel.h"

#include <QRandomGenerator>

namespace
{
constexpr int kMinNewValue = 5;
constexpr int kMaxNewValue = 60;
}

// Starts with the seed rows so every view has something to show at once.
ItemModel::ItemModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    resetToDefaults();
}

// Flat table: rows exist at the root only.
int ItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return int(m_items.size());
}

// Two columns, name and value.
int ItemModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return Column::ColumnCount;
}

// What each cell shows: the name or the value, values right-aligned.
QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const Item& item = m_items.at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (index.column() == Column::NameColumn)
            return item.name;

        return item.value;
    }

    if (role == Qt::TextAlignmentRole && index.column() == Column::ValueColumn)
        return int(Qt::AlignRight | Qt::AlignVCenter);

    return {};
}

// Edits from any view land here; blanks, negatives and no-ops are rejected,
// everything accepted is announced through dataChanged. Returns whether the
// model actually changed.
bool ItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || index.row() >= m_items.size())
        return false;

    Item& item = m_items[index.row()];
    if (index.column() == Column::NameColumn)
    {
        const QString name = value.toString().trimmed();
        if (name.isEmpty() || name == item.name)
            return false;

        item.name = name;
    }
    else
    {
        bool      ok = false;
        const int v  = value.toInt(&ok);
        if (!ok || v < 0 || v == item.value)
            return false;

        item.value = v;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

// Column captions for the table view.
QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section == Column::NameColumn)
        return tr("Name");

    return tr("Value");
}

// Every cell is editable in place.
Qt::ItemFlags ItemModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

// The rows, read-only: the pull side for every observer.
const QList<Item>& ItemModel::items() const
{
    return m_items;
}

// Sum of all values, for the summary line.
int ItemModel::total() const
{
    int sum = 0;
    for (const Item& item : m_items)
        sum += item.value;

    return sum;
}

// Row of the largest value, -1 when the model is empty.
int ItemModel::maxValueRow() const
{
    int best = -1;
    for (int i = 0; i < m_items.size(); ++i)
    {
        if (best < 0 || m_items.at(i).value > m_items.at(best).value)
            best = i;
    }

    return best;
}

// Appends a serial-named row with a random value, announced through
// begin/endInsertRows.
void ItemModel::addItem()
{
    const QString name  = tr("Item %1").arg(m_nextSerial);
    const int     value = int(QRandomGenerator::global()->bounded(kMinNewValue, kMaxNewValue));
    m_nextSerial++;

    const int row = int(m_items.size());
    beginInsertRows(QModelIndex(), row, row);
    m_items.append({name, value});
    endInsertRows();
}

// Removes one row by index, announced through begin/endRemoveRows.
void ItemModel::removeItem(int row)
{
    if (row < 0 || row >= m_items.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}

// Back to the three seed rows; the model reset tells every view to start over.
void ItemModel::resetToDefaults()
{
    beginResetModel();
    m_items.clear();
    m_items.append({QStringLiteral("Alpha"), 40});
    m_items.append({QStringLiteral("Beta"), 25});
    m_items.append({QStringLiteral("Gamma"), 35});
    m_nextSerial = int(m_items.size()) + 1;
    endResetModel();
}
