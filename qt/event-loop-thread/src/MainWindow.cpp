#include "MainWindow.h"

#include "StadiumView.h"

#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
// The ten outlets a stadium actually sells through. Their profiles differ on
// purpose: a group booking of 400 has to queue behind singles and either fits
// in one block or comes back sold out.
const BookingAgent::Profile kOutlets[] = {
    {QStringLiteral("Website"), 2, 2, 420, BoxOffice::Block::North, false},
    {QStringLiteral("Mobile App"), 1, 2, 620, BoxOffice::Block::North, false},
    {QStringLiteral("Call Center"), 2, 4, 45, BoxOffice::Block::North, false},
    {QStringLiteral("Walk-up Window"), 1, 2, 25, BoxOffice::Block::South, true},
    {QStringLiteral("On-site Kiosk"), 1, 2, 30, BoxOffice::Block::South, true},
    {QStringLiteral("Season Renewals"), 10, 50, 12, BoxOffice::Block::West, true},
    {QStringLiteral("Group Sales"), 100, 400, 2, BoxOffice::Block::East, false},
    {QStringLiteral("Corporate Hospitality"), 20, 40, 3, BoxOffice::Block::NorthEast, true},
    {QStringLiteral("Resale Partner"), 1, 4, 160, BoxOffice::Block::North, false},
    {QStringLiteral("Travel Package"), 4, 8, 40, BoxOffice::Block::SouthWest, true},
};

constexpr int kOutletCount    = int(std::end(kOutlets) - std::begin(kOutlets));
constexpr int kBoardRefreshMs = 200;
constexpr int kPaceMinimum    = 0;
constexpr int kPaceMaximum    = 400;
constexpr int kPaceStart      = 100;
constexpr int kPaceTick       = 50;
constexpr int kBoardPointSize = 9;
constexpr int kNameColumn     = 22;
constexpr int kMsPerSecond    = 1000;
}

// Wires the whole demo in order: the views, then the office thread, then the
// ten outlet threads that sell into it.
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    createViews();
    startOffice();
    startOutlets();

    m_clock.start();
    refreshBoard();
}

// Stops the office before the outlets; stopThreads carries the why.
MainWindow::~MainWindow()
{
    stopThreads();
}

// Arms the screenshot pause: the run freezes once this many seats are sold.
void MainWindow::pauseWhenSold(int seats)
{
    m_pauseAt = seats;
}

// Fills the ground as fast as the office can take it and stops on the last
// seat, so the throughput figure comes from a run nobody had to click through.
void MainWindow::runBurstBenchmark()
{
    m_pauseAt = BoxOffice::totalSeats();
    m_burst->setChecked(true);
}

// The office heartbeat: takes the fresh totals and freezes the run at the
// armed pause point, or on sell-out.
void MainWindow::onProgress(quint64 handled, int soldSeats)
{
    m_handled   = handled;
    m_soldSeats = soldSeats;

    if (m_pauseAt > 0 && !m_frozen && m_soldSeats >= m_pauseAt)
    {
        freeze();
        refreshBoard();
        emit paused();
        return;
    }

    if (m_soldSeats >= BoxOffice::totalSeats() && !m_frozen)
        freeze();
}

// One outlet's fresh counters, stored for the next board refresh.
void MainWindow::onTally(BookingAgent::Tally tally)
{
    if (tally.outlet >= 0 && tally.outlet < m_tallies.size())
        m_tallies[tally.outlet] = tally;
}

// Fans the slider value out to every outlet.
void MainWindow::onPaceChanged(int percent)
{
    for (BookingAgent* outlet : m_outlets)
    {
        QMetaObject::invokeMethod(outlet, "setPacePercent", Qt::QueuedConnection,
                                  Q_ARG(int, percent));
    }
}

// Switches every outlet between paced and flat-out selling; a fresh burst
// restarts the sell-out clock.
void MainWindow::onBurstToggled(bool on)
{
    for (BookingAgent* outlet : m_outlets)
    {
        QMetaObject::invokeMethod(outlet, "setBurst", Qt::QueuedConnection, Q_ARG(bool, on));
    }

    if (on)
        m_clock.restart();
}

