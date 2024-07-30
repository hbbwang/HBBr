#include "CollapsedWidget.h"

CollapsedWidget::CollapsedWidget(bool isSubWidget, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.CollapseButton->setObjectName("CollapsedWidget_CollapseButton");
	ui.line->setObjectName("CollapsedWidget_Separatrix");
	ui.MianTitle->setObjectName("CollapsedWidget_Title");
	ui.widget->setObjectName("CollapsedWidget_Title");
	ui.CollapsedWidget_Widget->setObjectName("CollapsedWidget_Widget");

	if (isSubWidget)
	{
		_isSubWidget = isSubWidget;
		ui.MianTitle->setObjectName("CollapsedWidget_SubTitle");
		ui.widget->setObjectName("CollapsedWidget_SubTitle");
		ui.horizontalLayout_2->insertSpacerItem(0,new QSpacerItem(20,20));
	}

	connect(this, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(TitleChanged(const QString&)));
	connect(ui.CollapseButton, SIGNAL(clicked(bool)), this, SLOT(CollapseOrExpand()));

	Expand();
	ResizeButton();
}

CollapsedWidget::~CollapsedWidget()
{


}

void CollapsedWidget::Collapse()
{
	bIsCollapsed = true;
	ui.CollapseButton->setText(QStringLiteral("ฃผ"));
	//ui.CollapsedWidget_Widget->setHidden(true);
	ui.CollapsedWidget_Widget->setMaximumHeight(0);
}

void CollapsedWidget::Expand()
{
	bIsCollapsed = false;
	ui.CollapseButton->setText(QStringLiteral("กล"));
	ui.CollapsedWidget_Widget->setMaximumHeight(16777214);
	//ui.CollapsedWidget_Widget->setHidden(false);
}

void CollapsedWidget::resizeEvent(QResizeEvent* event)
{
	//ResizeButton();
	QWidget::resizeEvent(event);
}

void CollapsedWidget::paintEvent(QPaintEvent* event)
{
	ResizeButton();
}

void CollapsedWidget::CollapseOrExpand()
{
	if (bIsCollapsed)
	{
		Expand();
	}
	else
	{
		Collapse();
	}
}

void CollapsedWidget::ResizeButton()
{
	//int fontSize =ui.MianTitle->fontMetrics().width("A") * 2.5;
	ui.CollapseButton->setMinimumSize(ui.MianTitle->height()*1.25, ui.MianTitle->height()/1.8);
}

void CollapsedWidget::TitleChanged(const QString& title)
{
	ui.MianTitle->setText(title);
}
