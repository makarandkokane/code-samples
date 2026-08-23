#pragma once

#include <QAbstractTableModel>
#include <QList>

struct Item
{
    QString name;
    int     value;
};

// The subject everyone observes. This model is the single source of truth;
// every view in the app subscribes to its change signals and nothing else.
class ItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        NameColumn  = 0,
        ValueColumn = 1,
        ColumnCount = 2
    };

    explicit ItemModel(QObject* parent = nullptr);

    int           rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex& index, int role) const override;
    bool          setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    const QList<Item>& items() const;
    int                total() const;
    int                maxValueRow() const; // -1 when empty

public slots:
    void addItem();
    void removeItem(int row);
    void resetToDefaults();

private:
    // the rows, and the serial the next new item is named after
    QList<Item> m_items;
    int         m_nextSerial = 1;
};