// Puts everything back on sale: office ledger, outlet counters, view, clock.
void MainWindow::onReopen()
{
    m_frozen    = false;
    m_handled   = 0;
    m_soldSeats = 0;

    QMetaObject::invokeMethod(m_office, "reopen", Qt::QueuedConnection);
    for (BookingAgent* outlet : m_outlets)
    {
        QMetaObject::invokeMethod(outlet, "reset", Qt::QueuedConnection);
    }

    m_view->clearSeats();
    m_clock.restart();
    onPaceChanged(m_pace->value());
}

// Assembles the window: stadium on the left, board on the right, controls
// underneath, and the timer that keeps the board fresh.
void MainWindow::createViews()
{
    m_view = new StadiumView;
    createBoard();
    QWidget* controls = createControls();

    QWidget*     right  = new QWidget;
    QVBoxLayout* column = new QVBoxLayout(right);
    column->addWidget(m_board);
    column->addWidget(m_status);
    column->addStretch();

    QWidget*     central = new QWidget;
    QGridLayout* layout  = new QGridLayout(central);
    layout->addWidget(m_view, 0, 0);
    layout->addWidget(right, 0, 1);
    layout->addWidget(controls, 1, 0, 1, 2);
    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 2);
    setCentralWidget(central);

    setWindowTitle(QStringLiteral("Stadium box office: ten outlets, one event loop, no locks"));

    QTimer* boardTimer = new QTimer(this);
    boardTimer->setInterval(kBoardRefreshMs);
    connect(boardTimer, &QTimer::timeout, this, &MainWindow::refreshBoard);
    boardTimer->start();
}

// The two monospace labels the figures land on: the per-outlet table and the
// one-line totals under it.
void MainWindow::createBoard()
{
    QFont mono = QFont(QStringLiteral("Consolas"));
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(kBoardPointSize);

    m_board = new QLabel;
    m_board->setFont(mono);
    m_board->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_status = new QLabel;
    m_status->setFont(mono);
}

// The row of controls the user drives: pace slider, burst toggle, reopen.
// Returns the assembled row, ready for the central layout.
QWidget* MainWindow::createControls()
{
    m_pace = new QSlider(Qt::Horizontal);
    m_pace->setRange(kPaceMinimum, kPaceMaximum);
    m_pace->setValue(kPaceStart);
    m_pace->setTickInterval(kPaceTick);
    m_pace->setTickPosition(QSlider::TicksBelow);
    connect(m_pace, &QSlider::valueChanged, this, &MainWindow::onPaceChanged);

    m_burst = new QPushButton(QStringLiteral("Burst (unthrottled)"));
    m_burst->setCheckable(true);
    connect(m_burst, &QPushButton::toggled, this, &MainWindow::onBurstToggled);

    QPushButton* reopen = new QPushButton(QStringLiteral("Reopen sales"));
    connect(reopen, &QPushButton::clicked, this, &MainWindow::onReopen);

    QWidget*     controls = new QWidget;
    QGridLayout* grid     = new QGridLayout(controls);
    grid->addWidget(new QLabel(QStringLiteral("Pace")), 0, 0);
    grid->addWidget(m_pace, 0, 1);
    grid->addWidget(m_burst, 0, 2);
    grid->addWidget(reopen, 0, 3);

    return controls;
}

// The office moves to its own thread; open() runs there so its timer is born
// on that loop, and the sold runs flow straight to the view.
void MainWindow::startOffice()
{
    m_office       = new BoxOffice;
    m_officeThread = new QThread(this);
    m_office->moveToThread(m_officeThread);

    connect(m_officeThread, &QThread::started, m_office, &BoxOffice::open);
    connect(m_officeThread, &QThread::finished, m_office, &QObject::deleteLater);
    connect(m_office, &BoxOffice::soldRuns, m_view, &StadiumView::markSold);
    connect(m_office, &BoxOffice::progress, this, &MainWindow::onProgress);

    m_officeThread->start();
}

