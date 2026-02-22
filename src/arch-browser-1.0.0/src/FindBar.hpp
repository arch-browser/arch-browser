/**
 * FindBar - Find-in-page toolbar (Ctrl+F)
 */

#ifndef FINDBAR_HPP
#define FINDBAR_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

class QWebEngineView;

class FindBar : public QWidget
{
    Q_OBJECT

public:
    explicit FindBar(QWidget* parent = nullptr);
    void setWebView(QWebEngineView* view);
    void showAndFocus();

signals:
    void closed();

private slots:
    void onFindNext();
    void onFindPrevious();
    void onTextChanged(const QString& text);
    void onCloseClicked();

private:
    void doFind(bool forward);

    QLineEdit* m_searchEdit;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QCheckBox* m_caseSensitive;
    QPushButton* m_closeBtn;
    QWebEngineView* m_webView;
};

#endif // FINDBAR_HPP
