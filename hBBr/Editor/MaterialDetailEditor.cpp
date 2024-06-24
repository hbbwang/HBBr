#include "MaterialDetailEditor.h"
#include "FormMain.h"
#include <MaterialEditor.h>
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
MaterialDetailEditor::MaterialDetailEditor(std::weak_ptr<Material> mat, QWidget *parent)
	: QMainWindow(parent)
{
	QWidget* r = new QWidget(this);
	QWidget* ma = new QWidget(this);
	QWidget* mp = new QWidget(this);
	ui_r.setupUi(r);
	ui_ma.setupUi(ma);
	ui_mp .setupUi(mp);
	_parent = (MaterialEditor*)parent;
	_material = mat;

	left_right = new QSplitter(this);
	left_right->setOrientation(Qt::Orientation::Horizontal);
	QWidget* widget = new QWidget(this);
	widget->setLayout(new QVBoxLayout(this));
	((QVBoxLayout*)widget->layout())->setContentsMargins(0,0,0,0);
	((QVBoxLayout*)widget->layout())->setSpacing(0);
	left_right->addWidget(widget);
	left_right->addWidget(mp);
	//
	QSplitter* up_bottom = new QSplitter(this);
	up_bottom->setOrientation(Qt::Orientation::Vertical);
	widget->layout()->addWidget(up_bottom);
	up_bottom->addWidget(r);
	up_bottom->addWidget(ma);
	//Ð¡±êÌâ
	ui_r.MaterialDetailEditor_RendererName->setText(GetEditorInternationalization("MaterialEditor","MaterialRendererTitle"));
	ui_mp.MaterialDetailEditor_MaterialParameterName->setText(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
	ui_ma.MaterialDetailEditor_MaterialAttributeName->setText(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));
	//äÖÈ¾Æ÷
	{
		_matWindow = VulkanApp::CreateNewWindow(512, 512, _material.lock()->_assetInfo.lock()->guid.str().c_str(), true, (void*)ui_r.MaterialDetailEditor_RenderView->winId());
		//HWND hwnd = (HWND)VulkanApp::GetWindowHandle(_matWindow);
		auto renderer = _matWindow->renderer;
		auto func = [this]() 
			{
				auto renderer = _matWindow->renderer;
				_gameObject = renderer->GetWorld()->SpawnGameObject("PreviewObject", renderer->GetWorld()->_editorLevel.get());
				auto modelComp = _gameObject->AddComponent<ModelComponent>();
				modelComp->SetModel(HGUID("6146e15a-0632-b197-b6f9-b12fc8a16b05"));
				for (int i = 0; i < modelComp->GetMaterialNum(); i++)
				{
					modelComp->SetMaterial(_material, i);
				}
				renderer->GetWorld()->GetMainCamera()->_cameraType = EditorCameraType::TargetRotation;
				renderer->GetWorld()->GetMainCamera()->GetTransform()->SetWorldLocation(glm::vec3(0, 0, -2.0f));
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
			path->ui.picture->setHidden(true);
			path->ui.pushButton->setHidden(true);	
			path->_bindFindButtonFunc = [this](const char* p) {
				auto assetInfo = _material.lock()->_assetInfo;
				ContentBrowser::FocusToAsset(assetInfo);
			};
		}
		//Refresh shader button
		{
			QToolButton* refreshButton = new QToolButton(this);
			pw_ma->AddItem("", refreshButton);
			refreshButton->setText(GetEditorInternationalization("MaterialEditor", "RefreshShaderButton"));
			connect(refreshButton, &QAbstractButton::clicked, this, [this]() {
				ConsoleDebug::printf_endl(
					HString(GetEditorInternationalization("MaterialEditor", "RefreshShader").toStdString()),
					_material.lock()->GetPrimitive()->vsShader.c_str(),
					_material.lock()->GetPrimitive()->psShader.c_str());
				Shader::ReloadMaterialShaderCacheAndPipelineObject(_material);
			});
		}
		//
		{

		}
		ui_ma.scroll_verticalLayout->addStretch(20);
	}

	//Material parameter
	//Uniform buffer
	{
		ToolBox* ubBox = new ToolBox(GetEditorInternationalization("MaterialEditor", "MaterialUniformBufferTitle"), true, this);
		ui_mp.scroll_verticalLayout->addWidget(ubBox);
		for (auto& i : _material.lock()->GetTextures())
		{
			
		}
	}
	//Texture
	{
		ToolBox* ubBox = new ToolBox(GetEditorInternationalization("MaterialEditor", "MaterialTextureTitle"), true, this);
		ui_mp.scroll_verticalLayout->addWidget(ubBox);

	}
	ui_mp.scroll_verticalLayout->addStretch(20);
}

MaterialDetailEditor::~MaterialDetailEditor()
{
}

void MaterialDetailEditor::resizeEvent(QResizeEvent * event)
{
	left_right->resize(width(), height());
}

void MaterialDetailEditor::closeEvent(QCloseEvent* event)
{
	if(_matWindow)
		VulkanApp::RemoveWindow(_matWindow);

	for (int i = 0 ; i < _parent->_allDetailWindows.size() ; i++)
	{
		if (_parent->_allDetailWindows[i].editor == this)
		{
			_parent->_allDetailWindows.removeAt(i);
			break;
		}
	}
}
