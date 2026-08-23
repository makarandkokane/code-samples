#pragma once

#include "BoxOffice.h"

#include <QObject>
#include <QRandomGenerator>
#include <QString>

class QTimer;

// One outlet that sells tickets: the website, the walk-up window, group sales.
// Each instance runs on its own thread and is a producer only. It never sees a
// seat count, it posts a Request and waits to be told the outcome, and its own
// counters are private to its thread, published as a Tally message.
class BookingAgent : public QObject
{
    Q_OBJECT

public:
    struct Profile
    {
        QString          name;
        int              minSeats          = 1;
        int              maxSeats          = 2;
        int              requestsPerSecond = 10;
        BoxOffice::Block block             = BoxOffice::Block::North;
        bool             prefersBlock      = false;
    };

    struct Tally
    {
        int     outlet     = -1;
        quint64 sent       = 0;
        quint64 confirmed  = 0;
        quint64 soldOut    = 0;
        quint64 seats      = 0;
        qint64  bestRttUs  = 0;
        qint64  worstRttUs = 0;
        qint64  meanRttUs  = 0;
    };

    BookingAgent(int outlet, Profile profile, QObject* parent = nullptr);

    const Profile& profile() const;

public slots:
    void open();
    void setPacePercent(int percent);
    void setBurst(bool on);
    void acceptResult(BoxOffice::Result result);
    void reset();

signals:
    void requestReady(BoxOffice::Request request);
    void tallyChanged(BookingAgent::Tally tally);

private:
    void fire();
    void applyPace();
    void publishTally();
    int  drawSeatCount();

    // who this outlet is and how it sells
    Profile          m_profile;
    QRandomGenerator m_rng;
    int              m_outlet = -1;

    // the two clocks: one fires requests, one publishes the tally
    QTimer* m_paceTimer   = nullptr;
    QTimer* m_reportTimer = nullptr;

    // the current throttle
    int  m_pacePercent = 100;
    bool m_burst       = false;

    // this thread's own counters, published as a Tally message
    quint64 m_nextId    = 1;
    quint64 m_sent      = 0;
    quint64 m_confirmed = 0;
    quint64 m_soldOut   = 0;
    quint64 m_seats     = 0;

    // round-trip statistics in microseconds
    qint64  m_bestRttUs  = 0;
    qint64  m_worstRttUs = 0;
    qint64  m_rttSumUs   = 0;
    quint64 m_rttSamples = 0;
};

Q_DECLARE_METATYPE(BookingAgent::Tally)
