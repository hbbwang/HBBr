#pragma once
#include <QWidget>
#include "ui_ToolBox.h"
#include "ui_ToolPage.h"

class QFormLayout;
class QLabel;
class ToolPage : public QWidget
{
    Q_OBJECT
public:
    explicit ToolPage(bool isExpanded = false, QWidget* parent = nullptr);
    ~ToolPage();
public slots:
    void addWidget(const QString& title, QWidget* widget);
    void expand();
    void collapse();
private slots:
    void onPushButtonFoldClicked();
private:
    Ui::PageClassTool ui;
    bool m_bIsExpanded;
    class QLabel* m_pLabel;
};

class QVBoxLayout;
class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget* parent = nullptr);
    ~ToolBox();
    void addWidget(const QString& title, QWidget* widget , bool isExpanded = false);
private:
    Ui::ToolBoxClass ui;
    QVBoxLayout* m_pContentVBoxLayout;
};