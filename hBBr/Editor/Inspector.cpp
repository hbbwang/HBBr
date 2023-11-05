#include "Inspector.h"

Inspector::Inspector(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("Inspector");
	ui.setupUi(this);
	_layoutMain = new QVBoxLayout(this);
	this->setLayout(_layoutMain);
	_layoutMain->setContentsMargins(0, 0, 0, 0);
	_layoutMain->setSpacing(0);
	LoadInspector_Empty();
}

Inspector::~Inspector()
{

}

void Inspector::ClearInspector()
{
	QLayoutItem* child;
	while ((child = _layoutMain->takeAt(0)) != 0) {
		if (child->widget())
			delete child->widget();
		delete child;
	}
}

void Inspector::LoadInspector_Empty()
{
	ClearInspector();
	auto mainWidget = new QWidget(this);
	mainWidget->setObjectName("Inspector");
	_layoutMain->addWidget(mainWidget);
}

void Inspector::LoadInspector_GameObject(GameObject* gameObj)
{
	ClearInspector();
}

void Inspector::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}
