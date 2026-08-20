#include "ValueDelegate.h"

#include <QSpinBox>

namespace
{
constexpr int kMinValue = 0;
constexpr int kMaxValue = 9999;
}

ValueDelegate::ValueDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);

    // The base class already supplies an integer-only spin box; take its arrows
    // away and widen the range, whose default stops at 99.
    auto* spinBox = qobject_cast<QSpinBox*>(editor);
    if (spinBox)
    {
        spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        spinBox->setRange(kMinValue, kMaxValue);
    }

    return editor;
}
