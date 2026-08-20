#include "MainWindow.h"

#include "BarChartWidget.h"
#include "ItemModel.h"
#include "SummaryPanel.h"
#include "ValueDelegate.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>

namespace
{
constexpr int kInitialWidth = 720;
constexpr int kInitialHeight = 400;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_model(new ItemModel(this))
{
    setWindowTitle(tr("Observer Demo"));
    createViews();
    createToolbar();
    resize(kInitialWidth, kInitialHeight);
}

void MainWindow::createViews()
{
    m_table = new QTableView(this);
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setSectionResizeMode(ItemModel::Column::NameColumn,
                                                      QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ItemModel::Column::ValueColumn,
                                                      QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setItemDelegateForColumn(ItemModel::Column::ValueColumn, new ValueDelegate(this));

    auto* chart = new BarChartWidget(m_model, this);
    auto* summary = new SummaryPanel(m_model, this);

    auto* splitter = new QSplitter(this);
    splitter->addWidget(m_table);
    splitter->addWidget(chart);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(splitter, 1);
    layout->addWidget(summary);
    setCentralWidget(central);
}

void MainWindow::createToolbar()
{
    auto* toolbar = addToolBar(tr("Actions"));
    toolbar->setMovable(false);

    toolbar->addAction(tr("Add item"), m_model, &ItemModel::addItem);

    m_removeAction = toolbar->addAction(tr("Remove selected"), this,
                                        [this]
                                        {
                                            const int selectedRow = m_table->currentIndex().row();
                                            m_model->removeItem(selectedRow);
                                        });

    toolbar->addAction(tr("Reset"), m_model, &ItemModel::resetToDefaults);

    // The toolbar is a view too: its enabled state observes selection and model.
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &MainWindow::updateRemoveAction);

    // A model reset drops the selection without emitting selectionChanged, so the
    // action would otherwise stay enabled with nothing selected.
    connect(m_model, &QAbstractItemModel::modelReset, this, &MainWindow::updateRemoveAction);

    updateRemoveAction();
}

void MainWindow::updateRemoveAction()
{
    m_removeAction->setEnabled(m_table->selectionModel()->hasSelection());
}
