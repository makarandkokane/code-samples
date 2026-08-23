#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include <array>

class QTimer;

// The single owner of the seating plan, living in its own thread with its own
// event loop. The seat counts are private members: no outlet ever reaches in,
// they post a Request and this object's loop applies them one at a time, in
// arrival order. That serialisation is what makes the class thread-safe, which
// is why there is no mutex, no lock and no atomic anywhere in this demo.
class BoxOffice : public QObject
{
    Q_OBJECT

public:
    enum class Block
    {
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
        BlockCount
    };

    // Seats are allocated as a contiguous run, the way best-available seating
    // works, so a booking of any size travels to the view as two integers.
    struct Run
    {
        int first = 0;
        int count = 0;
    };

    // The booking form. replyTo is the outlet's return address: the office
    // posts the outcome straight back to it rather than announcing it to all.
    struct Request
    {
        quint64  id        = 0;
        int      outlet    = -1;
        int      seats     = 0;
        Block    preferred = Block::North;
        bool     anyBlock  = true;
        qint64   sentNs    = 0;
        QObject* replyTo   = nullptr;
    };

    struct Result
    {
        enum class Outcome
        {
            Confirmed,
            SoldOut
        };

        quint64 id      = 0;
        int     seats   = 0;
        Outcome outcome = Outcome::SoldOut;
        qint64  sentNs  = 0;
    };

    explicit BoxOffice(QObject* parent = nullptr);

    static int     blockSeats();
    static int     totalSeats();
    static QString blockName(Block block);

public slots:
    void open();
    void submit(BoxOffice::Request request);
    void reopen();

signals:
    void soldRuns(QList<BoxOffice::Run> runs);
    void progress(quint64 handled, int soldSeats);

private:
    int  reserveIn(Block block, int wanted);
    int  reserveAnywhere(int wanted);
    void publish();

    // the seating ledger: only this object's own loop ever touches it
    std::array<int, size_t(Block::BlockCount)> m_sold = {};

    // sold runs batched onto the publish heartbeat for the view
    QList<Run> m_pending;
    QTimer*    m_publishTimer = nullptr;

    // running totals for the progress signal
    quint64 m_handled   = 0;
    int     m_soldSeats = 0;
};

Q_DECLARE_METATYPE(BoxOffice::Run)
Q_DECLARE_METATYPE(BoxOffice::Request)
Q_DECLARE_METATYPE(BoxOffice::Result)
