/**
 * MainWindow implementation - tabs, navigation, address bar, downloads,
 * find, zoom, bookmarks, home, stop
 */

#include "MainWindow.hpp"
#include "WebView.hpp"
#include "FindBar.hpp"
#include <QWebEngineProfile>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QProgressBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QSettings>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QShortcut>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QDateTime>
#include <QLocale>
#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QWebEngineCookieStore>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Arch Browser");
    resize(1200, 800);

    // Menu bar
    QMenuBar* menuBar = this->menuBar();
    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("New &Window"), this, &MainWindow::onNewWindow, QKeySequence::New);
    QAction* newTabAction = fileMenu->addAction(tr("New &Tab"), this, &MainWindow::onNewTab, QKeySequence(Qt::CTRL | Qt::Key_T));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close, QKeySequence::Quit);

    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Find in Page"), this, &MainWindow::onFindInPage, QKeySequence::Find);
    editMenu->addAction(tr("Zoom &In"), this, &MainWindow::onZoomIn, QKeySequence::ZoomIn);
    editMenu->addAction(tr("Zoom &Out"), this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    editMenu->addAction(tr("Zoom &Reset"), this, &MainWindow::onZoomReset, QKeySequence(Qt::CTRL | Qt::Key_0));
    editMenu->addSeparator();
    editMenu->addAction(tr("&Settings..."), this, &MainWindow::onOpenSettings, QKeySequence(Qt::CTRL | Qt::Key_Comma));

    m_bookmarksMenu = menuBar->addMenu(tr("&Bookmarks"));
    m_bookmarksMenu->addAction(tr("Add &Bookmark"), this, &MainWindow::onAddBookmark, QKeySequence(Qt::CTRL | Qt::Key_D));
    m_bookmarksMenu->addAction(tr("&Remove Bookmark"), this, &MainWindow::onRemoveBookmark);
    m_bookmarksMenu->addAction(tr("Set as &Home Page"), this, &MainWindow::onSetHomePage);
    m_bookmarksMenu->addAction(tr("&Manage Bookmarks..."), this, &MainWindow::onManageBookmarks);
    m_bookmarksMenu->addSeparator();

    QMenu* historyMenu = menuBar->addMenu(tr("&History"));
    historyMenu->addAction(tr("Show &History"), this, &MainWindow::onShowHistory, QKeySequence(Qt::CTRL | Qt::Key_H));
    historyMenu->addAction(tr("&Clear History"), this, &MainWindow::onClearHistory);

    loadBookmarks();
    loadHistory();
    rebuildBookmarksMenu();

    // Toolbar
    m_toolbar = addToolBar(tr("Navigation"));
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(24, 24));

    m_homeAction = m_toolbar->addAction(tr("Home"));
    m_homeAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Home));
    connect(m_homeAction, &QAction::triggered, this, &MainWindow::onHomeClicked);

    m_backAction = m_toolbar->addAction(tr("Back"));
    m_backAction->setShortcut(QKeySequence::Back);
    connect(m_backAction, &QAction::triggered, this, &MainWindow::onBackClicked);

    m_forwardAction = m_toolbar->addAction(tr("Forward"));
    m_forwardAction->setShortcut(QKeySequence::Forward);
    connect(m_forwardAction, &QAction::triggered, this, &MainWindow::onForwardClicked);

    m_refreshAction = m_toolbar->addAction(tr("Refresh"));
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::onRefreshClicked);

    m_stopAction = m_toolbar->addAction(tr("Stop"));
    m_stopAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(m_stopAction, &QAction::triggered, this, &MainWindow::onStopClicked);
    m_stopAction->setVisible(false);

    m_toolbar->addSeparator();

    // Address bar
    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText(tr("Enter URL or search..."));
    m_addressBar->setClearButtonEnabled(true);
    m_addressBar->setMinimumWidth(300);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &MainWindow::onAddressBarReturnPressed);
    m_toolbar->addWidget(m_addressBar);

    QAction* goAction = m_toolbar->addAction(tr("Go"));
    connect(goAction, &QAction::triggered, this, &MainWindow::onGoClicked);

    m_toolbar->addSeparator();

    // Progress bar (right side)
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();
    m_toolbar->addWidget(m_progressBar);

    // Central: tab widget + find bar
    QWidget* central = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setDocumentMode(true);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onCloseTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    centralLayout->addWidget(m_tabWidget);

    m_findBar = new FindBar(this);
    m_findBar->hide();
    connect(m_findBar, &FindBar::closed, this, [this]() { m_addressBar->setFocus(); });
    centralLayout->addWidget(m_findBar);

    setCentralWidget(central);

    // Keyboard shortcuts (Ctrl+T is on File->New Tab, don't duplicate)
    auto* shortcutCloseTab = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
    connect(shortcutCloseTab, &QShortcut::activated, this, [this]() {
        int i = m_tabWidget->currentIndex();
        if (i >= 0) onCloseTab(i);
    });
    auto* shortcutAddressBar = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(shortcutAddressBar, &QShortcut::activated, this, [this]() { m_addressBar->setFocus(); m_addressBar->selectAll(); });

    // Add first tab
    addTab(QUrl("https://google.com"));

    // Connect download handler once (default profile is shared across all tabs)
    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
            this, &MainWindow::onDownloadRequested);

    QSettings settings("ArchBrowser", "arch-browser");
    applyTheme(settings.value("theme", "System").toString());
}

