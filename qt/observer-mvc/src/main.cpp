#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

namespace
{
constexpr int kScreenshotDelayMs = 400;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("observer-mvc"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Observer / MVC demo (Qt Widgets)"));
    parser.addHelpOption();
    const QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"),
        QStringLiteral("Save a PNG of the window to <file> and exit."), QStringLiteral("file"));
    parser.addOption(screenshotOption);
    parser.process(app);

    MainWindow window;
    window.show();

    // Automation hook: lets CI or a script capture the README screenshot.
    if (parser.isSet(screenshotOption))
    {
        const QString file = parser.value(screenshotOption);
        QTimer::singleShot(kScreenshotDelayMs, &window,
                           [&window, file]
                           {
                               window.grab().save(file);
                               QApplication::quit();
                           });
    }
    return app.exec();
}