// Ten outlets, one thread each, all selling into the same office slot.
void MainWindow::startOutlets()
{
    for (int outlet = 0; outlet < kOutletCount; ++outlet)
    {
        BookingAgent* agent  = new BookingAgent(outlet, kOutlets[outlet]);
        QThread*      thread = new QThread(this);
        agent->moveToThread(thread);

        connect(thread, &QThread::started, agent, &BookingAgent::open);
        connect(thread, &QThread::finished, agent, &QObject::deleteLater);
        connect(agent, &BookingAgent::requestReady, m_office, &BoxOffice::submit);
        connect(agent, &BookingAgent::tallyChanged, this, &MainWindow::onTally);

        m_outlets.append(agent);
        m_outletThreads.append(thread);
        m_tallies.append(BookingAgent::Tally{});

        thread->start();
    }
}

// Order matters, and getting it wrong is a crash. Every request carries the
// return address of the outlet that sent it, so the office must stop before
// the outlets do: shut the outlets first and the office would still be
// draining its queue, posting confirmations to objects that no longer exist.
void MainWindow::stopThreads()
{
    m_officeThread->quit();
    m_officeThread->wait();

    for (QThread* thread : m_outletThreads)
    {
        thread->quit();
        thread->wait();
    }
}

// Repaints both labels from the stored figures.
void MainWindow::refreshBoard()
{
    m_board->setText(boardText());
    m_status->setText(statusText());
}

// Renders the per-outlet table: a header row, then one line of counters and
// round-trip figures per outlet.
QString MainWindow::boardText() const
{
    QString text = QStringLiteral("%1 %2 %3 %4 %5\n")
                       .arg(QStringLiteral("OUTLET"), -kNameColumn)
                       .arg(QStringLiteral("SENT"), 9)
                       .arg(QStringLiteral("SEATS"), 8)
                       .arg(QStringLiteral("SOLD OUT"), 9)
                       .arg(QStringLiteral("RTT us  best/mean/worst"), 24);

    for (int outlet = 0; outlet < m_outlets.size(); ++outlet)
    {
        const BookingAgent::Tally& tally = m_tallies.at(outlet);
        const QString              rtt   = QStringLiteral("%1 / %2 / %3")
                                .arg(tally.bestRttUs)
                                .arg(tally.meanRttUs)
                                .arg(tally.worstRttUs);

        text += QStringLiteral("%1 %2 %3 %4 %5\n")
                    .arg(m_outlets.at(outlet)->profile().name, -kNameColumn)
                    .arg(tally.sent, 9)
                    .arg(tally.seats, 8)
                    .arg(tally.soldOut, 9)
                    .arg(rtt, 24);
    }

    return text;
}

// The one-line totals: seats sold, requests handled, the in-flight backlog,
// and the sell-out throughput once the ground is full.
QString MainWindow::statusText() const
{
    // the backlog nobody shares: each outlet counts what it sent, the office
    // counts what it handled, and the difference arrives here by message
    quint64 sent = 0;
    for (const BookingAgent::Tally& tally : m_tallies)
    {
        sent += tally.sent;
    }

    const qint64 elapsedMs = m_clock.elapsed();
    const int    seatsPerSecond =
        elapsedMs > 0 ? int(qint64(m_soldSeats) * kMsPerSecond / elapsedMs) : 0;

    QString status = QStringLiteral("seats sold %1 of %2   requests handled %3   in flight %4\n")
                         .arg(m_soldSeats)
                         .arg(BoxOffice::totalSeats())
                         .arg(m_handled)
                         .arg(qint64(sent) - qint64(m_handled));

    if (m_soldSeats >= BoxOffice::totalSeats())
    {
        status +=
            QStringLiteral("SOLD OUT in %1 ms, %2 seats/s").arg(elapsedMs).arg(seatsPerSecond);
    }
    else
    {
        status += QStringLiteral("%1 seats/s   threads: 10 outlets + 1 office + this one")
                      .arg(seatsPerSecond);
    }

    return status;
}

// Stops all selling without touching the threads, so the picture on screen
// holds still for the screenshot.
void MainWindow::freeze()
{
    m_frozen = true;

    for (BookingAgent* outlet : m_outlets)
    {
        QMetaObject::invokeMethod(outlet, "setPacePercent", Qt::QueuedConnection, Q_ARG(int, 0));
        QMetaObject::invokeMethod(outlet, "setBurst", Qt::QueuedConnection, Q_ARG(bool, false));
    }
}
