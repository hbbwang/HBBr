#include "Inspector.h"
#include "qlabel.h"
#include "qtextedit.h"
#include "Component/GameObject.h"
#include "SceneOutline.h"
#include "CheckBox.h"
#include "Component.h"
#include "ToolBox.h"
#include "VectorSetting.h"
#include "ResourceLine.h"
#include "ResourceObject.h"
#include "FileSystem.h"

#include "ModelData.h"
#include "Material.h"
#include "Texture.h"

Inspector* Inspector::_currentInspector = NULL;

Inspector::Inspector(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("Inspector");
	ui.setupUi(this);
	this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 禁用横向滚动条
	_layoutMain = new QVBoxLayout(this);
	scrollWidget = new QWidget(scrollArea);
	scrollWidget->setLayout(_layoutMain);

	scrollWidget->setObjectName("Inspector");
	scrollArea->setObjectName("Inspector");

	scrollArea->setWidget(scrollWidget);

	_layoutMain->setContentsMargins(0, 0, 0, 0);
	_layoutMain->setSpacing(0);
	_updateTimer = new QTimer(this);
	_updateTimer->setSingleShot(false);
	_updateTimer->setInterval(50);
	connect(_updateTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()));
	_updateTimer->start();

	_currentInspector = this;
}

Inspector::~Inspector()
{
	ClearInspector();
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
	_property_needUpdate.clear();
}

