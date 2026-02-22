#include "FindBar.hpp"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QHBoxLayout>
#include <QLabel>

FindBar::FindBar(QWidget* parent)
    : QWidget(parent)
    , m_webView(nullptr)
{
    setFixedHeight(36);
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);

    layout->addWidget(new QLabel(tr("Find:")));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setMaximumWidth(250);
    m_searchEdit->setPlaceholderText(tr("Search in page..."));
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() { doFind(true); });
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FindBar::onTextChanged);
    layout->addWidget(m_searchEdit);

    m_prevBtn = new QPushButton(tr("Previous"), this);
    m_nextBtn = new QPushButton(tr("Next"), this);
    connect(m_prevBtn, &QPushButton::clicked, this, &FindBar::onFindPrevious);
    connect(m_nextBtn, &QPushButton::clicked, this, &FindBar::onFindNext);
    layout->addWidget(m_prevBtn);
    layout->addWidget(m_nextBtn);

    m_caseSensitive = new QCheckBox(tr("Match case"), this);
    connect(m_caseSensitive, &QCheckBox::toggled, this, [this]() { onTextChanged(m_searchEdit->text()); });
    layout->addWidget(m_caseSensitive);

    m_closeBtn = new QPushButton(tr("×"), this);
    m_closeBtn->setFixedSize(28, 28);
    connect(m_closeBtn, &QPushButton::clicked, this, &FindBar::onCloseClicked);
    layout->addWidget(m_closeBtn);

    layout->addStretch();
}

void FindBar::setWebView(QWebEngineView* view)
{
    m_webView = view;
}

void FindBar::showAndFocus()
{
    show();
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void FindBar::onFindNext()
{
    doFind(true);
}

void FindBar::onFindPrevious()
{
    doFind(false);
}

void FindBar::onTextChanged(const QString& text)
{
    Q_UNUSED(text);
    doFind(true);
}

void FindBar::onCloseClicked()
{
    hide();
    emit closed();
}

void FindBar::doFind(bool forward)
{
    if (!m_webView) return;
    QWebEnginePage::FindFlags flags;
    if (m_caseSensitive->isChecked()) flags |= QWebEnginePage::FindCaseSensitively;
    if (!forward) flags |= QWebEnginePage::FindBackward;
    m_webView->findText(m_searchEdit->text(), flags);
}
