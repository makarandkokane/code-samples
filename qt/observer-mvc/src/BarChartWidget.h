#pragma once

#include <QList>
#include <QWidget>

class ItemModel;
struct Item;
class QPainter;

// A "view" with no table in sight: it observes the same model the
// QTableView edits and repaints itself on every change signal.
class BarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarChartWidget(const ItemModel* model, QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    // Everything the row loop needs, computed once per paint.
    struct ChartLayout
    {
        int nameWidth;
        int barSpace;
        int valueWidth;
        int rowContentHeight;
        int maxValue;
    };

    ChartLayout computeLayout(const QList<Item>& items) const;
    void        drawNoItemsHint(QPainter& painter);
    void        drawNoModelHint(QPainter& painter);
    void        drawItemRow(QPainter& painter, const Item& item, const ChartLayout& layout, int y);
    void        forgetModel();

    // the observed subject; cleared, never dereferenced, once destroyed() fires
    const ItemModel* m_model = nullptr;
};
