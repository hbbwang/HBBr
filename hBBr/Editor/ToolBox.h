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
    Ui::PageClassTool ui;
public slots:
    void addWidget(const QString& title, QWidget* widget);
    void expand();
    void collapse();
private slots:
    void onPushButtonFoldClicked();
private:
    bool m_bIsExpanded;
    class QLabel* m_pLabel;
};

class QVBoxLayout;
class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget* parent = nullptr);
    explicit ToolBox(QString title, bool isExpanded = false, QWidget* parent = nullptr);
    ~ToolBox();
    //添加页面,bAdditive: 是否强制加一个新页面
    void addPage(const QString& title, bool isExpanded = false, bool bAdditive = false);
    //在最新的页面上添加
    void addSubWidget(QWidget* widget);
    //根据标题名字获取到页面，然后添加
    void addSubWidget(const QString& title, QWidget* widget);
    //page的mianWidget
    std::map<QString, class QWidget*> _widgets;

    QWidget* getWidget()const {return m_widget;}
    Ui::ToolBoxClass ui;
private:
    QVBoxLayout* m_pContentVBoxLayout;
    QWidget* m_widget = nullptr;
};