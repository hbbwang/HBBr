#include "MaterialDetailEditor.h"
#include "FormMain.h"
#include <qlayout.h>
#include <qboxlayout.h>
#include <EditorCommonFunction.h>
#include <AssetLine.h>
#include <ContentBrowser.h>
#include <VulkanRenderer.h>
#include <Pipeline.h>
#include <ShaderCompiler.h>
#include <Shader.h>
#include <World.h>
#include <Level.h>
#include <GameObject.h>
#include <ModelComponent.h>
#include "PropertyWidget.h"
#include <CameraComponent.h>
#include <ToolBox.h>
#include <QMap>
#include <QString>
#include <VectorSetting.h>
#include <ComboBox.h>
#include <QTimer>
#include <EditorMain.h>
#include <SDLWidget.h>
#include "Asset/Texture2D.h"
#include "Asset/TextureCube.h"
QList<MaterialDetailEditor*>MaterialDetailEditor::_allDetailWindows;
MaterialDetailEditor::MaterialDetailEditor(std::weak_ptr<Material> mat, QWidget *parent)
	: QMainWindow(parent)
{
	_material = mat;
	_parent = (MaterialEditor*)parent;
	//this->setAttribute(Qt::WA_DeleteOnClose, true);

	_left_right_sizes = QList<int>() << this->width() * (2.0f / 5.0f) << this->width() * (3.0f / 5.0f);
	_up_bottom_sizes = QList<int>() << this->width() * (2.0f / 5.0f) << this->width() * (3.0f / 5.0f);

	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_material.lock().get() == _material.lock().get())
		{
			this->close();
			return;
		}
	}
	setObjectName("MaterialDetailEditor");
	setWindowTitle(_material.lock()->_assetInfo.lock()->displayName.c_str());
	setWindowFlag(Qt::Window, true);
	_allDetailWindows.append(this);
	LoadEditorWindowSetting(this, "MaterialEditor");


	//setLayout(new QHBoxLayout(this));
	//((QHBoxLayout*)layout())->setContentsMargins(0, 0, 0, 0);
	//((QHBoxLayout*)layout())->setSpacing(0);


	Init();

	show();
}

MaterialDetailEditor::~MaterialDetailEditor()
{
}

void deleteItem(QLayout* layout)
{
	if (layout == NULL)
		return;
	QLayoutItem* child;
	while ((child = layout->takeAt(0)) != nullptr)
	{
		//setParent为NULL，防止删除之后界面不消失
		if (child->widget())
		{
			child->widget()->setParent(nullptr);
			delete child->widget();
		}
		else if (child->layout())
		{
			deleteItem(child->layout());
			delete child->layout();
		}
		delete child;
	}
	if (layout->widget())
	{
		layout->widget()->setLayout(nullptr);
	}
}

MaterialDetailEditor* MaterialDetailEditor::OpenMaterialEditor(std::weak_ptr<Material> mat)
{
	return new MaterialDetailEditor(mat, EditorMain::_self);
}

void MaterialDetailEditor::CloseMaterialEditor(std::weak_ptr<Material> mat)
{
	MaterialDetailEditor* editor = nullptr;
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_material.lock().get() == mat.lock().get())
		{
			editor = _allDetailWindows[i];
			break;
		}
	}
	if (editor != nullptr)
	{
		editor->close();
	}
}

MaterialDetailEditor* MaterialDetailEditor::RefreshMaterialEditor(std::weak_ptr<Material> mat)
{
	//CloseMaterialEditor(mat);
	//return OpenMaterialEditor(mat);
	MaterialDetailEditor* editor = nullptr;
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_material.lock().get() == mat.lock().get())
		{
			editor = _allDetailWindows[i];
			break;
		}
	}
	if (editor != nullptr)
	{
		delete editor->mp;
		delete editor->ma;

		editor->Init();
	}
	return editor;
}

void MaterialDetailEditor::RefreshAllMaterialEditor()
{
	std::vector<std::weak_ptr<Material>> _allMat;
	auto allEditorCache = _allDetailWindows;
	_allMat.reserve(allEditorCache.size());
	for (int i = 0; i < allEditorCache.size(); i++)
	{
		_allMat.push_back(allEditorCache[i]->_material);
	}

	for (auto& i : _allMat)
	{
		RefreshMaterialEditor(i);
	}
}

