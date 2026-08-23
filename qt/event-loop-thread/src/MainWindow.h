#pragma once

#include "BookingAgent.h"
#include "BoxOffice.h"

#include <QElapsedTimer>
#include <QList>
#include <QMainWindow>

class QLabel;
class QPushButton;
class QSlider;
class QThread;
class StadiumView;

// The mediator. It is the only class that knows about the office, the outlets
// and the views at once, and the only one that starts threads. Everything it
// wires is a queued connection, because the two ends live on different
// threads: that is the whole architecture in one constructor.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void pauseWhenSold(int seats);
    void runBurstBenchmark();

signals:
    void paused();

private slots:
    void onProgress(quint64 handled, int soldSeats);
    void onTally(BookingAgent::Tally tally);
    void onPaceChanged(int percent);
    void onBurstToggled(bool on);
    void onReopen();

private:
    void     createViews();
    void     createBoard();
    QWidget* createControls();
    void     startOffice();
    void     startOutlets();
    void     stopThreads();
    void     refreshBoard();
    QString  boardText() const;
    QString  statusText() const;
    void     freeze();

    // the views on the window
    StadiumView* m_view   = nullptr;
    QLabel*      m_board  = nullptr;
    QLabel*      m_status = nullptr;

    // the controls the user drives
    QSlider*     m_pace  = nullptr;
    QPushButton* m_burst = nullptr;

    // the office and the ten outlets, each behind its own thread
    BoxOffice*           m_office       = nullptr;
    QThread*             m_officeThread = nullptr;
    QList<BookingAgent*> m_outlets;
    QList<QThread*>      m_outletThreads;

    // the live figures the board is drawn from
    QList<BookingAgent::Tally> m_tallies;
    QElapsedTimer              m_clock;
    quint64                    m_handled   = 0;
    int                        m_soldSeats = 0;

    // screenshot and benchmark control
    int  m_pauseAt = 0;
    bool m_frozen  = false;
};