#define IsAsset(className)\
for (int i = 0; i < p.value.size(); i++)\
{\
	std::weak_ptr<className>** className##_value = std::any_cast<std::weak_ptr<className>*>(&p.value[i]);\
	if(className##_value)\
	{\
		assetInfoArray.push_back((*className##_value)->lock()->_assetInfo);\
	}\
}\

void Inspector::LoadInspector_GameObject(std::weak_ptr<GameObject> gameObj, bool bFoucsUpdate)
{
	if (gameObj.expired() || 
		(!bFoucsUpdate && !_currentGameObject.expired() && _currentGameObject.lock()->GetGUID() == gameObj.lock()->GetGUID())
		)
		return;
	ClearInspector();
	_currentGameObject = gameObj.lock()->GetSelfWeekPtr();
	std::shared_ptr<GameObject> obj = gameObj.lock();
	//---------------------- Active & Title (GameObject Name)
	QWidget* titleWidget = new QWidget(this);
	QHBoxLayout* horLayout = new QHBoxLayout(this);
	_layoutMain->addWidget(titleWidget);
	titleWidget->setLayout(horLayout);
	CheckBox* active = new CheckBox(this, obj->IsActive());
	active->_callback = [obj](bool bb) {
		if (obj)
		{
			obj->SetActive(bb);
		}
	};
	active->_boolBind = &obj->_bActive;
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
	//---------------------- Transform
	ToolBox* box = new ToolBox(this);
	_layoutMain->addWidget(box);
	{
		QWidget* transformWidget = new QWidget(this);
		transformWidget->setLayout(new QVBoxLayout(this));
		box->addWidget("Transform", transformWidget, true);
		auto transform = obj->_transform;
		VectorSetting* pos =	new VectorSetting("Position", this, 3, 0.001f, 4);
		VectorSetting* rot =	new VectorSetting("Rotation", this, 3, 0.01f, 4);
		VectorSetting* scale =	new VectorSetting("Scale   ", this, 3, 0.001f, 4);
		_property_needUpdate.append(pos);
		_property_needUpdate.append(rot);
		_property_needUpdate.append(scale);
		pos->ui.Name->setMinimumWidth(50);
		rot->ui.Name->setMinimumWidth(50);
		scale->ui.Name->setMinimumWidth(50);
		transformWidget->layout()->addWidget(pos);
		transformWidget->layout()->addWidget(rot);
		transformWidget->layout()->addWidget(scale);

		pos->SetValue(transform->location);
		pos->_vec4_f[0] = &transform->location.x;
		pos->_vec4_f[1] = &transform->location.y;
		pos->_vec4_f[2] = &transform->location.z;
		pos->BindValue = [gameObj](QList<FloatSetting*> v) {
			glm::vec3 newValue = glm::vec3(v[0]->GetValue(), v[1]->GetValue(), v[2]->GetValue());
			if (GameObject::IsValid(gameObj))
			{
				gameObj.lock()->GetTransform()->SetLocation(newValue);
			}
		};

		rot->SetValue(transform->eulerAngle);
		rot->_vec4_f[0] = &transform->eulerAngle.x;
		rot->_vec4_f[1] = &transform->eulerAngle.y;
		rot->_vec4_f[2] = &transform->eulerAngle.z;
		rot->BindValue = [gameObj](QList<FloatSetting*> v) {
			glm::vec3 newValue = glm::vec3(v[0]->GetValue(), v[1]->GetValue(), v[2]->GetValue());
			if (GameObject::IsValid(gameObj))
			{
				gameObj.lock()->GetTransform()->SetRotation(newValue);
			}
		};

		scale->SetValue(transform->scale3D);
		scale->_vec4_f[0] = &transform->scale3D.x;
		scale->_vec4_f[1] = &transform->scale3D.y;
		scale->_vec4_f[2] = &transform->scale3D.z;
		scale->BindValue = [gameObj](QList<FloatSetting*> v) {
			glm::vec3 newValue = glm::vec3(v[0]->GetValue(), v[1]->GetValue(), v[2]->GetValue());
			if (GameObject::IsValid(gameObj))
			{
				gameObj.lock()->GetTransform()->SetScale3D(newValue);
			}
		};
	}
	//---------------------- Components
	for (auto c : obj->_comps)
	{
		Component* comp = c ;
		QWidget* compWidget = new QWidget(this);
		compWidget->setLayout(new QVBoxLayout(this));
		box->addWidget(c->GetComponentName().c_str(), compWidget, true);
		auto pro = c->GetProperties();
		for (auto p : pro)
		{
			//Bool value
			std::vector<bool**> boolValue;
			for (int v = 0; v < p.value.size(); v++)
			{
				auto v_value = std::any_cast<bool*>(&p.value[v]);
				if (v_value)
				{
					boolValue.push_back(v_value);
				}
			}

			//AssetObject value
			std::vector<AssetInfoBase*> assetInfoArray;
			IsAsset(ModelData);
			IsAsset(Texture);
			IsAsset(Material);

			if (assetInfoArray.size() > 0)
			{	
				for (int vi = 0; vi < assetInfoArray.size(); vi++)
				{
					ResourceLine* line = new ResourceLine(p.name, this, assetInfoArray[vi]->virtualPath, assetInfoArray[vi]->suffix);
					compWidget->layout()->addWidget(line);
					line->_bindFindButtonFunc = [](const char* p) { //查找按钮回调函数

					};
					line->_bindStringFunc = [vi,p, assetInfoArray](ResourceLine* line, const char* s) { //文件拖拽回调函数
						auto guidStr = FileSystem::GetBaseName(s);
						HGUID guid;
						StringToGUID(guidStr.c_str(), &guid);
						if (guid.isValid())
						{
							auto assetInfo = assetInfoArray[vi];
							auto pp = p;
							if (assetInfo)
							{
								if (assetInfo->type == AssetType::Model)
								{
									std::weak_ptr<ModelData>** ModelData_value = std::any_cast<std::weak_ptr<ModelData>*>(&pp.value[vi]);
									auto newObject = ModelData::LoadAsset(guid);
									if (!newObject.expired())
									{
										*(*ModelData_value) = newObject;
									}
									line->_objectBind = ((std::weak_ptr<class ResourceObject>*) * ModelData_value);
								}
								else if (assetInfo->type == AssetType::Material)
								{
									//newObject = Material::LoadAsset(guid);
								}
								else if (assetInfo->type == AssetType::Texture2D)
								{
									//newObject = Texture::LoadAsset(guid);
								}
							}
						}
					};
				}
			}
			else if (boolValue.size() > 0)
			{
				for (int i = 0; i < boolValue.size(); i++)
				{
					CheckBox* checkBox = new CheckBox(p.name, this, c->IsActive());
					checkBox->_callback = [comp](bool b) {
						if (comp)
							comp->SetActive(b);
					};
					checkBox->_boolBind = *boolValue[i];
					compWidget->layout()->addWidget(checkBox);
				}				
			}
		}
	}
	box->ui.verticalLayout->addStretch(999);
	_layoutMain->addStretch(999);
}

void Inspector::PropertyUpdate()
{
	for (auto i : _property_needUpdate)
	{
		i->Update();
	}
}

void Inspector::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
	if (_currentInspector == this)
		_currentInspector = NULL;
}

void Inspector::resizeEvent(QResizeEvent* event)
{
	if (scrollArea)
	{
		scrollArea->setGeometry(0, 0, this->width(), this->height());
	}
}

void Inspector::focusInEvent(QFocusEvent* event)
{
	_currentInspector = this;
}

void Inspector::focusOutEvent(QFocusEvent* event)
{

}

void Inspector::TimerUpdate()
{

}
