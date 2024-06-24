#include "PropertyWidget.h"
#include "qlayout.h"
#include <qevent.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QRegularExpression>
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



	QList <SItem> items = { };
	_items.emplace(nullptr, items);
}

PropertyWidget::~PropertyWidget()
{
	ClearItems();
}

void PropertyWidget::AddItem(QString name, QWidget* widget, int Height, SGroup* group)
{
	//Name
	QLineEdit* name_label = new QLineEdit(name, this);
	name_label->setReadOnly(true);
	name_label->setObjectName("PropertyTableWidgetRowName");

	QWidget* p_name = new QWidget(this);
	p_name->setLayout(new QHBoxLayout(this));
	p_name->setObjectName("PropertyTableWidgetRow");
	p_name->layout()->addWidget(name_label);
	p_name->layout()->setContentsMargins(0, 0, 0, 0);
	p_name->layout()->setSpacing(0);
	p_name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	//Widget
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QWidget* p_value = new QWidget(this);
	p_value->setLayout(new QHBoxLayout(this));
	p_value->setObjectName("PropertyTableWidgetRow");
	p_value->layout()->addWidget(widget);
	p_value->layout()->setContentsMargins(0, 0, 0, 0);
	p_value->layout()->setSpacing(0);
	p_value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	if (Height > 0)
	{
		name_label->setMinimumHeight(Height - 6);
		name_label->setMaximumHeight(Height - 6);
		widget->setMinimumHeight(Height - 6);
		widget->setMaximumHeight(Height - 6);
		p_name->setMinimumHeight(Height);
		p_name->setMaximumHeight(Height);
		p_value->setMinimumHeight(Height);
		p_value->setMaximumHeight(Height);
	}

	SItem newItem = {};
	newItem.name = p_name;
	newItem.value = p_value;


	if (group != nullptr)
	{
		auto it = _items.find(group);
		if (it != _items.end())
		{
			_items[group].append(newItem);
		}
		else
		{
			_items[nullptr].append(newItem);
		}
	}
	else
	{
		_items[nullptr].append(newItem);
	}
}

SGroup* PropertyWidget::AddGroup(QString groupName, SGroup* parent)
{
	std::shared_ptr<SGroup> newGroup;
	newGroup.reset(new SGroup);
	newGroup->groupName = groupName;
	newGroup->parentGroup = parent;
	if (parent != nullptr)
	{
		newGroup->depth = parent->depth + 1;
	}
	_groupCache.push_back(newGroup);

	QList <SItem> items = { };
	_items.emplace(newGroup.get(), items);

	return newGroup.get();
}

void PropertyWidget::ClearItems()
{
	_items.clear();
	_groupCache.clear();
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

void PropertyWidget::ShowItems()
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

	//先处理没有分组,并且也没有Parent组的
	for (auto& g : _items)
	{
		if (g.first == nullptr)
		{
			for (auto& i : g.second)
			{
				_name_layout->addWidget(i.name);
				_value_layout->addWidget(i.value);
			}
			break;
		}
	}

	//生成组
	for (auto& g : _groupCache)
	{
		AddGroupButton(g);
	}

	//处理分组的
	for (auto& g : _items)
	{
		if (g.first != nullptr)
		{
			auto group = g.first;
			PropertyWidgetButton* groupButton;
			//找到组的位置
			int index = -1;
			for (int i = 0; i < _name_layout->count(); i++)
			{
				if (_name_layout->itemAt(i)->widget() && _name_layout->itemAt(i)->widget()->objectName().compare(group->groupName) == 0)
				{
					index = i;
					groupButton = (PropertyWidgetButton*)_name_layout->itemAt(i)->widget();
					break;
				}
			}
			if (index >= 0)
			{
				auto group_items = g.second;
				for (auto& i : g.second)
				{
					_name_layout->insertWidget(index + 1, i.name);
					_value_layout->insertWidget(index + 1, i.value);
				}
				connect(groupButton->_button, &QAbstractButton::clicked, this,
					[this, group, groupButton]() {
						if (groupButton->bVisiable)
						{
							groupButton->bVisiable = false;
						}
						else
						{
							groupButton->bVisiable = true;
						}
						ChildrenHidden(group, !groupButton->bVisiable);
					});
			}
		}
	}

	_name_layout->addStretch(20);
	_value_layout->addStretch(20);
}

void PropertyWidget::AddGroupButton(std::shared_ptr<SGroup> g)
{
	int Height = 20;

	//Name
	QToolButton* button = new QToolButton(this);
	button->setText(g->groupName);
	button->setObjectName("PropertyTableWidgetRowButton");
	button->setMinimumHeight(Height);
	button->setMaximumHeight(Height);
	PropertyWidgetButton* n_space = new PropertyWidgetButton(button, this);
	n_space->setMinimumHeight(Height);
	n_space->setMaximumHeight(Height);
	n_space->setLayout(new QVBoxLayout(this));
	n_space->layout()->addWidget(button);
	((QVBoxLayout*)n_space->layout())->setContentsMargins(0, 0, 0, 0);
	((QVBoxLayout*)n_space->layout())->setSpacing(0);
	n_space->setObjectName(g->groupName);

	QWidget* v_space = new QWidget(this);
	v_space->setMinimumHeight(Height);
	v_space->setMaximumHeight(Height);
	v_space->setObjectName(g->groupName);

	_name_layout->addWidget(n_space);
	_value_layout->addWidget(v_space);

	if (g->parentGroup != nullptr)
	{
		SItem newItem = {};
		newItem.group_name = n_space;
		newItem.group_value = v_space;
		auto it = _items.find(g->parentGroup);
		if (it != _items.end())
		{
			_items[g->parentGroup].append(newItem);
		}
		else
		{
			_items[nullptr].append(newItem);
		}
	}
}

void PropertyWidget::ChildrenHidden(SGroup* group, bool bHidden)
{
	for (auto& i : _items[group])
	{
		if (i.name == nullptr)
		{
			i.group_name->setHidden(bHidden);
			i.group_value->setHidden(bHidden);
		}
		else
		{
			i.name->setHidden(bHidden);
			i.value->setHidden(bHidden);
		}
	}
	for (auto& i : _groupCache)
	{
		if (i->parentGroup == group)
		{
			ChildrenHidden(i.get(), bHidden);
		}
	}

}

PropertyWidgetButton::PropertyWidgetButton(QToolButton* button, QWidget* parent)
	:QWidget(parent)
{
	_button = button;

}

PropertyWidgetButton::~PropertyWidgetButton()
{

}
