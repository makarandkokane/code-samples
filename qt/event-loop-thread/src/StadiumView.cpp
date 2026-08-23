#include "StadiumView.h"

#include <QFontMetricsF>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>

namespace
{
// The bowl in seat units: one seat is one pixel of the plan image, and every
// block holds exactly BoxOffice::blockSeats() of them. Order matches
// BoxOffice::Block.
struct BlockPlan
{
    int x       = 0;
    int y       = 0;
    int columns = 0;
    int rows    = 0;
};

// clang-format off
constexpr BlockPlan kBlocks[] = {
    {125,   0, 250,  50}, // North
    {375,   0, 125, 100}, // North East
    {450, 100,  50, 250}, // East
    {375, 350, 125, 100}, // South East
    {125, 400, 250,  50}, // South
    {  0, 350, 125, 100}, // South West
    {  0, 100,  50, 250}, // West
    {  0,   0, 125, 100}, // North West
};
// clang-format on

// Two pixels per seat, one of them the seat and one the gap. That gap is the
// whole reason the view reads as a hundred thousand seats rather than eight
// coloured rectangles.
constexpr int kSeatCell      = 2;
constexpr int kPlanSeatsWide = 500;
constexpr int kPlanSeatsHigh = 450;
constexpr int kPlanWidth     = kPlanSeatsWide * kSeatCell;
constexpr int kPlanHeight    = kPlanSeatsHigh * kSeatCell;

// The pitch fills the void the ring leaves, exactly.
constexpr int kPitchLeft          = 50;
constexpr int kPitchTop           = 100;
constexpr int kPitchWidth         = 400;
constexpr int kPitchHeight        = 250;
constexpr int kCentreCircleRadius = 42;
constexpr int kLabelPointSize     = 11;
constexpr int kPercentWhole       = 100;
constexpr int kMinimumWidth       = 620;
constexpr int kPreferredWidth     = kPlanWidth;

// clang-format off
const QColor kConcourse( 78,  82,  80);
const QColor kEmptySeat( 46, 125,  50);
const QColor kSoldSeat( 198,  40,  40);
const QColor kPitch(     27,  77,  42);
const QColor kPitchLine(120, 168, 130);
const QColor kLabel(    206, 224, 210);
// clang-format on

// Maps a seat number to its pixel in the plan image: block first, then
// row-major within the block.
QPoint seatPixel(int seat)
{
    const int        block  = seat / BoxOffice::blockSeats();
    const int        index  = seat % BoxOffice::blockSeats();
    const BlockPlan& plan   = kBlocks[block];
    const int        column = plan.x + index % plan.columns;
    const int        row    = plan.y + index / plan.columns;

    return QPoint(column * kSeatCell, row * kSeatCell);
}

// Keeps a label box inside the view, so the narrow side stands do not have
// their text half off the edge.
QRectF clampInside(QRectF box, const QRect& area)
{
    if (box.left() < area.left())
        box.moveLeft(area.left());

    if (box.right() > area.right())
        box.moveRight(area.right());

    return box;
}

// Largest rectangle of the plan's own proportions that fits, centred, so the
// bowl never stretches out of shape.
QRect fitPlan(const QRect& area)
{
    const int byWidth = area.width() * kPlanHeight / kPlanWidth;
    int       width   = area.width();
    int       height  = byWidth;

    if (byWidth > area.height())
    {
        height = area.height();
        width  = area.height() * kPlanWidth / kPlanHeight;
    }

    const int x = area.x() + (area.width() - width) / 2;
    const int y = area.y() + (area.height() - height) / 2;

    return QRect(x, y, width, height);
}
}

// The plan image is painted in full once, here; after that only newly sold
// seats are touched.
StadiumView::StadiumView(QWidget* parent)
    : QWidget(parent),
      m_plan(kPlanWidth, kPlanHeight, QImage::Format_RGB32)
{
    paintEmptyPlan();
}

// Small enough for a laptop, in the plan's own proportions.
QSize StadiumView::minimumSizeHint() const
{
    return QSize(kMinimumWidth, kMinimumWidth * kPlanHeight / kPlanWidth);
}

// One screen pixel per plan pixel is the size worth opening at: below it the
// seat gaps start to drop out and the grain goes muddy.
QSize StadiumView::sizeHint() const
{
    return QSize(kPreferredWidth, kPreferredWidth * kPlanHeight / kPlanWidth);
}

// The office's heartbeat lands here: colours each sold run red and tallies it
// against its block for the percentage labels.
void StadiumView::markSold(QList<BoxOffice::Run> runs)
{
    for (const BoxOffice::Run& run : runs)
    {
        m_soldPerBlock[size_t(run.first / BoxOffice::blockSeats())] += run.count;

        for (int seat = run.first; seat < run.first + run.count; ++seat)
        {
            m_plan.setPixelColor(seatPixel(seat), kSoldSeat);
        }
    }

    update();
}

// Sales reopened: every seat back to green.
void StadiumView::clearSeats()
{
    m_soldPerBlock.fill(0);
    paintEmptyPlan();
    update();
}

// Scales the cached plan to the widget and draws the pitch and stand labels
// over it.
void StadiumView::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), kConcourse);

    // nearest-neighbour on purpose: a smoothed scale turns single seats into
    // mush, and the point of the view is that you can see one seat go red
    const QRect target = fitPlan(rect());
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(target, m_plan);

    drawPitch(painter, target);
    drawBlockLabels(painter, target);
}

// Repaints the base image: concourse background, every seat green.
void StadiumView::paintEmptyPlan()
{
    m_plan.fill(kConcourse);

    for (int seat = 0; seat < BoxOffice::totalSeats(); ++seat)
    {
        m_plan.setPixelColor(seatPixel(seat), kEmptySeat);
    }
}

// The pitch, halfway line and centre circle, scaled with the plan.
void StadiumView::drawPitch(QPainter& painter, const QRect& target) const
{
    const qreal  scale = qreal(target.width()) / kPlanSeatsWide;
    const QRectF pitch(target.x() + kPitchLeft * scale, target.y() + kPitchTop * scale,
                       kPitchWidth * scale, kPitchHeight * scale);

    painter.setPen(QPen(kPitchLine, 1.0));
    painter.setBrush(kPitch);
    painter.drawRect(pitch);

    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(pitch.center().x(), pitch.top()),
                     QPointF(pitch.center().x(), pitch.bottom()));
    painter.drawEllipse(pitch.center(), kCentreCircleRadius * scale, kCentreCircleRadius * scale);
}

// Names each stand and its percentage sold, kept inside the view edge.
void StadiumView::drawBlockLabels(QPainter& painter, const QRect& target) const
{
    const qreal scale = qreal(target.width()) / kPlanSeatsWide;

    QFont font = painter.font();
    font.setPointSize(kLabelPointSize);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(kLabel);

    const QFontMetricsF metrics(font);

    for (int block = 0; block < int(BoxOffice::Block::BlockCount); ++block)
    {
        const BlockPlan& plan = kBlocks[block];
        const QRectF     stand(target.x() + plan.x * scale, target.y() + plan.y * scale,
                               plan.columns * scale, plan.rows * scale);
        const int filled = m_soldPerBlock[size_t(block)] * kPercentWhole / BoxOffice::blockSeats();
        const QString label = QStringLiteral("%1  %2%")
                                  .arg(BoxOffice::blockName(BoxOffice::Block(block)))
                                  .arg(filled);

        QRectF box = metrics.boundingRect(label);
        box.moveCenter(stand.center());
        painter.drawText(clampInside(box, target), Qt::AlignCenter, label);
    }
}
