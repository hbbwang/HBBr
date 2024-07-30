#include "PropertyWidget.h"
#include "qlayout.h"
#include <qevent.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QRegularExpression>
#include "HString.h"
#include "FileSystem.h"

#define ParagraphLength 24

PropertyWidget::PropertyWidget(QWidget* parent)
	: QWidget(parent)
{
	setLayout(new QVBoxLayout(this));
	_splitter = new QSplitter(Qt::Orientation::Horizontal, this);
	((QVBoxLayout*)layout())->addWidget(_splitter);
	setObjectName("PropertyTableWidget");
	//
	QWidget* nw = new QWidget(this);
	QWidget* vw = new QWidget(this);
	_name_layout = new QVBoxLayout(nw);
	_value_layout = new QVBoxLayout(vw);
	nw->setLayout(_name_layout);
	vw->setLayout(_value_layout);

	_splitter->addWidget(nw);
	_splitter->addWidget(vw);

	((QVBoxLayout*)layout())->setContentsMargins(1, 1, 1, 1);
	((QVBoxLayout*)layout())->setSpacing(0);
	_name_layout->setContentsMargins(1, 1, 1, 1);
	_name_layout->setSpacing(0);
	_value_layout->setContentsMargins(1, 1, 1, 1);
	_value_layout->setSpacing(0);

	std::vector <SItem> items = { };
	_items.emplace(nullptr, items);

	_splitter->setSizes({ _splitter->width() /2 , _splitter->width() /2});
}

PropertyWidget::~PropertyWidget()
{
	ClearItems();
}

void PropertyWidget::AddItem(QString name, QWidget* widget, int Height, SGroup* group)
{
	//Name
	QLineEdit* p_name = new QLineEdit(name, this);
	p_name->setReadOnly(true);
	p_name->setObjectName("PropertyTableWidgetRow");
	p_name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	if (group)
	{
		p_name->setContentsMargins(group->depth * ParagraphLength, 0, 0, 0);
	}

	//Widget
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QWidget* p_value = new QWidget(this);
	p_value->setLayout(new QHBoxLayout(p_value));
	p_value->setObjectName("PropertyTableWidgetRowValue");
	p_value->layout()->addWidget(widget);
	p_value->layout()->setContentsMargins(0, 0, 0, 0);
	p_value->layout()->setSpacing(0);
	p_value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	if (Height > 0)
	{
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
			_items[group].push_back(newItem);
		}
		else
		{
			_items[nullptr].push_back(newItem);
		}
	}
	else
	{
		_items[nullptr].push_back(newItem);
	}
}

