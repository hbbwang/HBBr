#include "CustomSearchLine.h"
#include "HString.h"
#include "EditorCommonFunction.h"
#include <QPixmap>
#include <qabstractitemview.h>
#include <QPainter>
#include <QStyleOption>
CustomSearchLine::CustomSearchLine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setObjectName("SearchLine");
	ui.lineEdit->setObjectName("SearchLine_LineEdit");
	ui.comboBox->setObjectName("SearchLine_ComboBox");
	ui.comboBox->view()->viewport()->setObjectName("SearchLine_ComboBoxViewport");
	ui.label->setScaledContents(true);
	//ui.label->setMinimumSize(20, 20);
	//ui.label->setMaximumSize(20, 20);
	//this->setMaximumHeight(32);
	//this->setMinimumHeight(32);
	ui.verticalLayout->setContentsMargins(0,0,0,2);
	ui.verticalLayout->setSpacing(0);
	ui.horizontalLayout->setContentsMargins(0, 0, 0, 0);
	ui.horizontalLayout->setSpacing(0);
	//QPixmap image((FileSystem::GetProgramPath() + "Config/Theme/Icons/ICON_SEARCH.png").c_str());
	//image.scaled(4,4);
	//ui.label->setPixmap(image);
	connect(ui.lineEdit, &QLineEdit::returnPressed, this, 
		[this]() {
			_enterSearchCallback(this->ui.lineEdit);
		});

}

CustomSearchLine::~CustomSearchLine()
{

}

void CustomSearchLine::SetLabel(QString text)
{
	ui.label->setText(text);
}

void CustomSearchLine::SetLabel(QPixmap pixmap)
{
	ui.label->setPixmap(pixmap);
}

void CustomSearchLine::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

	if (ui.label->pixmap() && !ui.label->pixmap()->isNull())
	{
		ui.label->setMinimumSize(ui.lineEdit->height() - 1, ui.lineEdit->height() - 1);
		ui.label->setMaximumSize(ui.lineEdit->height() - 1, ui.lineEdit->height() - 1);
	}
}