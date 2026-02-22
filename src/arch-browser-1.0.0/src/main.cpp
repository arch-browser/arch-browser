/**
 * Arch Browser - Native Chromium-based web browser for Arch Linux
 *
 * Entry point: Initializes Qt application and Chromium engine (via QtWebEngine),
 * then launches the main window. Supports multiple application instances for
 * multi-window operation.
 */

#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QStandardPaths>
#include "MainWindow.hpp"

int main(int argc, char* argv[])
{
    // High DPI scaling for modern displays
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName("Arch Browser");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ArchBrowser");

    // Persistent storage for cookies, local storage, cache (keeps login sessions)
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/arch-browser";
    QWebEngineProfile::defaultProfile()->setPersistentStoragePath(dataPath);
    QWebEngineProfile::defaultProfile()->setCachePath(dataPath + "/cache");

    // Global Chromium/WebEngine settings - enables HTTPS, JavaScript, etc.
    QWebEngineSettings::defaultSettings()->setAttribute(
        QWebEngineSettings::LocalStorageEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(
        QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(
        QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(
        QWebEngineSettings::AutoLoadImages, true);

    // Create and show main window (empty or with URL from command line)
    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    if (argc > 1) {
        mainWindow->navigateTo(QString::fromUtf8(argv[1]));
    } else {
        mainWindow->navigateTo("about:blank");
    }

    return app.exec();
}