MainWindow::~MainWindow()
{
}

void MainWindow::navigateTo(const QString& urlOrSearch)
{
    WebView* view = currentWebView();
    if (view) {
        QString url = validateAndNormalizeUrl(urlOrSearch);
        view->setUrl(QUrl(url));
        m_addressBar->setText(url);
    }
}

void MainWindow::onNewWindow()
{
    MainWindow* win = new MainWindow();
    win->show();
}

void MainWindow::onNewTab()
{
    addTab(QUrl("https://google.com"));
}

void MainWindow::onCloseTab(int index)
{
    if (m_tabWidget->count() > 1) {
        QWidget* w = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        w->deleteLater();
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        WebView* view = currentWebView();
        if (view) {
            m_addressBar->setText(view->url().toString());
            m_findBar->setWebView(view);
            updateNavigationButtons();
        }
    }
}

void MainWindow::onCurrentViewUrlChanged(const QUrl& url)
{
    if (sender() == currentWebView()) {
        m_addressBar->setText(url.toString());
    }
}

void MainWindow::onCurrentViewTitleChanged(const QString& title)
{
    WebView* view = qobject_cast<WebView*>(sender());
    if (view) {
        int idx = m_tabWidget->indexOf(view);
        if (idx >= 0) {
            QString tabTitle = title.isEmpty() ? tr("New Tab") : title;
            if (tabTitle.length() > 30) {
                tabTitle = tabTitle.left(27) + "...";
            }
            m_tabWidget->setTabText(idx, tabTitle);
        }
    }
}

void MainWindow::onCurrentViewLoadProgress(int progress)
{
    if (sender() != currentWebView()) return;
    m_progressBar->setValue(progress);
    m_progressBar->setVisible(progress < 100);
}


void MainWindow::onGoClicked()
{
    onAddressBarReturnPressed();
}

void MainWindow::onAddressBarReturnPressed()
{
    QString input = m_addressBar->text().trimmed();
    if (!input.isEmpty()) {
        navigateTo(input);
    }
}

void MainWindow::onBackClicked()
{
    WebView* view = currentWebView();
    if (view && view->history()->canGoBack()) {
        view->back();
    }
}

void MainWindow::onForwardClicked()
{
    WebView* view = currentWebView();
    if (view && view->history()->canGoForward()) {
        view->forward();
    }
}

void MainWindow::onRefreshClicked()
{
    WebView* view = currentWebView();
    if (view) {
        view->reload();
    }
}