void MaterialDetailEditor::paintEvent(QPaintEvent* event)
{
}

void MaterialDetailEditor::closeEvent(QCloseEvent* event)
{
	if (_renderer && _renderer->_rendererForm)
	{
		VulkanApp::RemoveWindow(_renderer->_rendererForm);
		_renderer->_rendererForm = nullptr;
	}
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_material.lock().get() == _material.lock().get())
		{
			_allDetailWindows.removeAt(i);
			break;
		}
	}
	SaveEditorWindowSetting(this, "MaterialEditor");
}

void MaterialDetailEditor::Init()
{
	/*_inspector = new Inspector(this);
	_inspector_dock = new CustomDockWidget(this);
	_inspector_dock->setFeatures(
		QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	_inspector_dock->setWidget(_inspector);
	_inspector->setObjectName("Inspector");
	_inspector_dock->setWindowTitle(GetEditorInternationalization("MainWindow", "Inspector"));
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, _inspector_dock);*/

	//left_right = new QSplitter(this);
	//left_right->setChildrenCollapsible(false);
	//left_right->setOrientation(Qt::Orientation::Horizontal);

	up_bottom = new QSplitter(this);
	up_bottom->setOrientation(Qt::Orientation::Vertical);
	setCentralWidget(up_bottom);

	r = new QWidget(this);
	r->setObjectName("MaterialDetailEditor");
	ui_r.setupUi(r);
	up_bottom->addWidget(r);

	InitMA();
	InitMP();

	//小标题
	ui_r.MaterialDetailEditor_RendererName->setText(GetEditorInternationalization("MaterialEditor", "MaterialRendererTitle"));
	//渲染器
	{
		_renderer = new SDLWidget(this, _material.lock()->_assetInfo.lock()->guid.str().c_str());
		//HWND hwnd = (HWND)VulkanApp::GetWindowHandle(_matWindow);
		ui_r.verticalLayout_2->addWidget(_renderer);
		ui_r.verticalLayout_2->setStretch(1, 10);
		auto renderer = _renderer->_rendererForm->renderer;
		//渲染器需要一帧时间去创建，所以下一帧执行
		auto func =
			[this]()
			{
				auto renderer = _renderer->_rendererForm->renderer;
				renderer->GetWorld().lock()->SetWorldName("Material Editor Renderer");
				renderer->GetWorld().lock()->GetMainCamera()->_cameraType = EditorCameraType::TargetRotation;
				renderer->GetWorld().lock()->GetMainCamera()->GetTransform()->SetWorldLocation(glm::vec3(0, 0, -2.0f));

				_gameObject = renderer->GetWorld().lock()->SpawnGameObject("PreviewObject", renderer->GetWorld().lock()->_editorLevel.get());
				auto modelComp = _gameObject->AddComponent<ModelComponent>();
				modelComp->SetModel(HGUID("6146e15a-0632-b197-b6f9-b12fc8a16b05"));
				for (int i = 0; i < modelComp->GetMaterialNum(); i++)
				{
					modelComp->SetMaterial(_material, i);
				}
			};
		renderer->ExecFunctionOnRenderThread(func);
	}
	up_bottom->setSizes(_up_bottom_sizes);
}

