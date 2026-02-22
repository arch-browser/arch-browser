/**
 * WebView - Chromium-embedded web content widget
 *
 * Wraps QWebEngineView to add:
 * - URL change notifications for address bar sync
 * - Load progress for status/UI feedback
 * - Title updates for tab display
 */

#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include <QWebEngineView>
#include <QUrl>
#include <functional>

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWidget* parent = nullptr);

    // Set callback for opening links in new tab (target="_blank", etc.)
    void setCreateTabCallback(std::function<QWebEngineView*()> callback);

    // Override to handle link navigation (e.g., open in new tab)
    QWebEngineView* createWindow(QWebEnginePage::WebWindowType type) override;

signals:
    void urlChanged(const QUrl& url);
    void loadProgress(int progress);
    void titleChanged(const QString& title);

private slots:
    void onUrlChanged(const QUrl& url);
    void onLoadProgress(int progress);
    void onTitleChanged(const QString& title);

private:
    std::function<QWebEngineView*()> m_createTabCallback;
};

#endif // WEBVIEW_HPP