void MainWindow::onStopClicked()
{
    WebView* view = currentWebView();
    if (view) {
        view->stop();
    }
}

void MainWindow::onHomeClicked()
{
    QSettings settings("ArchBrowser", "arch-browser");
    QString home = settings.value("homePage", "https://google.com").toString();
    navigateTo(home);
}

void MainWindow::onFindInPage()
{
    m_findBar->setWebView(currentWebView());
    m_findBar->showAndFocus();
}

void MainWindow::onZoomIn()
{
    WebView* view = currentWebView();
    if (view) {
        double z = qMin(view->zoomFactor() * ZOOM_STEP, ZOOM_MAX);
        view->setZoomFactor(z);
    }
}

void MainWindow::onZoomOut()
{
    WebView* view = currentWebView();
    if (view) {
        double z = qMax(view->zoomFactor() / ZOOM_STEP, ZOOM_MIN);
        view->setZoomFactor(z);
    }
}

void MainWindow::onZoomReset()
{
    WebView* view = currentWebView();
    if (view) {
        view->setZoomFactor(1.0);
    }
}

void MainWindow::onAddBookmark()
{
    WebView* view = currentWebView();
    if (!view) return;
    QUrl url = view->url();
    if (url.isEmpty() || url.scheme().isEmpty()) return;
    QString title = view->title().isEmpty() ? url.host() : view->title();
    m_bookmarks.append({title, url.toString()});
    saveBookmarks();
    rebuildBookmarksMenu();
    statusBar()->showMessage(tr("Bookmarked: %1").arg(title), 2000);
}

void MainWindow::onRemoveBookmark()
{
    WebView* view = currentWebView();
    if (!view) return;
    QString url = view->url().toString();
    if (url.isEmpty()) return;
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (m_bookmarks[i].url == url) {
            m_bookmarks.removeAt(i);
            saveBookmarks();
            rebuildBookmarksMenu();
            statusBar()->showMessage(tr("Bookmark removed"), 2000);
            return;
        }
    }
    statusBar()->showMessage(tr("Current page is not bookmarked"), 2000);
}

void MainWindow::onManageBookmarks()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Manage Bookmarks"));
    dlg.setMinimumSize(450, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QListWidget* list = new QListWidget(&dlg);
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(
            m_bookmarks[i].title + " \u2014 " + m_bookmarks[i].url);
        item->setData(Qt::UserRole, i);
        list->addItem(item);
    }

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* deleteBtn = new QPushButton(tr("&Delete"), &dlg);
    QPushButton* closeBtn = new QPushButton(tr("&Close"), &dlg);
    closeBtn->setDefault(true);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    layout->addWidget(list);
    layout->addLayout(btnLayout);

    connect(deleteBtn, &QPushButton::clicked, &dlg, [&]() {
        QListWidgetItem* cur = list->currentItem();
        if (!cur) return;
        int idx = cur->data(Qt::UserRole).toInt();
        m_bookmarks.removeAt(idx);
        delete list->takeItem(list->row(cur));
        saveBookmarks();
        rebuildBookmarksMenu();
        for (int i = 0; i < list->count(); ++i) {
            list->item(i)->setData(Qt::UserRole, i);
        }
    });
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

void MainWindow::onBookmarkTriggered(QAction* action)
{
    QString url = action->data().toString();
    if (!url.isEmpty()) {
        navigateTo(url);
    }
}

void MainWindow::onPageLoadedForHistory(const QUrl& url, const QString& title)
{
    QString urlStr = url.toString();
    if (urlStr.isEmpty() || urlStr == "about:blank" || url.scheme() == "data") return;
    addToHistory(urlStr, title.isEmpty() ? url.host() : title);
}

