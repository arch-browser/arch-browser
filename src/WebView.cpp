/**
 * WebView implementation
 */

#include "WebView.hpp"
#include <QWebEnginePage>

WebView::WebView(QWidget* parent)
    : QWebEngineView(parent)
    , m_createTabCallback(nullptr)
{
    connect(page(), &QWebEnginePage::urlChanged,
            this, &WebView::onUrlChanged);
    connect(this, &QWebEngineView::loadProgress,
            this, &WebView::onLoadProgress);
    connect(this, &QWebEngineView::titleChanged,
            this, &WebView::onTitleChanged);
}

void WebView::setCreateTabCallback(std::function<QWebEngineView*()> callback)
{
    m_createTabCallback = callback;
}

// Called when page requests new window (e.g., target="_blank" link)
// Returns the view to use - MainWindow creates new tab and returns its WebView
QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type);
    if (m_createTabCallback) {
        return m_createTabCallback();
    }
    return nullptr;
}

void WebView::onUrlChanged(const QUrl& url)
{
    emit urlChanged(url);
}

void WebView::onLoadProgress(int progress)
{
    emit loadProgress(progress);
}

void WebView::onTitleChanged(const QString& title)
{
    emit titleChanged(title);
}
