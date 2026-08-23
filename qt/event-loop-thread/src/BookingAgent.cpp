#include "BookingAgent.h"

#include <QTimer>

#include <chrono>

namespace
{
constexpr int     kTickMs           = 20;
constexpr int     kBurstBatch       = 250;
constexpr int     kReportIntervalMs = 200;
constexpr int     kTicksPerSecond   = 1000 / kTickMs;
constexpr int     kPercentWhole     = 100;
constexpr int     kNanosPerMicro    = 1000;
constexpr quint32 kSeedOffset       = 7919;

// Monotonic and process-wide, so a stamp taken here is comparable to one
// taken anywhere else, and a clock adjustment cannot turn a round trip
// negative.
qint64 nowNs()
{
    const auto since = std::chrono::steady_clock::now().time_since_epoch();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(since).count();
}
}

// The profile is the outlet's whole personality. The rng is seeded per outlet
// so the outlets differ from each other while a rerun repeats itself.
BookingAgent::BookingAgent(int outlet, Profile profile, QObject* parent)
    : QObject(parent),
      m_profile(std::move(profile)),
      m_rng(quint32(outlet) + kSeedOffset),
      m_outlet(outlet)
{
}

// The outlet's fixed identity, read by the scoreboard.
const BookingAgent::Profile& BookingAgent::profile() const
{
    return m_profile;
}

// Both timers are created here rather than in the constructor, so that they
// belong to the thread this object was moved to.
void BookingAgent::open()
{
    m_paceTimer = new QTimer(this);
    connect(m_paceTimer, &QTimer::timeout, this, &BookingAgent::fire);

    m_reportTimer = new QTimer(this);
    m_reportTimer->setInterval(kReportIntervalMs);
    connect(m_reportTimer, &QTimer::timeout, this, &BookingAgent::publishTally);
    m_reportTimer->start();

    applyPace();
}

// Throttle from the UI slider: 0 stops the outlet, 100 is its natural rate,
// 400 is four times that.
void BookingAgent::setPacePercent(int percent)
{
    m_pacePercent = percent;
    applyPace();
}

// Burst ignores the profile rate and sells flat out; the benchmark uses it.
void BookingAgent::setBurst(bool on)
{
    m_burst = on;
    applyPace();
}

// The office's reply: updates the outlet's own counters and the round-trip
// statistics, all private to this thread.
void BookingAgent::acceptResult(BoxOffice::Result result)
{
    if (result.outcome == BoxOffice::Result::Outcome::Confirmed)
    {
        ++m_confirmed;
        m_seats += quint64(result.seats);
    }
    else
    {
        ++m_soldOut;
    }

    // round trip measured by the one who sent it: this thread owns both stamps
    const qint64 rttUs = (nowNs() - result.sentNs) / kNanosPerMicro;
    m_rttSumUs += rttUs;
    ++m_rttSamples;

    if (rttUs > m_worstRttUs)
        m_worstRttUs = rttUs;

    if (m_bestRttUs == 0 || rttUs < m_bestRttUs)
        m_bestRttUs = rttUs;
}

// Back to zero for a fresh sale, and says so on the board at once.
void BookingAgent::reset()
{
    m_sent       = 0;
    m_confirmed  = 0;
    m_soldOut    = 0;
    m_seats      = 0;
    m_bestRttUs  = 0;
    m_worstRttUs = 0;
    m_rttSumUs   = 0;
    m_rttSamples = 0;

    publishTally();
}

// One pace tick: works out how many requests this tick has earned, fills in
// that many booking forms and posts them to the office.
void BookingAgent::fire()
{
    int perTick = kBurstBatch;
    if (!m_burst)
    {
        const int scaled = m_profile.requestsPerSecond * m_pacePercent / kPercentWhole;
        perTick          = scaled / kTicksPerSecond;

        // a slow outlet earns less than one request per tick, so let the
        // remainder decide whether this tick produces anything at all
        if (perTick == 0)
        {
            const int remainder = scaled % kTicksPerSecond;
            if (int(m_rng.bounded(kTicksPerSecond)) >= remainder)
                return;

            perTick = 1;
        }
    }

    for (int i = 0; i < perTick; ++i)
    {
        BoxOffice::Request request;
        request.id        = m_nextId++;
        request.outlet    = m_outlet;
        request.seats     = drawSeatCount();
        request.preferred = m_profile.block;
        request.anyBlock  = !m_profile.prefersBlock;
        request.sentNs    = nowNs();
        request.replyTo   = this;

        ++m_sent;
        emit requestReady(request);
    }
}

// Restarts the pace timer to match the current throttle. Burst runs the timer
// flat out at interval zero; pace zero stops it altogether.
void BookingAgent::applyPace()
{
    if (m_paceTimer == nullptr)
        return;

    if (m_burst)
    {
        m_paceTimer->start(0);
        return;
    }

    if (m_pacePercent == 0)
    {
        m_paceTimer->stop();
        return;
    }

    m_paceTimer->start(kTickMs);
}

// Snapshots the counters into a Tally message for the GUI thread.
void BookingAgent::publishTally()
{
    Tally tally;
    tally.outlet     = m_outlet;
    tally.sent       = m_sent;
    tally.confirmed  = m_confirmed;
    tally.soldOut    = m_soldOut;
    tally.seats      = m_seats;
    tally.bestRttUs  = m_bestRttUs;
    tally.worstRttUs = m_worstRttUs;

    if (m_rttSamples > 0)
        tally.meanRttUs = m_rttSumUs / qint64(m_rttSamples);

    emit tallyChanged(tally);
}

// Party size for the next request, drawn from the profile's range.
int BookingAgent::drawSeatCount()
{
    const int spread = m_profile.maxSeats - m_profile.minSeats + 1;

    return m_profile.minSeats + int(m_rng.bounded(spread));
}
