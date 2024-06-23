#include "PropertyWidget.h"
#include "qlayout.h"
#include <qevent.h>
#include <qlabel.h>
#include <qlineedit.h>
PropertyWidget::PropertyWidget(QWidget* parent)
	: QWidget(parent)
{
	setLayout(new QVBoxLayout(this));
	_splitter = new QSplitter(Qt::Orientation::Horizontal, this);
	((QVBoxLayout*)layout())->addWidget(_splitter);
	setObjectName("PropertyTableWidget");
	//
	_name_layout = new QVBoxLayout(this);
	_value_layout = new QVBoxLayout(this);
	QWidget* nw = new QWidget(this);
	nw->setLayout(_name_layout);
	QWidget* vw = new QWidget(this);
	vw->setLayout(_value_layout);

	_splitter->addWidget(nw);
	_splitter->addWidget(vw);

	((QVBoxLayout*)layout())->setContentsMargins(1, 1, 1, 1);
	((QVBoxLayout*)layout())->setSpacing(0);
	_name_layout->setContentsMargins(1, 1, 1, 1);
	_name_layout->setSpacing(0);
	_value_layout->setContentsMargins(1, 1, 1, 1);
	_value_layout->setSpacing(0);

	_name_layout->addStretch(20);
	_value_layout->addStretch(20);
}

PropertyWidget::~PropertyWidget()
{
}

void PropertyWidget::AddItem(QString name, QWidget* widget)
{
	//Name
	QLineEdit* name_label = new QLineEdit(name, this);
	name_label->setReadOnly(true);
	name_label->setObjectName("PropertyTableWidgetRowName");
	name_label->setMinimumHeight(24);
	name_label->setMaximumHeight(24);

	QWidget* www = new QWidget(this);
	www->setLayout(new QHBoxLayout(this));
	www->setObjectName("PropertyTableWidgetRow");
	www->layout()->addWidget(name_label);
	www->layout()->setContentsMargins(0, 0, 0, 0);
	www->layout()->setSpacing(0);
	www->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	_name_layout->insertWidget(_name_layout->count() - 1, www);
	www->setMinimumHeight(30);
	www->setMaximumHeight(30);

	//Widget
	widget->setMinimumHeight(24);
	widget->setMaximumHeight(24);
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	www = new QWidget(this);
	www->setLayout(new QHBoxLayout(this));
	www->setObjectName("PropertyTableWidgetRow");
	www->layout()->addWidget(widget);
	www->layout()->setContentsMargins(0, 0, 0, 0);
	www->layout()->setSpacing(0);
	www->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	_value_layout->insertWidget(_value_layout->count() - 1, www);
	www->setMinimumHeight(30);
	www->setMaximumHeight(30);
}

void PropertyWidget::ClearItems()
{
	QLayoutItem* child = nullptr;
	while ((child = _name_layout->takeAt(0)) != 0)
	{
		if (child->widget())
		{
			child->widget()->setParent(nullptr);
		}
		delete child;
	}
	child = nullptr;
	while ((child = _value_layout->takeAt(0)) != 0)
	{
		if (child->widget())
		{
			child->widget()->setParent(nullptr);
		}
		delete child;
	}
}