void MainWindow::addToHistory(const QString& url, const QString& title)
{
    // Remove existing entry for same URL (we'll add at front)
    for (int i = 0; i < m_history.size(); ++i) {
        if (m_history[i].url == url) {
            m_history.removeAt(i);
            break;
        }
    }
    m_history.prepend({url, title, QDateTime::currentMSecsSinceEpoch()});
    const int MAX_HISTORY = 500;
    while (m_history.size() > MAX_HISTORY) m_history.removeLast();
    saveHistory();
}

void MainWindow::loadHistory()
{
    m_history.clear();
    QSettings settings("ArchBrowser", "arch-browser");
    int size = settings.beginReadArray("history");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        m_history.append({
            settings.value("url").toString(),
            settings.value("title").toString(),
            settings.value("timestamp", 0).toLongLong()
        });
    }
    settings.endArray();
}

void MainWindow::saveHistory()
{
    QSettings settings("ArchBrowser", "arch-browser");
    settings.beginWriteArray("history");
    for (int i = 0; i < m_history.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("url", m_history[i].url);
        settings.setValue("title", m_history[i].title);
        settings.setValue("timestamp", m_history[i].timestamp);
    }
    settings.endArray();
}

void MainWindow::onShowHistory()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("History"));
    dlg.setMinimumSize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QListWidget* list = new QListWidget(&dlg);
    for (const HistoryEntry& e : m_history) {
        QString timeStr = QLocale().toString(QDateTime::fromMSecsSinceEpoch(e.timestamp), QLocale::ShortFormat);
        QListWidgetItem* item = new QListWidgetItem(e.title.isEmpty() ? e.url : (e.title + " \u2014 " + e.url));
        item->setData(Qt::UserRole, e.url);
        item->setToolTip(e.url + "\n" + timeStr);
        list->addItem(item);
    }

    QPushButton* closeBtn = new QPushButton(tr("&Close"), &dlg);
    closeBtn->setDefault(true);
    layout->addWidget(list);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(list, &QListWidget::itemDoubleClicked, &dlg, [this, &dlg](QListWidgetItem* item) {
        QString url = item->data(Qt::UserRole).toString();
        if (!url.isEmpty()) navigateTo(url);
        dlg.accept();
    });
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

void MainWindow::onClearHistory()
{
    m_history.clear();
    saveHistory();
    statusBar()->showMessage(tr("History cleared"), 2000);
}

