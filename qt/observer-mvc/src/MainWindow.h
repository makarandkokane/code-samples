#pragma once

#include <QMainWindow>

class ItemModel;
class QAction;
class QTableView;

// The mediator: the only place that knows the model and all of its views.
// The views never reference each other.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void createViews();
    void createToolbar();
    void updateRemoveAction();

    ItemModel* m_model = nullptr;
    QTableView* m_table = nullptr;
    QAction* m_removeAction = nullptr;
};