SGroup* PropertyWidget::AddGroup(QString groupName, SGroup* parent)
{
	auto gg = std::find_if(_groupCache.begin(), _groupCache.end(), [&](std::shared_ptr<SGroup>& g) {
			return g->groupName == groupName && g->parentGroup == parent;
		});
	std::shared_ptr<SGroup> newGroup;

	if (gg == _groupCache.end())
	{
		newGroup.reset(new SGroup);
		newGroup->groupName = groupName;
		newGroup->parentGroup = parent;
		if (parent != nullptr)
		{
			newGroup->depth = parent->depth + 1;
		}
		_groupCache.push_back(newGroup);
		std::vector<SItem> items = { };
		_items.emplace(newGroup.get(), items);
	}
	else
	{
		newGroup = *gg;
	}
	SGroup* result = newGroup.get();
	return result;
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
		child = nullptr;
	}
	child = nullptr;
	while ((child = _value_layout->takeAt(0)) != 0)
	{
		if (child->widget())
		{
			child->widget()->setParent(nullptr);
		}
		delete child;
		child = nullptr;
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
		child = nullptr;
	}
	child = nullptr;
	while ((child = _value_layout->takeAt(0)) != 0)
	{
		if (child->widget())
		{
			child->widget()->setParent(nullptr);
		}
		delete child;
		child = nullptr;
	}

	//�ȴ���û�з���,����Ҳû��Parent���
	for (auto& g : _items)
	{
		if (g.first == nullptr)
		{
			for (auto& i : g.second)
			{
				if (i.name)
				{
					_name_layout->addWidget(i.name);
				}
				if (i.value)
				{
					_value_layout->addWidget(i.value);
				}
			}
			break;
		}
	}

	//������
	for (auto& g : _groupCache)
	{
		g->count = 0;
		AddGroupButton(g);
	}

	//��������
	for (auto& g : _items)
	{
		if (g.first != nullptr)
		{
			auto group = g.first;
			PropertyWidgetButton* groupButton;
			//�ҵ����λ��
			int index = -1;
			for (int i = 0; i < _name_layout->count(); i++)
			{
				QString parentGroupName = "NULL";
				if (group->parentGroup != nullptr)
				{
					parentGroupName = group->parentGroup->groupName;
				}
				parentGroupName = parentGroupName + "_" + group->groupName;
				if (_name_layout->itemAt(i)->widget() && _name_layout->itemAt(i)->widget()->objectName().compare(parentGroupName) == 0)
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
					if (i.name)
					{
						_name_layout->insertWidget(index + 1 + g.first->count, i.name);
					}
					if (i.value)
					{
						_value_layout->insertWidget(index + 1 + g.first->count, i.value);
					}
					g.first->count++;
				}
				connect(groupButton->_button, &QAbstractButton::clicked, this,
					[this, group, groupButton]() {
						if (groupButton->bVisiable)
						{
							groupButton->bVisiable = false;
							groupButton->_image->setPixmap(QPixmap((HString(FileSystem::GetConfigAbsPath()) + "/Theme/Icons/TreeWidget_Close.png").c_str()).scaled(groupButton->_image->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
						}
						else
						{
							groupButton->bVisiable = true;
							groupButton->_image->setPixmap(QPixmap((HString(FileSystem::GetConfigAbsPath()) + "/Theme/Icons/TreeWidget_Open.png").c_str()).scaled(groupButton->_image->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

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
	int Height = 28;
	//Name

	QToolButton* button = new QToolButton(this);
	button->setText(g->groupName);
	button->setObjectName("PropertyTableWidgetRowGroupButton");
	button->setMinimumHeight(Height);
	button->setMaximumHeight(Height);
	button->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	PropertyWidgetButton* n_space = new PropertyWidgetButton(nullptr , button, this);
	n_space->setMinimumHeight(Height);
	n_space->setMaximumHeight(Height);
	n_space->setLayout(new QHBoxLayout(n_space));

	QLabel* image = new QLabel(button);
	image->setFixedSize(Height/2, Height /2);
	image->setObjectName("PropertyTableWidgetRowButtonImage");
	image->setPixmap(QPixmap((HString(FileSystem::GetConfigAbsPath()) + "/Theme/Icons/TreeWidget_Open.png").c_str()).scaled(image->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	n_space->_image = image;
	//n_space->layout()->addWidget(image);

	//QLabel* null = new QLabel(this);
	//null->setMinimumHeight(Height - 2);
	//null->setMaximumHeight(Height - 2);
	//null->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	//n_space->layout()->addWidget(null);
	n_space->layout()->addWidget(button);

	((QVBoxLayout*)n_space->layout())->setContentsMargins(0, 0, 0, 0);
	((QVBoxLayout*)n_space->layout())->setSpacing(0);

	QString parentGroupName = "NULL";
	if (g->parentGroup != nullptr)
	{
		parentGroupName = g->parentGroup->groupName;
	}
	parentGroupName = parentGroupName + "_" + g->groupName;
	n_space->setObjectName(parentGroupName);

	QWidget* v_space = new QWidget(this);
	v_space->setMinimumHeight(Height );
	v_space->setMaximumHeight(Height);


	v_space->setObjectName("PropertyTableWidgetRowGroupButtonValue");
	v_space->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

	n_space->setContentsMargins(g->depth * ParagraphLength, 0, 0, 0);
	image->move(4, Height / 4 );

	if (g->depth <= 6)
	{
		n_space->_button->setObjectName("PropertyTableGroup_" + QString::number(g->depth));
	}
	else
	{
		n_space->_button->setObjectName("PropertyTableGroup_Other");
	}

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
			_items[g->parentGroup].push_back(newItem);
		}
		else
		{
			_items[nullptr].push_back(newItem);
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

PropertyWidgetButton::PropertyWidgetButton(QLabel* image,QToolButton* button, QWidget* parent)
	:QWidget(parent)
{
	_image = image;
	_button = button;
}

PropertyWidgetButton::~PropertyWidgetButton()
{

}
