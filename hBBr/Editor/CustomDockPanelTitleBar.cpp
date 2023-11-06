#include "CustomDockPanelTitleBar.h"
#include <QStyleOption>
#include <qpainter.h>
#include <qpushbutton.h>
#include "EditorCommonFunction.h"
#include "HString.h"
#include "FileSystem.h"
CustomDockPanelTitleBar::CustomDockPanelTitleBar(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setMaximumHeight(28);

	_parent = parent;
	this->setObjectName("DockWidgetTitle");
	ui.Title->setObjectName("DockWidgetTitleText");
	ui.closeButton->setObjectName("DockWidgetTitleCloseButton");


	connect(this,SIGNAL(windowTitleChanged(const QString&)),this ,SLOT(TitleChange(const QString&)));
	connect(ui.closeButton, &QToolButton::clicked, this, [this]() {
		if(_parent)
			_parent->close();
	});
}

CustomDockPanelTitleBar::~CustomDockPanelTitleBar()
{
}

void CustomDockPanelTitleBar::TitleChange(const QString&  newTitle)
{
	ui.Title->setText(newTitle);
}

void CustomDockPanelTitleBar::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
	//

}