void MainWindow::onOpenSettings()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Settings"));
    dlg.setMinimumSize(460, 320);

    QVBoxLayout* root = new QVBoxLayout(&dlg);

    QGroupBox* appearanceGroup = new QGroupBox(tr("Appearance"), &dlg);
    QHBoxLayout* appearanceLayout = new QHBoxLayout(appearanceGroup);
    QLabel* themeLabel = new QLabel(tr("Theme:"), appearanceGroup);
    QComboBox* themeCombo = new QComboBox(appearanceGroup);
    themeCombo->addItems({tr("System"), tr("Light"), tr("Dark")});

    QSettings settings("ArchBrowser", "arch-browser");
    const QString currentTheme = settings.value("theme", "System").toString();
    if (themeCombo->findText(currentTheme) >= 0) {
        themeCombo->setCurrentText(currentTheme);
    }

    appearanceLayout->addWidget(themeLabel);
    appearanceLayout->addWidget(themeCombo, 1);

    QGroupBox* privacyGroup = new QGroupBox(tr("Privacy"), &dlg);
    QVBoxLayout* privacyLayout = new QVBoxLayout(privacyGroup);
    QPushButton* clearHistoryBtn = new QPushButton(tr("Clear Browsing History"), privacyGroup);
    QPushButton* clearCookiesBtn = new QPushButton(tr("Clear Cookies + Saved Session"), privacyGroup);
    QPushButton* clearCacheBtn = new QPushButton(tr("Clear Cache"), privacyGroup);
    privacyLayout->addWidget(clearHistoryBtn);
    privacyLayout->addWidget(clearCookiesBtn);
    privacyLayout->addWidget(clearCacheBtn);

    QGroupBox* dataGroup = new QGroupBox(tr("Data"), &dlg);
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);
    QPushButton* clearBookmarksBtn = new QPushButton(tr("Clear All Bookmarks"), dataGroup);
    QPushButton* resetHomeBtn = new QPushButton(tr("Reset Home Page to Google"), dataGroup);
    dataLayout->addWidget(clearBookmarksBtn);
    dataLayout->addWidget(resetHomeBtn);

    QPushButton* closeBtn = new QPushButton(tr("&Close"), &dlg);
    closeBtn->setDefault(true);

    root->addWidget(appearanceGroup);
    root->addWidget(privacyGroup);
    root->addWidget(dataGroup);
    root->addStretch();
    root->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(themeCombo, &QComboBox::currentTextChanged, this, [this](const QString& theme) {
        QSettings s("ArchBrowser", "arch-browser");
        s.setValue("theme", theme);
        applyTheme(theme);
        statusBar()->showMessage(tr("Theme changed to %1").arg(theme), 2000);
    });

    connect(clearHistoryBtn, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, tr("Clear History"),
                                  tr("Delete all browsing history?")) == QMessageBox::Yes) {
            onClearHistory();
        }
    });

    connect(clearCookiesBtn, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, tr("Clear Cookies and Session"),
                                  tr("Delete all cookies and session data?")) == QMessageBox::Yes) {
            QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
            profile->cookieStore()->deleteAllCookies();
            profile->clearHttpCache();
            profile->clearAllVisitedLinks();
            statusBar()->showMessage(tr("Cookies and session data cleared"), 3000);
        }
    });

    connect(clearCacheBtn, &QPushButton::clicked, this, [this]() {
        QWebEngineProfile::defaultProfile()->clearHttpCache();
        statusBar()->showMessage(tr("Cache cleared"), 3000);
    });

    connect(clearBookmarksBtn, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, tr("Clear Bookmarks"),
                                  tr("Delete all saved bookmarks?")) == QMessageBox::Yes) {
            m_bookmarks.clear();
            saveBookmarks();
            rebuildBookmarksMenu();
            statusBar()->showMessage(tr("All bookmarks cleared"), 3000);
        }
    });

    connect(resetHomeBtn, &QPushButton::clicked, this, [this]() {
        QSettings s("ArchBrowser", "arch-browser");
        s.setValue("homePage", "https://google.com");
        statusBar()->showMessage(tr("Home page reset to https://google.com"), 3000);
    });

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void MainWindow::onSetHomePage()
{
    WebView* view = currentWebView();
    if (!view) return;
    QString url = view->url().toString();
    if (url.isEmpty() || url == "about:blank") return;
    QSettings settings("ArchBrowser", "arch-browser");
    settings.setValue("homePage", url);
    statusBar()->showMessage(tr("Home page set to: %1").arg(url), 2000);
}

void MainWindow::onDownloadRequested(QWebEngineDownloadItem* download)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"),
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + download->suggestedFileName());
    if (!path.isEmpty()) {
        QFileInfo fi(path);
        download->setDownloadDirectory(fi.absolutePath());
        download->setDownloadFileName(fi.fileName());
        connect(download, &QWebEngineDownloadItem::finished, this, &MainWindow::onDownloadFinished);
        download->accept();
    }
}

void MainWindow::onDownloadFinished()
{
    statusBar()->showMessage(tr("Download finished"), 3000);
}

void MainWindow::loadBookmarks()
{
    m_bookmarks.clear();
    QSettings settings("ArchBrowser", "arch-browser");
    int size = settings.beginReadArray("bookmarks");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        m_bookmarks.append({
            settings.value("title").toString(),
            settings.value("url").toString()
        });
    }
    settings.endArray();
}

void MainWindow::saveBookmarks()
{
    QSettings settings("ArchBrowser", "arch-browser");
    settings.beginWriteArray("bookmarks");
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("title", m_bookmarks[i].title);
        settings.setValue("url", m_bookmarks[i].url);
    }
    settings.endArray();
}