void MaterialDetailEditor::InitMP()
{
	mp = new QDockWidget(this);
	mp->setObjectName("MaterialDetailEditor");
	mp->setFeatures(QDockWidget::NoDockWidgetFeatures);

	QWidget* widget = new QWidget(mp);
	mp->setWidget(widget);
	ui_mp.setupUi(widget);
	mp->setWindowTitle(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
	//ui_mp.MaterialDetailEditor_MaterialParameterName->setText(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
	ui_mp.MaterialDetailEditor_MaterialParameterName->setHidden(true);
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, mp);
	//
	PropertyWidget* pw_mp = new PropertyWidget(this);
	ui_mp.scroll_verticalLayout->addWidget(pw_mp);
	auto prim = _material.lock()->GetPrimitive();
	//Material parameter
	auto mp_main_group = pw_mp->AddGroup(GetEditorInternationalization("MaterialEditor", "MaterialUniformBufferTitle"));
	//Uniform buffer VS
	{
		QString shaderGroupName = GetEditorInternationalization("MaterialEditor", "VertexUniformBuffer");
		auto mp_shader = pw_mp->AddGroup(shaderGroupName, mp_main_group);
		for (auto& i : prim->_paramterInfos_vs)
		{
			auto mp_sub_group = pw_mp->AddGroup(i.group.c_str(), mp_shader);
			VectorSetting* vector = nullptr;
			int vecType = 1;
			if (i.type == MPType::VSFloat)
			{
				vector = new VectorSetting(this, 1, 0.0001f, 8);
				vector->SetValue(prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index]);
				vector->_vec4_f[0] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index];
			}
			else if (i.type == MPType::VSFloat2)
			{
				vector = new VectorSetting(this, 2, 0.0001f, 8);
				vecType = 2;
				vector->SetValue(
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1],
					0
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1];
			}
			else if (i.type == MPType::VSFloat3)
			{
				vector = new VectorSetting(this, 3, 0.0001f, 8);
				vecType = 3;
				vector->SetValue(
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 2]
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1];
				vector->_vec4_f[2] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 2];
			}
			else if (i.type == MPType::VSFloat4)
			{
				vector = new VectorSetting(this, 4, 0.0001f, 8);
				vecType = 4;
				vector->SetValue(
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 2],
					prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 3]
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 1];
				vector->_vec4_f[2] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 2];
				vector->_vec4_f[3] = &prim->uniformBuffer_vs[i.arrayIndex][i.vec4Index + 3];
			}
			pw_mp->AddItem(i.name.c_str(), vector, 30, mp_sub_group);
		}
	}
	//Uniform buffer PS
	{
		QString shaderGroupName = GetEditorInternationalization("MaterialEditor", "PixelUniformBuffer");
		auto mp_shader = pw_mp->AddGroup(shaderGroupName, mp_main_group);
		for (auto& i : prim->_paramterInfos_ps)
		{
			auto mp_sub_group = pw_mp->AddGroup(i.group.c_str(), mp_shader);
			VectorSetting* vector = nullptr;
			int vecType = 1;
			if (i.type == MPType::PSFloat)
			{
				vector = new VectorSetting(this, 1, 0.0001f, 8);
				vector->SetValue(prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index]);
				vector->_vec4_f[0] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index];
			}
			else if (i.type == MPType::PSFloat2)
			{
				vector = new VectorSetting(this, 2, 0.0001f, 8);
				vecType = 2;
				vector->SetValue(
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1],
					0
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1];
			}
			else if (i.type == MPType::PSFloat3)
			{
				vector = new VectorSetting(this, 3, 0.0001f, 8);
				vecType = 3;
				vector->SetValue(
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 2]
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1];
				vector->_vec4_f[2] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 2];
			}
			else if (i.type == MPType::PSFloat4)
			{
				vector = new VectorSetting(this, 4, 0.0001f, 8);
				vecType = 4;
				vector->SetValue(
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 2],
					prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 3]
				);
				vector->_vec4_f[0] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index];
				vector->_vec4_f[1] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 1];
				vector->_vec4_f[2] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 2];
				vector->_vec4_f[3] = &prim->uniformBuffer_ps[i.arrayIndex][i.vec4Index + 3];
			}
			pw_mp->AddItem(i.name.c_str(), vector, 30, mp_sub_group);
		}
	}
	//Texture
	{
		auto mt_group = pw_mp->AddGroup(GetEditorInternationalization("MaterialEditor", "MaterialTextureTitle"));
		for (auto i : prim->_textureInfos)
		{
			auto mt_sub_group = pw_mp->AddGroup(i.group.c_str(), mt_group);
			std::weak_ptr<AssetInfoBase> assetInfo = prim->GetTextures()[i.index]->_assetInfo;
			if (!assetInfo.expired())
			{
				AssetLine* line = new AssetLine(this, assetInfo.lock()->virtualFilePath, assetInfo.lock()->type);
				//查找按钮
				line->_bindFindButtonFunc =
					[](const char* p)
					{
						std::weak_ptr<AssetInfoBase> assetInfo = ContentManager::Get()->GetAssetByVirtualPath(p);
						ContentBrowser::FocusToAsset(assetInfo);
					};
				//路径发生变化的时候执行
				line->_bindAssetPath =
					[this, i, line](const char* s)
					{
						auto newTexInfo = ContentManager::Get()->GetAssetByVirtualPath(s);
						if (!newTexInfo.expired())
						{
							std::weak_ptr<Texture2D> getTex; 
							if (newTexInfo.lock()->type == AssetType::Texture2D)
							{
								getTex = newTexInfo.lock()->GetAssetObject<Texture2D>();
							}
							else if (newTexInfo.lock()->type == AssetType::TextureCube)
							{
								getTex = newTexInfo.lock()->GetAssetObject<TextureCube>();
							}
							if (!getTex.expired())
							{
								line->ui.LineEdit->setText(s);
								_material.lock()->GetPrimitive()->SetTexture(i.index, getTex.lock().get());
							}
						}
					};
				pw_mp->AddItem(i.name.c_str(), line, 30, mt_sub_group);
			}
		}
	}
	pw_mp->ShowItems();
	ui_mp.scroll_verticalLayout->addStretch(20);
}

