/**
 * MainWindow - Main browser window with tabs, toolbar, and Chromium content
 *
 * Features:
 * - Tab bar: open, close, switch tabs
 * - Navigation: back, forward, refresh, stop, home
 * - Address bar with basic URL validation
 * - Find in page (Ctrl+F)
 * - Zoom in/out/reset
 * - Bookmarks (add, menu, persist)
 * - HTTPS support (via QtWebEngine/Chromium)
 * - Download handling
 * - Multiple windows
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QToolBar>
#include <QTabBar>
#include <QUrl>
#include <QWebEngineView>
#include <QWebEngineDownloadItem>

class WebView;
class QProgressBar;
class FindBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Called from main.cpp or when opening URL in new window
    void navigateTo(const QString& urlOrSearch);

    // Called by WebView::createWindow when page requests new tab/window
    QWebEngineView* createTabForExternalRequest();

private slots:
    void onNewWindow();
    void onNewTab();
    void onCloseTab(int index);
    void onTabChanged(int index);
    void onCurrentViewUrlChanged(const QUrl& url);
    void onCurrentViewTitleChanged(const QString& title);
    void onCurrentViewLoadProgress(int progress);
    void onGoClicked();
    void onAddressBarReturnPressed();
    void onBackClicked();
    void onForwardClicked();
    void onRefreshClicked();
    void onStopClicked();
    void onHomeClicked();
    void onDownloadRequested(QWebEngineDownloadItem* download);
    void onDownloadFinished();
    void onFindInPage();
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onAddBookmark();
    void onRemoveBookmark();
    void onManageBookmarks();
    void onBookmarkTriggered(QAction* action);
    void onSetHomePage();
    void onShowHistory();
    void onClearHistory();
    void onPageLoadedForHistory(const QUrl& url, const QString& title);

private:
    WebView* addTab(const QUrl& url = QUrl("about:blank"));
    WebView* currentWebView() const;
    void updateNavigationButtons();
    // Basic URL validation: ensures http/https or converts search to URL
    QString validateAndNormalizeUrl(const QString& input) const;
    void loadBookmarks();
    void saveBookmarks();
    void rebuildBookmarksMenu();
    void addToHistory(const QString& url, const QString& title);
    void loadHistory();
    void saveHistory();

    QTabWidget* m_tabWidget;
    QLineEdit* m_addressBar;
    QToolBar* m_toolbar;
    QAction* m_backAction;
    QAction* m_forwardAction;
    QAction* m_refreshAction;
    QAction* m_stopAction;
    QAction* m_homeAction;
    QProgressBar* m_progressBar;
    FindBar* m_findBar;
    QMenu* m_bookmarksMenu;
    struct Bookmark { QString title; QString url; };
    struct HistoryEntry { QString url; QString title; qint64 timestamp; };
    QList<Bookmark> m_bookmarks;
    QList<HistoryEntry> m_history;
    static constexpr double ZOOM_STEP = 1.2;
    static constexpr double ZOOM_MIN = 0.25;
    static constexpr double ZOOM_MAX = 5.0;
};

#endif // MAINWINDOW_HPP
