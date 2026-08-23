#include "BoxOffice.h"

#include <QMetaObject>
#include <QTimer>

namespace
{
constexpr int kSeatsPerBlock     = 12500;
constexpr int kPublishIntervalMs = 33;

const char* const kBlockNames[] = {"North", "North East", "East", "South East",
                                   "South", "South West", "West", "North West"};
}

// Construction stays trivial on purpose: the object is still on the creating
// thread here, so nothing loop-bound may start yet.
BoxOffice::BoxOffice(QObject* parent)
    : QObject(parent)
{
}

// Seats per block, the one capacity figure everything else derives from.
int BoxOffice::blockSeats()
{
    return kSeatsPerBlock;
}

// Capacity of the whole ground, derived so the figures cannot drift apart.
int BoxOffice::totalSeats()
{
    return kSeatsPerBlock * int(Block::BlockCount);
}

// Display name of a block, for the stand labels on the view.
QString BoxOffice::blockName(Block block)
{
    return QString::fromLatin1(kBlockNames[int(block)]);
}

// Called after the object has been moved to its thread, so the timer is born
// there and ticks on this loop rather than the caller's.
void BoxOffice::open()
{
    m_publishTimer = new QTimer(this);
    m_publishTimer->setInterval(kPublishIntervalMs);
    connect(m_publishTimer, &QTimer::timeout, this, &BoxOffice::publish);
    m_publishTimer->start();
}

// Books one request: allocates the seats, then posts the outcome back to the
// outlet named by the request's own return address.
void BoxOffice::submit(BoxOffice::Request request)
{
    ++m_handled;

    // the seats: the outlet's preferred block if it insists, otherwise
    // whichever block can still seat the whole party together
    int first = -1;
    if (request.anyBlock)
        first = reserveAnywhere(request.seats);
    else
        first = reserveIn(request.preferred, request.seats);

    Result result;
    result.id     = request.id;
    result.sentNs = request.sentNs;

    if (first >= 0)
    {
        m_pending.append(Run{first, request.seats});
        m_soldSeats += request.seats;
        result.seats   = request.seats;
        result.outcome = Result::Outcome::Confirmed;
    }

    // the confirmation goes back to the one outlet that asked, by the return
    // address on its own form
    if (request.replyTo != nullptr)
    {
        QMetaObject::invokeMethod(request.replyTo, "acceptResult", Qt::QueuedConnection,
                                  Q_ARG(BoxOffice::Result, result));
    }
}

// Puts every seat back on sale for the next run.
void BoxOffice::reopen()
{
    m_sold.fill(0);
    m_pending.clear();
    m_handled   = 0;
    m_soldSeats = 0;
}

// Seats the whole party together in one block. Returns the run's first seat,
// or -1 when the block cannot hold them all.
int BoxOffice::reserveIn(Block block, int wanted)
{
    int& sold = m_sold[size_t(block)];
    if (sold + wanted > kSeatsPerBlock)
        return -1;

    const int first = int(block) * kSeatsPerBlock + sold;
    sold += wanted;

    return first;
}

// Walks the blocks in order until one can take the party. Returns the run's
// first seat, or -1 when the ground is effectively sold out for this size.
int BoxOffice::reserveAnywhere(int wanted)
{
    for (int block = 0; block < int(Block::BlockCount); ++block)
    {
        const int first = reserveIn(Block(block), wanted);
        if (first >= 0)
            return first;
    }

    return -1;
}

// One batched heartbeat to the UI instead of a signal per booking: at burst
// rates a per-booking signal would bury the GUI loop under its own backlog.
void BoxOffice::publish()
{
    if (!m_pending.isEmpty())
    {
        emit soldRuns(m_pending);
        m_pending.clear();
    }

    emit progress(m_handled, m_soldSeats);
}