void MaterialDetailEditor::InitMA()
{
	ma = new QWidget(this);
	ui_ma.setupUi(ma);
	ui_ma.MaterialDetailEditor_MaterialAttributeName->setText(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));
	ma->setWindowTitle(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));

	up_bottom->addWidget(ma);

	pw_ma = new PropertyWidget(this);
	ui_ma.scroll_verticalLayout->addWidget(pw_ma);
	ui_ma.scroll_verticalLayout->addStretch(20);
	{
		//ui_ma.scroll_verticalLayout
		//material path text
		{
			AssetLine* path = new AssetLine(this, "");
			pw_ma->AddItem("Virtual Path", path);
			path->ui.LineEdit->setReadOnly(true);
			path->ui.LineEdit->setText(_material.lock()->_assetInfo.lock()->virtualFilePath.c_str());
			path->ui.pushButton->setHidden(true);
			path->_bindFindButtonFunc = 
			[this](const char* p)
			{
				auto assetInfo = _material.lock()->_assetInfo;
				ContentBrowser::FocusToAsset(assetInfo);
			};
		}
		//Save
		{
			QToolButton* saveButton = new QToolButton(this);
			pw_ma->AddItem("", saveButton);
			saveButton->setText(GetEditorInternationalization("MaterialEditor", "SaveButton"));
			connect(saveButton, &QAbstractButton::clicked, this,
			[this]() {
				_material.lock()->SaveAsset(_material.lock()->_assetInfo.lock()->absFilePath);
			});
		}
		// Shader Selection
		{
			ComboBox* comboBox = new ComboBox(this);
			pw_ma->AddItem("Shader", comboBox);
			std::vector<HString> shaderNames;
			for (auto& i : Shader::GetPSCaches())
			{
				if (i.second->header.flags & HideInEditor)
				{
					continue;
				}
				HString shader_name = i.second->shaderName;
				auto it = std::find_if(shaderNames.begin(), shaderNames.end(), 
				[shader_name](HString& s) {
					return s == shader_name;
				});
				if (it == shaderNames.end())
				{
					shaderNames.push_back(shader_name);
					comboBox->AddItem(shader_name.c_str());
				}
			}
			comboBox->SetCurrentSelection(_material.lock()->GetPrimitive()->graphicsIndex.GetPSShaderName().c_str());
			comboBox->_bindCurrentTextChanged = 
				[](int index, const char* s) 
				{
					//切换shader的时候，所有变体都使用默认的
					auto vs_cache_it = Shader::GetVSCache(HString(s) + "@0");
					auto ps_cache_it = Shader::GetPSCache(HString(s) + "@0");
					//if(vs_cache_it)
				};
		}
		//Refresh shader button
		{
			QToolButton* refreshButton = new QToolButton(this);
			pw_ma->AddItem("", refreshButton);
			refreshButton->setText(GetEditorInternationalization("MaterialEditor", "RefreshShaderButton"));
			connect(refreshButton, &QAbstractButton::clicked, this, 
			[this]()
			{
				ConsoleDebug::printf_endl(
					HString(GetEditorInternationalization("MaterialEditor", "RefreshShader").toStdString()),
					_material.lock()->GetPrimitive()->graphicsIndex.GetVSShaderName().c_str(),
					_material.lock()->GetPrimitive()->graphicsIndex.GetPSShaderName().c_str());
				Shader::ReloadMaterialShaderCacheAndPipelineObject(_material);
				//重新打开，主要为了重新加载参数页面
				RefreshMaterialEditor(_material);
			});
		}
		pw_ma->ShowItems();
	}
}
