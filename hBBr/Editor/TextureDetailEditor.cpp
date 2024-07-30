#include "TextureDetailEditor.h"
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
QList<TextureDetailEditor*>TextureDetailEditor::_allDetailWindows;
TextureDetailEditor::TextureDetailEditor(std::weak_ptr<Texture2D> tex, QWidget *parent)
	: QMainWindow(parent)
{
	_texture = tex;
	//this->setAttribute(Qt::WA_DeleteOnClose, true);
	_left_right_sizes = QList<int>() << this->width() * (2.0f / 5.0f) << this->width() * (3.0f / 5.0f);

	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_texture.lock().get() == _texture.lock().get())
		{
			this->close();
			return;
		}
	}
	setObjectName("TextureDetailEditor");
	setWindowTitle(_texture.lock()->_assetInfo.lock()->displayName.c_str());
	setWindowFlag(Qt::Window, true);
	_allDetailWindows.append(this);
	LoadEditorWindowSetting(this, "TextureEditor");

	Init();

	show();
}

TextureDetailEditor::~TextureDetailEditor()
{
}

void TextureDetailEditor::deleteItem(QLayout* layout)
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
		child = nullptr;
	}
	if (layout->widget())
	{
		layout->widget()->setLayout(nullptr);
	}
}

TextureDetailEditor* TextureDetailEditor::OpenEditor(std::weak_ptr<Texture2D> mat)
{
	return new TextureDetailEditor(mat, EditorMain::_self);
}

void TextureDetailEditor::CloseEditor(std::weak_ptr<Texture2D> mat)
{
	TextureDetailEditor* editor = nullptr;
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_texture.lock().get() == mat.lock().get())
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

TextureDetailEditor* TextureDetailEditor::RefreshEditor(std::weak_ptr<Texture2D> mat)
{
	TextureDetailEditor* editor = nullptr;
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_texture.lock().get() == mat.lock().get())
		{
			editor = _allDetailWindows[i];
			break;
		}
	}
	if (editor != nullptr)
	{
		delete editor->_mp;
		editor->_mp = nullptr;
		editor->Init();
	}
	return editor;
}

void TextureDetailEditor::RefreshAllEditor()
{
	std::vector<std::weak_ptr<Texture2D>> _allMat;
	auto allEditorCache = _allDetailWindows;
	_allMat.reserve(allEditorCache.size());
	for (int i = 0; i < allEditorCache.size(); i++)
	{
		_allMat.push_back(allEditorCache[i]->_texture);
	}

	for (auto& i : _allMat)
	{
		RefreshEditor(i);
	}
}

void TextureDetailEditor::paintEvent(QPaintEvent* event)
{
}

void TextureDetailEditor::closeEvent(QCloseEvent* event)
{
	if (_renderer && _renderer->_rendererForm)
	{
		VulkanApp::RemoveWindow(_renderer->_rendererForm);
		_renderer->_rendererForm = nullptr;
	}
	for (int i = 0; i < _allDetailWindows.size(); i++)
	{
		if (_allDetailWindows[i]->_texture.lock().get() == _texture.lock().get())
		{
			_allDetailWindows.removeAt(i);
			break;
		}
	}
	SaveEditorWindowSetting(this, "TextureEditor");
}

void TextureDetailEditor::Init()
{
	left_right = new QSplitter(this);
	left_right->setOrientation(Qt::Orientation::Horizontal);
	setCentralWidget(left_right);

	//渲染器
	{
		_r = new QWidget(this);
		left_right->addWidget(_r);
		ui_r.setupUi(_r);
		//小标题
		ui_r.DetailEditor_RendererName->setText(GetEditorInternationalization("TextureEditor", "RendererTitle"));

		_renderer = new SDLWidget(this, _texture.lock()->_assetInfo.lock()->guid.str().c_str());
		ui_r.verticalLayout_2->addWidget(_renderer);
		ui_r.verticalLayout_2->setStretch(1, 10);
		auto renderer = _renderer->_rendererForm->renderer;
		//渲染器需要一帧时间去创建，所以下一帧执行
		auto func =
			[this]()
			{
				auto renderer = _renderer->_rendererForm->renderer;
				renderer->GetWorld().lock()->SetWorldName("Texture Editor Renderer");
				renderer->GetWorld().lock()->GetMainCamera()->_cameraType = EditorCameraType::TargetRotation;
				renderer->GetWorld().lock()->GetMainCamera()->GetTransform()->SetWorldLocation(glm::vec3(0, 0, -2.0f));

				_gameObject = renderer->GetWorld().lock()->SpawnGameObject("PreviewObject", renderer->GetWorld().lock()->_editorLevel.get());
				auto modelComp = _gameObject->AddComponent<ModelComponent>();
				modelComp->SetModel(HGUID("8b5fc385-2f6e-611e-415e-d5fa39875e50"));//Plane
			};
		renderer->ExecFunctionOnRenderThread(func);
	}
	//参数设置
	{
		_mp = new QWidget(this);
		left_right->addWidget(_mp);
		ui_mp.setupUi(_mp);
		//小标题
		ui_mp.DetailEditor_ParameterName->setText(GetEditorInternationalization("TextureEditor", "ParameterTitle"));
	}

	left_right->setSizes(_left_right_sizes);

}
