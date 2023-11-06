#include "Inspector.h"
#include "qlabel.h"
#include "qtextedit.h"
#include "Component/GameObject.h"
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

void Inspector::RefreshInspector()
{
	if (!_currentGameObject.expired())
	{
		LoadInspector_GameObject(_currentGameObject);
	}
	else
	{
		LoadInspector_Empty();
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

void Inspector::LoadInspector_Empty()
{
	ClearInspector();
	auto mainWidget = new QWidget(this);
	mainWidget->setObjectName("Inspector");
	_layoutMain->addWidget(mainWidget);
}

void Inspector::LoadInspector_GameObject(std::weak_ptr<GameObject> gameObj)
{
	if (gameObj.expired())
		return;
	_currentGameObject = gameObj;
	std::shared_ptr<GameObject> obj = gameObj.lock();
	ClearInspector();
	//---------------------- Title (GameObject Name)
	QTextEdit* title = new QTextEdit(this);
	title->setObjectName("InspectorTitle");
	_layoutMain->addWidget(title);
	title->setText(obj->GetObjectName().c_str());
	connect(title, &QTextEdit::textChanged, this, 
		[&]() {
			if (title->toPlainText().endsWith('\n'))
				obj->SetObjectName(title->toPlainText().toStdString().c_str());
		});
	//---------------------- GameObject active check box


	//
	_layoutMain->addStretch(9999);
}

void Inspector::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}
