#include "BarChartWidget.h"

#include "ItemModel.h"

#include <QPainter>

namespace
{
constexpr int kRowHeight           = 30;
constexpr int kMargin              = 12;
constexpr int kGap                 = 10;
constexpr int kMinChartWidth       = 260;
constexpr int kNameSlack           = 6;
constexpr int kNameWidthFraction   = 3;
constexpr int kRowVerticalPadding  = 10;
constexpr int kValueDigits         = 4;
constexpr int kValuePadding        = 8;
constexpr int kValueGap            = 6;
constexpr int kMinBarSpace         = 20;
constexpr int kMinBarWidth         = 2;
constexpr int kBarCornerRadius     = 3;
constexpr int kMinScaleDenominator = 1;
}

// Wires the five subscriptions: four repaint triggers, plus the destruction
// notice that turns this widget into its explicit gone state.
BarChartWidget::BarChartWidget(const ItemModel* model, QWidget* parent)
    : QWidget(parent),
      m_model(model)
{
    auto refresh = [this]
    {
        updateGeometry();
        update();
    };

    // Subscribe to the subject: it announces change, this view pulls the data.
    connect(model, &QAbstractItemModel::dataChanged, this, refresh);
    connect(model, &QAbstractItemModel::rowsInserted, this, refresh);
    connect(model, &QAbstractItemModel::rowsRemoved, this, refresh);
    connect(model, &QAbstractItemModel::modelReset, this, refresh);
    connect(model, &QObject::destroyed, this, &BarChartWidget::forgetModel);
}

// The subject is gone: drop the pointer and repaint as the gone state.
void BarChartWidget::forgetModel()
{
    // destroyed() is emitted from ~QObject, once ~ItemModel has already run: the
    // pointer may only be cleared here, never dereferenced.
    m_model = nullptr;
    updateGeometry();
    update();
}

// Tall enough for every row, so the chart never clips items away.
QSize BarChartWidget::minimumSizeHint() const
{
    int itemCount = 0;
    if (m_model)
        itemCount = int(m_model->items().size());

    const int neededHeight = kMargin * 2 + itemCount * kRowHeight;
    return QSize(kMinChartWidth, neededHeight);
}

// Chooses between the gone, empty and normal states; the layout is computed
// once per paint, then the row loop only draws.
void BarChartWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_model)
    {
        drawNoModelHint(painter);
        return;
    }

    const QList<Item>& items = m_model->items();
    if (items.isEmpty())
    {
        drawNoItemsHint(painter);
        return;
    }

    const ChartLayout layout = computeLayout(items);
    int               y      = kMargin;
    for (const Item& item : items)
    {
        drawItemRow(painter, item, layout, y);
        y += kRowHeight;
    }
}

// Measures the three columns (name, bars, value) against the current font and
// items, and returns them as one ChartLayout for the row loop.
BarChartWidget::ChartLayout BarChartWidget::computeLayout(const QList<Item>& items) const
{
    const QFontMetrics fm(font());

    // Name column: as wide as the longest name, capped so the bars keep their space.
    int widestName = 0;
    for (const Item& item : items)
        widestName = qMax(widestName, fm.horizontalAdvance(item.name));
    const int maxNameWidth = width() / kNameWidthFraction;

    ChartLayout layout;
    layout.nameWidth        = qMin(widestName + kNameSlack, maxNameWidth); // slack avoids elision
    layout.rowContentHeight = kRowHeight - kRowVerticalPadding;

    const QString widestValue(kValueDigits, QLatin1Char('9'));
    layout.valueWidth = fm.horizontalAdvance(widestValue) + kValuePadding;

    const int fixedWidth = kMargin * 2 + layout.nameWidth + kGap + layout.valueWidth;
    layout.barSpace      = qMax(kMinBarSpace, width() - fixedWidth);

    layout.maxValue = kMinScaleDenominator;
    for (const Item& item : items)
        layout.maxValue = qMax(layout.maxValue, item.value);

    return layout;
}

// The empty state, worded differently from the gone state on purpose.
void BarChartWidget::drawNoItemsHint(QPainter& painter)
{
    painter.setPen(palette().color(QPalette::PlaceholderText));
    painter.drawText(rect(), Qt::AlignCenter, tr("No items"));
}

// The gone state: the subject was deleted, which is not the same as empty.
void BarChartWidget::drawNoModelHint(QPainter& painter)
{
    painter.setPen(palette().color(QPalette::PlaceholderText));
    painter.drawText(rect(), Qt::AlignCenter, tr("Model deleted"));
}

// One row at height y: elided name, proportional bar, plain value.
void BarChartWidget::drawItemRow(QPainter& painter, const Item& item, const ChartLayout& layout,
                                 int y)
{
    const QFontMetrics fm(font());
    const QColor       textColor = palette().color(QPalette::WindowText);

    // The name at the left, elided if the column is too narrow to hold it.
    const QRect   nameRect(kMargin, y, layout.nameWidth, layout.rowContentHeight);
    const QString elidedName = fm.elidedText(item.name, Qt::ElideRight, nameRect.width());
    painter.setPen(textColor);
    painter.drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, elidedName);

    // The bar itself, its length proportional to the value.
    const int   barLeft  = kMargin + layout.nameWidth + kGap;
    const int   barWidth = qMax(kMinBarWidth, layout.barSpace * item.value / layout.maxValue);
    const QRect barRect(barLeft, y, barWidth, layout.rowContentHeight);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Highlight));
    painter.drawRoundedRect(barRect, kBarCornerRadius, kBarCornerRadius);

    // The value in plain digits, just past the end of the bar.
    const QRect valueRect(barRect.right() + kValueGap, y, layout.valueWidth,
                          layout.rowContentHeight);
    painter.setPen(textColor);
    painter.drawText(valueRect, Qt::AlignVCenter | Qt::AlignLeft, QString::number(item.value));
}
