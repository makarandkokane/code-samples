#pragma once

#include <QStyledItemDelegate>

// Editing aid for the value column. The stock editor is a spin box whose
// arrows crowd the digits out of so narrow a column, so this strips them
// and leaves plain typing, still integer only.
class ValueDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ValueDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
};
