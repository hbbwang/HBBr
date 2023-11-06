#include "Inspector.h"
#include "qlabel.h"
#include "qtextedit.h"
#include "Component/GameObject.h"
#include "SceneOutline.h"
#include "CheckBox.h"
Inspector::Inspector(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("Inspector");
	ui.setupUi(this);

	mainWidget = new QWidget(this);
	mainWidget->setObjectName("Inspector");

	_layoutMain = new QVBoxLayout(this);
	mainWidget->setLayout(_layoutMain);
	_layoutMain->setContentsMargins(0, 0, 0, 0);
	_layoutMain->setSpacing(0);
	_updateTimer = new QTimer(this);
	_updateTimer->setSingleShot(false);
	_updateTimer->setInterval(50);
	connect(_updateTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()));
	_updateTimer->start();
}

Inspector::~Inspector()
{

}

void Inspector::RefreshInspector()
{
	if (!_currentGameObject.expired())
	{
		LoadInspector_GameObject(_currentGameObject);
	}
}

void Inspector::ClearInspector()
{
	QLayoutItem* child;
	while ((child = _layoutMain->takeAt(0)) != 0) {
		if (child->widget())
		{
			child->widget()->deleteLater();
			//delete child->widget();
		}		
		//delete child;
	}
	_currentGameObject.reset();
}

void Inspector::LoadInspector_GameObject(std::weak_ptr<GameObject> gameObj)
{
	if (gameObj.expired() || 
		(!_currentGameObject.expired() && _currentGameObject.lock()->GetGUID() == gameObj.lock()->GetGUID()))
		return;
	ClearInspector();
	_currentGameObject = gameObj.lock()->GetSelfWeekPtr();
	std::shared_ptr<GameObject> obj = gameObj.lock();
	//---------------------- Active & Title (GameObject Name)
	QWidget* titleWidget = new QWidget(this);
	QHBoxLayout* horLayout = new QHBoxLayout(this);
	_layoutMain->addWidget(titleWidget);
	titleWidget->setLayout(horLayout);
	//
	CheckBox* active = new CheckBox(this, obj->IsActive());
	active->_callback = [obj](bool bb) {
		if (obj)
		{
			obj->SetActive(bb);
		}
	};
	active->SetAlignmentLeft();
	active->_boolBind = &obj->_bActive;
	active->setMaximumWidth(20);
	horLayout->addWidget(active);
	QTextEdit* title = new QTextEdit(this);
	title->setObjectName("InspectorTitle");
	title->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
	title->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	title->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	title->setMaximumHeight(20);
	title->setMinimumHeight(20);
	title->setText(obj->GetObjectName().c_str());
	horLayout->addWidget(title);
	horLayout->addStretch(999);
	connect(title, &QTextEdit::textChanged, this, 
		[title , this]() {
			if (title && !_currentGameObject.expired() && title->toPlainText().endsWith('\n'))
			{
				_currentGameObject.lock()->SetObjectName(title->toPlainText().toStdString().c_str());
				title->clearFocus();
			}
		});
	//---------------------- 


	//
	_layoutMain->addStretch(9999);
}

void Inspector::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}

void Inspector::resizeEvent(QResizeEvent* event)
{
	if (mainWidget)
	{
		mainWidget->setGeometry(0, 0, this->width(), this->height());
	}
}

void Inspector::TimerUpdate()
{
	auto objects = SceneOutline::_treeWidget->GetSelectionObjects();
	if (objects.size() > 0)
	{
		LoadInspector_GameObject(objects[0]->GetSelfWeekPtr());
	}
}
