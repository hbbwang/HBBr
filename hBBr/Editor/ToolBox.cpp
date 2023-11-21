#include "ToolBox.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QFormLayout>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>
#include <QPushButton>
#include "FileSystem.h"
ToolPage::ToolPage(bool isExpanded, QWidget* parent) :
    QWidget(parent),
    m_bIsExpanded(true),
    m_pLabel(nullptr)
{
    ui.setupUi(this);
    ui.widgetContent->setAttribute(Qt::WA_StyledBackground);

    m_pLabel = new QLabel(this);
    m_pLabel->setFixedSize(20, 20);
    QHBoxLayout* layout = new QHBoxLayout(ui.pushButtonFold);
    layout->setContentsMargins(0, 0, 5, 0);
    layout->addStretch(1);
    layout->addWidget(m_pLabel);

    connect(ui.pushButtonFold, &QPushButton::clicked, this, &ToolPage::onPushButtonFoldClicked);

    if (isExpanded) {
        expand();
    }
    else {
        collapse();
    }
}

ToolPage::~ToolPage()
{
}

void ToolPage::addWidget(const QString& title, QWidget* widget)
{
    ui.pushButtonFold->setText(title);
    ui.verticalLayoutContent->addWidget(widget);
}

void ToolPage::expand()
{
    ui.widgetContent->show();
    m_bIsExpanded = true;
    m_pLabel->setPixmap(QPixmap((HString(FileSystem::GetConfigAbsPath()) + "/Theme/Icons/TreeWidget_Open.png").c_str()).scaled(m_pLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ToolPage::collapse()
{
    ui.widgetContent->hide();
    m_bIsExpanded = false;
    m_pLabel->setPixmap(QPixmap((HString(FileSystem::GetConfigAbsPath()) + "/Theme/Icons/TreeWidget_Close.png").c_str()).scaled(m_pLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ToolPage::onPushButtonFoldClicked()
{
    if (m_bIsExpanded) {
        collapse();
    }
    else {
        expand();
    }
}

ToolBox::ToolBox(QWidget* parent) :
    QWidget(parent),
    m_pContentVBoxLayout(nullptr)
{
    ui.setupUi(this);
    ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget* widget = new QWidget(this);
    m_pContentVBoxLayout = new QVBoxLayout;
    m_pContentVBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentVBoxLayout->setSpacing(2);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(widget);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(m_pContentVBoxLayout);
    vBoxLayout->addStretch(1);

    ui.scrollArea->setWidget(widget);
}

ToolBox::ToolBox(QString title, bool isExpanded, QWidget* parent)
{
    ui.setupUi(this);
    ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget* widget = new QWidget(this);
    m_pContentVBoxLayout = new QVBoxLayout;
    m_pContentVBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentVBoxLayout->setSpacing(2);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(widget);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(m_pContentVBoxLayout);
    vBoxLayout->addStretch(1);

    ui.scrollArea->setWidget(widget);

    m_widget = new QWidget(this);
    m_widget->setLayout(new QVBoxLayout(this));
    ToolPage* page = new ToolPage(isExpanded, this);
    page->addWidget(title, m_widget);
    m_pContentVBoxLayout->addWidget(page);
}

ToolBox::~ToolBox()
{
    m_widget = NULL;
}

void ToolBox::addWidget(const QString& title, QWidget* widget, bool isExpanded)
{
    m_widget = widget;
    ToolPage* page = new ToolPage(isExpanded, this);
    page->addWidget(title, widget);
    m_pContentVBoxLayout->addWidget(page);
}

void ToolBox::addSubWidget(QWidget* widget)
{
    if (m_widget && m_widget->layout())
    {
        m_widget->layout()->addWidget(widget);
    }
}
