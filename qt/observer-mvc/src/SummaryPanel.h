#pragma once

#include <QWidget>

class ItemModel;
class QLabel;

// Third observer of the same model: a one-line aggregate view.
class SummaryPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SummaryPanel(const ItemModel* model, QWidget* parent = nullptr);

private:
    void rebuildText();
    void forgetModel();

    const ItemModel* m_model = nullptr;
    QLabel* m_label = nullptr;
};
