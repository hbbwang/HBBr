#include "Inspector.h"
#include "qlabel.h"
#include "qtextedit.h"
#include "Component/GameObject.h"
#include "SceneOutline.h"
#include "CheckBox.h"
#include "Component.h"
#include "CollapsedWidget.h"
#include "ToolBox.h"
#include "VectorSetting.h"
#include "AssetLine.h"
#include "AssetObject.h"
#include "FileSystem.h"
#include "ContentBrowser.h"

#include "Model.h"
#include "Material.h"
#include "Texture2D.h"

Inspector* Inspector::_currentInspector = nullptr;

Inspector::Inspector(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("Inspector");
	ui.setupUi(this);
	this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
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

#define AssetStringBinding(className)\
std::weak_ptr<className>** className##_value = std::any_cast<std::weak_ptr<className>*>(&pp.value[vi]);\
auto newObject = className::LoadAsset(guid);\
if (!newObject.expired())\
{\
	*(*className##_value) = newObject;\
}\
line->_objectBind = ((std::weak_ptr<class AssetObject>*) * className##_value);

#define IsType(typeStr) p.type.IsSame(typeStr, false)

void Inspector::LoadInspector_GameObject(std::weak_ptr<GameObject> gameObj, bool bFoucsUpdate)
{
	if (gameObj.expired() || 
		(!bFoucsUpdate && !_currentGameObject.expired() && _currentGameObject.lock()->GetGUID() == gameObj.lock()->GetGUID())
		)
		return;
	ClearInspector();
	_currentGameObject = gameObj.lock()->GetSelfWeekPtr();
	std::shared_ptr<GameObject> obj = gameObj.lock();
	_layoutMain->setContentsMargins(1, 1, 1, 1);
	_layoutMain->setSpacing(1);
	//---------------------- Active & Title (GameObject Name)
	QWidget* titleWidget = new QWidget(this);
	QHBoxLayout* horLayout = new QHBoxLayout(this);
	horLayout->setContentsMargins(1, 1, 1, 1);
	horLayout->setSpacing(1);
	titleWidget->setLayout(horLayout);
	_layoutMain->addWidget(titleWidget);
	_layoutMain->addStretch(10);
	CheckBox* active = new CheckBox(this, obj->IsActive());
	active->setObjectName("Inspector_Active");
	active->ui.checkBox->setObjectName("Inspector_Active");
	active->_callback = [obj](bool bb) {
		if (obj)
		{
			obj->SetActive(bb);
		}
	};
	active->_boolBind = &obj->_bActive;
	horLayout->addWidget(active);
	QLineEdit* title = new QLineEdit(this);
	title->setObjectName("Inspector_ObjectName");
	title->setText(obj->GetObjectName().c_str());
	horLayout->addWidget(title);
	horLayout->setStretch(0, 0);
	horLayout->setStretch(1, 10);
	connect(title, &QLineEdit::editingFinished, this,
		[title , this]() {
			if (title && !_currentGameObject.expired()&& _currentGameObject.lock()->GetObjectName() != title->text().toStdString())
			{
				_currentGameObject.lock()->SetObjectName(title->text().toStdString().c_str());
				title->clearFocus();
			}
		});
	ToolBox* box = new ToolBox(this);
	_layoutMain->addWidget(box);
	//---------------------- Transform
	box->addPage("Transform", true);
	{
		QWidget* transformWidget = new QWidget(this);
		transformWidget->setLayout(new QVBoxLayout(this));
		//box->addSubWidget("Transform", transformWidget);
		box->addSubWidget(transformWidget);
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
		box->addPage(c->GetComponentName().c_str(), true);
		box->addSubWidget(compWidget);
		auto pro = c->GetProperties();
		for (auto p : pro)
		{
			//Bool value
			if (IsType("bool"))
			{
				if (p.bArray)
				{
				}
				else
				{
					auto value = (bool*)p.value;
					CheckBox* checkBox = new CheckBox(p.name, this, c->IsActive());
					checkBox->_callback = [comp](bool b) {
						if (comp)
							comp->SetActive(b);
					};
					checkBox->_boolBind = value;
					compWidget->layout()->addWidget(checkBox);
				}
				continue;
			}
			//std::weak_ptr<Model> value
			else if (IsType("AssetRef"))
			{
				if (p.bArray)
				{
					std::vector<AssetRef>* refs = (std::vector<AssetRef>*)p.value;
					ToolBox* cw = new ToolBox(p.name.c_str(), true, this);
					compWidget->layout()->addWidget(cw);
					for (int i = 0; i < refs->size(); i++)
					{
						HString text = "";
						if (!refs->at(i).assetInfo.expired())
						{
							text = refs->at(i).assetInfo.lock()->virtualFilePath;
						}
						AssetLine* line = new AssetLine(p.name, this, text, p.condition);
						cw->addSubWidget(line);
						//component callback
						refs->at(i).callBack = [line, refs,i]() {
							if (!refs->at(i).assetInfo.expired())
							{
								line->ui.LineEdit->setText(refs->at(i).assetInfo.lock()->virtualFilePath.c_str());
							}
							else
							{
								line->ui.LineEdit->setText("");
							}
						};
						//查找按钮
						line->_bindFindButtonFunc = [refs, i](const char* p) {
							auto assetInfo = refs->at(i).assetInfo;
							if (!assetInfo.expired())
							{
								auto treeItems = assetInfo.lock()->virtualPath.Split("/");
								QString path;
								CustomViewItem* treeItem = nullptr;
								QModelIndex treeIndex;
								for (auto& i : treeItems)
								{
									path += ("/" + i).c_str();
									treeItem = ContentBrowser::GetCurrentBrowser()->_treeView->FindItem(path);
									if (treeItem)
									{
										ContentBrowser::GetCurrentBrowser()->_treeView->_bSaveSelectionItem = true;
										treeIndex = ((QStandardItemModel*)ContentBrowser::GetCurrentBrowser()->_treeView->model())->indexFromItem(treeItem);
										ContentBrowser::GetCurrentBrowser()->_treeView->expand(treeIndex);
									}
								}
								if (treeIndex.isValid())
								{
									ContentBrowser::GetCurrentBrowser()->_treeView->selectionModel()->setCurrentIndex(treeIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect);
								}
								auto item = ContentBrowser::GetCurrentBrowser()->_listView->FindAssetItem(assetInfo.lock()->guid);
								if (item)
								{
									ContentBrowser::GetCurrentBrowser()->_listView->scrollToItem(item);
									ContentBrowser::GetCurrentBrowser()->_listView->setCurrentItem(item, QItemSelectionModel::SelectionFlag::ClearAndSelect);
								}
							}
						};
						//路径发生变化的时候执行
						line->_bindAssetPath = [p, refs, i](const char* s) {
							refs->at(i).path = s;
							p.comp->UpdateData();
						};
					}
				}
				else
				{
					AssetRef* ref = (AssetRef*)p.value;
					HString text = "";
					if (!ref->assetInfo.expired())
					{
						text = ref->assetInfo.lock()->virtualFilePath;
					}
					AssetLine* line = new AssetLine(p.name, this, text, p.condition);
					compWidget->layout()->addWidget(line);
					//component callback
					ref->callBack = [line,ref]() {
						if (!ref->assetInfo.expired())
						{
							line->ui.LineEdit->setText(ref->assetInfo.lock()->virtualFilePath.c_str());
						}
						else
						{
							line->ui.LineEdit->setText("");
						}
					};
					//查找按钮
					line->_bindFindButtonFunc = [ref](const char* p) {
						auto assetInfo = ref->assetInfo;
						if (!assetInfo.expired())
						{
							auto treeItems = assetInfo.lock()->virtualPath.Split("/");
							QString path;
							CustomViewItem* treeItem = nullptr;
							QModelIndex treeIndex;
							for (auto& i : treeItems)
							{
								path += ("/" + i).c_str();
								treeItem = ContentBrowser::GetCurrentBrowser()->_treeView->FindItem(path);
								if (treeItem)
								{
									ContentBrowser::GetCurrentBrowser()->_treeView->_bSaveSelectionItem = true;
									treeIndex = ((QStandardItemModel*)ContentBrowser::GetCurrentBrowser()->_treeView->model())->indexFromItem(treeItem);
									ContentBrowser::GetCurrentBrowser()->_treeView->expand(treeIndex);
								}
							}
							if (treeIndex.isValid())
							{
								ContentBrowser::GetCurrentBrowser()->_treeView->selectionModel()->setCurrentIndex(treeIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect);
							}
							auto item = ContentBrowser::GetCurrentBrowser()->_listView->FindAssetItem(assetInfo.lock()->guid);
							if (item)
							{
								ContentBrowser::GetCurrentBrowser()->_listView->scrollToItem(item);
								ContentBrowser::GetCurrentBrowser()->_listView->setCurrentItem(item, QItemSelectionModel::SelectionFlag::ClearAndSelect);
							}
						}

					};
					//路径发生变化的时候执行
					line->_bindAssetPath = [p, ref](const char* s) {
						ref->path = s;
						p.comp->UpdateData();
					};
				}
				continue;
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
		_currentInspector = nullptr;
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
