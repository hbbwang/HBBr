#include "LineEditDialog.h"

LineEditDialog::LineEditDialog(QString name , QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name);
	this->setWindowTitle(name);

	this->setAttribute(Qt::WidgetAttribute::WA_Resized, false);
	this->setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop, true);
	this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
	this->setWindowFlag(Qt::FramelessWindowHint, true);
	this->setWindowFlag(Qt::WindowSystemMenuHint, false);
	this->setWindowFlag(Qt::WindowMinimizeButtonHint, false);
	this->setWindowFlag(Qt::WindowMaximizeButtonHint, false);
	this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, true);

	connect(ui.EnterButton,&QPushButton::clicked,this, &LineEditDialog::Enter);
	connect(ui.CancelButton, &QPushButton::clicked, this, &LineEditDialog::Cancel);

	CancelCallBack = [this]() {
	};
	this->show();
}

LineEditDialog::~LineEditDialog()
{}

void LineEditDialog::Enter()
{
	EnterCallBack();
	this->setParent(nullptr);
	this->close();
}

void LineEditDialog::Cancel()
{
	CancelCallBack();
	this->setParent(nullptr);
	this->close();
}