void MainWindow::rebuildBookmarksMenu()
{
    // Remove old bookmark items (keep Add, Remove, Set Home, Manage, separator)
    QList<QAction*> actions = m_bookmarksMenu->actions();
    for (int i = 5; i < actions.size(); ++i) {
        m_bookmarksMenu->removeAction(actions[i]);
    }
    for (const Bookmark& b : m_bookmarks) {
        QAction* a = m_bookmarksMenu->addAction(b.title);
        a->setData(b.url);
        connect(a, &QAction::triggered, this, [this, a]() { onBookmarkTriggered(a); });
    }
}

QWebEngineView* MainWindow::createTabForExternalRequest()
{
    WebView* view = addTab();
    return view;
}

WebView* MainWindow::addTab(const QUrl& url)
{
    WebView* view = new WebView(this);
    view->setUrl(url);

    view->setCreateTabCallback([this]() -> QWebEngineView* {
        return createTabForExternalRequest();
    });

    connect(view, &WebView::urlChanged, this, &MainWindow::onCurrentViewUrlChanged);
    connect(view, &WebView::titleChanged, this, &MainWindow::onCurrentViewTitleChanged);
    connect(view, &WebView::loadProgress, this, &MainWindow::onCurrentViewLoadProgress);
    connect(view, &WebView::urlChanged, this, [this]() { updateNavigationButtons(); });
    connect(view->page(), &QWebEnginePage::loadStarted, this, [this, view]() {
        if (view == currentWebView()) {
            m_refreshAction->setVisible(false);
            m_stopAction->setVisible(true);
        }
    });
    connect(view->page(), &QWebEnginePage::loadFinished, this, [this, view](bool ok) {
        if (view == currentWebView()) {
            m_refreshAction->setVisible(true);
            m_stopAction->setVisible(false);
        }
        if (ok) onPageLoadedForHistory(view->url(), view->title());
    });

    int idx = m_tabWidget->addTab(view, tr("New Tab"));
    m_tabWidget->setCurrentIndex(idx);
    m_findBar->setWebView(view);
    updateNavigationButtons();

    return view;
}

WebView* MainWindow::currentWebView() const
{
    return qobject_cast<WebView*>(m_tabWidget->currentWidget());
}

void MainWindow::updateNavigationButtons()
{
    WebView* view = currentWebView();
    if (view) {
        m_backAction->setEnabled(view->history()->canGoBack());
        m_forwardAction->setEnabled(view->history()->canGoForward());
    }
}

QString MainWindow::validateAndNormalizeUrl(const QString& input) const
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return "about:blank";

    static QRegularExpression schemeRx("^[a-zA-Z][a-zA-Z0-9+.-]*:");
    if (schemeRx.match(trimmed).hasMatch()) {
        return trimmed;
    }

    if (trimmed.contains('.')) {
        return "https://" + trimmed;
    }

    return "https://duckduckgo.com/?q=" + QUrl::toPercentEncoding(trimmed);
}

void MainWindow::applyTheme(const QString& theme)
{
    if (theme == "Dark") {
        qApp->setStyleSheet(
            "QWidget { background-color: #1c1f24; color: #e8eaed; }"
            "QLineEdit, QListWidget, QTabWidget::pane, QMenu, QToolBar {"
            "  background-color: #242830; color: #e8eaed; border: 1px solid #3a404c; }"
            "QPushButton { background-color: #2d3440; border: 1px solid #4b5566; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #394252; }");
        return;
    }
    if (theme == "Light") {
        qApp->setStyleSheet(
            "QWidget { background-color: #f5f7fa; color: #1f2933; }"
            "QLineEdit, QListWidget, QTabWidget::pane, QMenu, QToolBar {"
            "  background-color: #ffffff; color: #1f2933; border: 1px solid #cfd8e3; }"
            "QPushButton { background-color: #eef2f7; border: 1px solid #c3ccd7; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #dde6f0; }");
        return;
    }
    qApp->setStyleSheet("");
}
