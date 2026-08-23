#pragma once

#include "BoxOffice.h"

#include <QImage>
#include <QList>
#include <QSize>
#include <QWidget>

#include <array>

class QPainter;
class QRect;

// The seating plan as the crowd sees it: a bowl of eight blocks around the
// pitch, one pixel per seat, 100,000 of them. It lives on the GUI thread
// because Qt allows painting nowhere else, which is exactly the boundary this
// demo exists to show: the office owns the seats, this only draws what it is
// told about.
class StadiumView : public QWidget
{
    Q_OBJECT

public:
    explicit StadiumView(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:
    void markSold(QList<BoxOffice::Run> runs);
    void clearSeats();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paintEmptyPlan();
    void drawPitch(QPainter& painter, const QRect& target) const;
    void drawBlockLabels(QPainter& painter, const QRect& target) const;

    // the plan image, one pixel per seat, repainted only as bookings land
    QImage m_plan;

    // how full each block is, for the percentage labels
    std::array<int, size_t(BoxOffice::Block::BlockCount)> m_soldPerBlock = {};
};
