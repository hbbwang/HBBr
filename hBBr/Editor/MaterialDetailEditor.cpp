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
QList<MaterialDetailEditor*>MaterialDetailEditor::_allDetailWindows;

MaterialDetailEditor::MaterialDetailEditor(std::weak_ptr<Material> mat, QWidget *parent)
	: QMainWindow(parent)
{
	_parent = (MaterialEditor*)parent;
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	QTimer* t = new QTimer(this);
	t->setSingleShot(true);
	t->setInterval(100);

	connect(t, &QTimer::timeout, this, [this,mat](){
		for (int i = 0; i < _allDetailWindows.size(); i++)
		{
			if (_allDetailWindows[i]->_material.lock().get() == mat.lock().get())
			{
				this->close();
				return;
			}
		}
		setObjectName("MaterialDetailEditor");
		setWindowTitle(mat.lock()->_assetInfo.lock()->displayName.c_str());

		setWindowFlag(Qt::Window, true);
		_material = mat;
		_allDetailWindows.append(this);

		LoadEditorWindowSetting(this, "MaterialEditor");





		QWidget* r = new QWidget(this);
		QWidget* ma = new QWidget(this);
		QWidget* mp = new QWidget(this);
		r->setObjectName("MaterialDetailEditor");
		ma->setObjectName("MaterialDetailEditor");
		mp->setObjectName("MaterialDetailEditor");
		ui_r.setupUi(r);
		ui_ma.setupUi(ma);
		ui_mp.setupUi(mp);

		left_right = new QSplitter(this);
		left_right->setOrientation(Qt::Orientation::Horizontal);
		QWidget* widget = new QWidget(this);
		widget->setLayout(new QVBoxLayout(widget));
		((QVBoxLayout*)widget->layout())->setContentsMargins(0, 0, 0, 0);
		((QVBoxLayout*)widget->layout())->setSpacing(0);
		left_right->addWidget(widget);
		left_right->addWidget(mp);
		//
		QSplitter* up_bottom = new QSplitter(this);
		up_bottom->setOrientation(Qt::Orientation::Vertical);
		widget->layout()->addWidget(up_bottom);
		up_bottom->addWidget(r);
		up_bottom->addWidget(ma);
		//小标题
		ui_r.MaterialDetailEditor_RendererName->setText(GetEditorInternationalization("MaterialEditor", "MaterialRendererTitle"));
		ui_mp.MaterialDetailEditor_MaterialParameterName->setText(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
		ui_ma.MaterialDetailEditor_MaterialAttributeName->setText(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));
		//渲染器
		{
			_renderer = new SDLWidget(this, _material.lock()->_assetInfo.lock()->guid.str().c_str());
			ui_r.verticalLayout_2->addWidget(_renderer);
			ui_r.verticalLayout_2->setStretch(1,10);
			//HWND hwnd = (HWND)VulkanApp::GetWindowHandle(_matWindow);
			auto renderer = _renderer->_rendererForm->renderer;
			//渲染器需要一帧时间去创建，所以下一帧执行
			auto func = [this]()
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
		//Material attribute
		PropertyWidget* pw_ma = new PropertyWidget(this);
		ui_ma.scroll_verticalLayout->addWidget(pw_ma);
		{
			//ui_ma.scroll_verticalLayout
			//material path text
			{
				AssetLine* path = new AssetLine(this, "");
				pw_ma->AddItem("Virtual Path", path);
				path->ui.LineEdit->setReadOnly(true);
				path->ui.LineEdit->setText(_material.lock()->_assetInfo.lock()->virtualFilePath.c_str());
				path->ui.pushButton->setHidden(true);
				path->_bindFindButtonFunc = [this](const char* p) {
					auto assetInfo = _material.lock()->_assetInfo;
					ContentBrowser::FocusToAsset(assetInfo);
				};
			}
			//Save
			{
				QToolButton* saveButton = new QToolButton(this);
				pw_ma->AddItem("", saveButton);
				saveButton->setText(GetEditorInternationalization("MaterialEditor", "SaveButton"));
				connect(saveButton, &QAbstractButton::clicked, this, [this]() {
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
					HString shader_name = i.second->shaderName;
					auto it = std::find_if(shaderNames.begin(), shaderNames.end(), [shader_name](HString& s) {
						return s == shader_name;
						});
					if (it == shaderNames.end())
					{
						shaderNames.push_back(shader_name);
						comboBox->AddItem(shader_name.c_str());
					}
				}
				comboBox->SetCurrentSelection(_material.lock()->GetPrimitive()->psShader.c_str());
			}
			//Refresh shader button
			{
				QToolButton* refreshButton = new QToolButton(this);
				pw_ma->AddItem("", refreshButton);
				refreshButton->setText(GetEditorInternationalization("MaterialEditor", "RefreshShaderButton"));
				connect(refreshButton, &QAbstractButton::clicked, this, [this]() 
					{
						ConsoleDebug::printf_endl(
							HString(GetEditorInternationalization("MaterialEditor", "RefreshShader").toStdString()),
							_material.lock()->GetPrimitive()->vsShader.c_str(),
							_material.lock()->GetPrimitive()->psShader.c_str());
						Shader::ReloadMaterialShaderCacheAndPipelineObject(_material);
						//重新打开，主要为了重新加载参数页面
						RefreshMaterialEditor(_material);
					});
			}
			ui_ma.scroll_verticalLayout->addStretch(20);
			pw_ma->ShowItems();
		}

		PropertyWidget* pw_mp = new PropertyWidget(this);
		ui_mp.scroll_verticalLayout->addWidget(pw_mp);
		auto prim = _material.lock()->GetPrimitive();
		//Material parameter
		//Uniform buffer
		{
			auto mp_group = pw_mp->AddGroup(GetEditorInternationalization("MaterialEditor", "MaterialUniformBufferTitle"));
			for (auto& i : prim->_paramterInfos)
			{
				auto mp_sub_group = pw_mp->AddGroup(i.group.c_str(), mp_group);
				VectorSetting* vector = nullptr;
				int vecType = 1;
				if (i.type == MPType::Float)
				{
					vector = new VectorSetting(this, 1, 0.0001f, 8);
					vector->SetValue(prim->uniformBuffer[i.arrayIndex][i.vec4Index]);
					vector->_vec4_f[0] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index];
				}
				else if (i.type == MPType::Float2)
				{
					vector = new VectorSetting(this, 2, 0.0001f, 8);
					vecType = 2;
					vector->SetValue(
						prim->uniformBuffer[i.arrayIndex][i.vec4Index],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1],
						0
					);
					vector->_vec4_f[0] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index];
					vector->_vec4_f[1] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1];
				}
				else if (i.type == MPType::Float3)
				{
					vector = new VectorSetting(this, 3, 0.0001f, 8);
					vecType = 3;
					vector->SetValue(
						prim->uniformBuffer[i.arrayIndex][i.vec4Index],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 2]
					);
					vector->_vec4_f[0] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index];
					vector->_vec4_f[1] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1];
					vector->_vec4_f[2] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 2];
				}
				else if (i.type == MPType::Float4)
				{
					vector = new VectorSetting(this, 4, 0.0001f, 8);
					vecType = 4;
					vector->SetValue(
						prim->uniformBuffer[i.arrayIndex][i.vec4Index],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 2],
						prim->uniformBuffer[i.arrayIndex][i.vec4Index + 3]
					);
					vector->_vec4_f[0] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index];
					vector->_vec4_f[1] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 1];
					vector->_vec4_f[2] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 2];
					vector->_vec4_f[3] = &prim->uniformBuffer[i.arrayIndex][i.vec4Index + 3];
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
					AssetLine* line = new AssetLine(this, assetInfo.lock()->virtualFilePath, "dds");
					//查找按钮
					line->_bindFindButtonFunc = 
						[](const char* p)
						{
							std::weak_ptr<AssetInfoBase> assetInfo = ContentManager::Get()->GetAssetByVirtualPath(p);
							ContentBrowser::FocusToAsset(assetInfo);
						};
					//路径发生变化的时候执行
					line->_bindAssetPath =
						[this,i](const char* s) 
						{
							auto newTexInfo = ContentManager::Get()->GetAssetByVirtualPath(s);
							if (!newTexInfo.expired())
							{
								_material.lock()->GetPrimitive()->SetTexture(i.index, newTexInfo.lock()->GetAssetObject<Texture2D>().lock().get());
							}
						};
					pw_mp->AddItem(i.name.c_str(), line, 30, mt_sub_group);
				}
			}
		}
		pw_mp->ShowItems();
		ui_mp.scroll_verticalLayout->addStretch(20);







		show();
	});
	t->start();
}

MaterialDetailEditor::~MaterialDetailEditor()
{
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
	CloseMaterialEditor(mat);
	return OpenMaterialEditor(mat);
}

void MaterialDetailEditor::resizeEvent(QResizeEvent * event)
{
	left_right->resize(width(), height());
}

void MaterialDetailEditor::closeEvent(QCloseEvent* event)
{
	if(_renderer && _renderer->_rendererForm)
		VulkanApp::RemoveWindow(_renderer->_rendererForm);
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
