#include "SummaryPanel.h"

#include "ItemModel.h"

#include <QHBoxLayout>
#include <QLabel>

namespace
{
constexpr int kHorizontalMargin = 12;
constexpr int kVerticalMargin   = 6;
}

// Builds the label and subscribes to the same five signals as every other
// observer of the model.
SummaryPanel::SummaryPanel(const ItemModel* model, QWidget* parent)
    : QWidget(parent),
      m_model(model),
      m_label(new QLabel(this))
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(kHorizontalMargin, kVerticalMargin, kHorizontalMargin,
                               kVerticalMargin);
    layout->addWidget(m_label);

    auto refresh = [this]
    {
        rebuildText();
    };

    // Subscribe to the subject: it announces change, this view pulls the data.
    connect(model, &QAbstractItemModel::dataChanged, this, refresh);
    connect(model, &QAbstractItemModel::rowsInserted, this, refresh);
    connect(model, &QAbstractItemModel::rowsRemoved, this, refresh);
    connect(model, &QAbstractItemModel::modelReset, this, refresh);
    connect(model, &QObject::destroyed, this, &SummaryPanel::forgetModel);

    rebuildText();
}

// The subject is gone: drop the pointer and switch to the gone-state text.
void SummaryPanel::forgetModel()
{
    // destroyed() is emitted from ~QObject, once ~ItemModel has already run: the
    // pointer may only be cleared here, never dereferenced.
    m_model = nullptr;
    rebuildText();
}

// Recomputes the one-line aggregate: count, total, and the largest item.
void SummaryPanel::rebuildText()
{
    if (!m_model)
    {
        m_label->setText(tr("Model deleted"));
        return;
    }

    const QList<Item>& items = m_model->items();
    if (items.isEmpty())
    {
        m_label->setText(tr("No items"));
        return;
    }

    const int     maxRow  = m_model->maxValueRow();
    const Item&   largest = items.at(maxRow);
    const QString text    = tr("%1 items, total %2, largest: %3 (%4)")
                             .arg(items.size())
                             .arg(m_model->total())
                             .arg(largest.name)
                             .arg(largest.value);

    m_label->setText(text);
}
