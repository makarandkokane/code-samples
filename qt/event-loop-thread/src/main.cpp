#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

namespace
{
constexpr int kScreenshotDelayMs = 400;
constexpr int kScreenshotSeats   = 35000;
}

// Entry point: registers the metatypes that cross thread boundaries, wires
// the --screenshot and --benchmark automation flags and runs the loop.
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("event-loop-thread"));

    // Queued connections copy their arguments, so every type that crosses a
    // thread boundary here has to be a known metatype first.
    qRegisterMetaType<BoxOffice::Request>();
    qRegisterMetaType<BoxOffice::Result>();
    qRegisterMetaType<BoxOffice::Run>();
    qRegisterMetaType<QList<BoxOffice::Run>>();
    qRegisterMetaType<BookingAgent::Tally>();

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Stadium box office: one event loop owns 100,000 seats (Qt Widgets)"));
    parser.addHelpOption();

    const QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"),
        QStringLiteral("Pause at 35,000 seats sold, save a PNG to <file> and exit."),
        QStringLiteral("file"));
    parser.addOption(screenshotOption);

    const QCommandLineOption benchmarkOption(
        QStringLiteral("benchmark"),
        QStringLiteral("Sell out at full speed and stop on the last seat."));
    parser.addOption(benchmarkOption);
    parser.process(app);

    MainWindow window;
    window.show();

    if (parser.isSet(benchmarkOption))
        window.runBurstBenchmark();

    // Automation hook for the README image. The pause makes the shot
    // reproducible; the throughput figures in the README come from an
    // ordinary run, since stopping the outlets mid-flow ends the measurement.
    if (parser.isSet(screenshotOption))
    {
        const QString file = parser.value(screenshotOption);
        if (!parser.isSet(benchmarkOption))
            window.pauseWhenSold(kScreenshotSeats);

        QObject::connect(&window, &MainWindow::paused, &window,
                         [&window, file]
                         {
                             QTimer::singleShot(kScreenshotDelayMs, &window,
                                                [&window, file]
                                                {
                                                    window.grab().save(file);
                                                    QApplication::quit();
                                                });
                         });
    }
    return app.exec();
}
