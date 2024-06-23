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
	//小标题
	ui_r.MaterialDetailEditor_RendererName->setText(GetEditorInternationalization("MaterialEditor","MaterialRendererTitle"));
	ui_mp.MaterialDetailEditor_MaterialParameterName->setText(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
	ui_ma.MaterialDetailEditor_MaterialAttributeName->setText(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));
	//渲染器
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

	PropertyWidget* pw = new PropertyWidget(this);
	ui_ma.scroll_verticalLayout->addWidget(pw);

	//Material attribute
	{
		//ui_ma.scroll_verticalLayout
		//path
		{
			AssetLine* path = new AssetLine("", this, "");
			pw->AddItem("Virtual Path", path);
			path->ui.LineEdit->setReadOnly(true);
			path->ui.LineEdit->setText(_material.lock()->_assetInfo.lock()->virtualFilePath.c_str());

			path->ui.horizontalSpacer->setGeometry(QRect(0, 0, 0, 0));
			path->ui.Name->setHidden(true);
			path->ui.picture->setHidden(true);
			path->ui.pushButton->setHidden(true);
			path->ui.horizontalLayout->setStretch(0, 0);
			path->ui.horizontalLayout->setStretch(1, 0);
			path->ui.horizontalLayout->setStretch(2, 0);
			path->ui.horizontalLayout->setStretch(3, 0);
			path->ui.horizontalLayout->setStretch(4, 10);
			path->ui.horizontalLayout->setStretch(5, 0);

			path->_bindFindButtonFunc = [this](const char* p) {
				auto assetInfo = _material.lock()->_assetInfo;
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
		}
		//Refresh shader
		{
			//((QHBoxLayout*)w->layout())->addStretch(100);
			QToolButton* refreshButton = new QToolButton(this);
			pw->AddItem("", refreshButton);
			refreshButton->setText(GetEditorInternationalization("MaterialEditor", "RefreshShaderButton"));
			connect(refreshButton, &QAbstractButton::clicked, this, [this]() {
				ConsoleDebug::printf_endl(
					HString(GetEditorInternationalization("MaterialEditor", "RefreshShader").toStdString()), 
					_material.lock()->GetPrimitive()->vsShader.c_str(),
					_material.lock()->GetPrimitive()->psShader.c_str());

				HString vs_n = _material.lock()->GetPrimitive()->graphicsIndex.GetVSShaderFullName();
				HString ps_n = _material.lock()->GetPrimitive()->graphicsIndex.GetVSShaderFullName();
				std::weak_ptr<ShaderCache> vs = Shader::GetVSCache(vs_n);
				std::weak_ptr<ShaderCache> ps = Shader::GetPSCache(ps_n);
				HString vs_shaderSourceName = vs.lock()->shaderName;
				HString ps_shaderSourceName = ps.lock()->shaderName;
				HString vs_shaderSourceFileName = vs.lock()->shaderName + ".fx";
				HString ps_shaderSourceFileName = ps.lock()->shaderName + ".fx";
				HString vs_cachePath = vs.lock()->shaderAbsPath;
				HString ps_cachePath = ps.lock()->shaderAbsPath;

				auto manager = VulkanManager::GetManager();
				manager->DeviceWaitIdle();

				//删除管线
				PipelineManager::RemovePipelineObjects(_material.lock()->GetPrimitive()->graphicsIndex);
				//重新编译所有变体
				vs_shaderSourceFileName = FileSystem::Append(FileSystem::GetShaderIncludeAbsPath(), vs_shaderSourceFileName);
				ps_shaderSourceFileName = FileSystem::Append(FileSystem::GetShaderIncludeAbsPath(), ps_shaderSourceFileName);
				Shaderc::ShaderCompiler::CompileShader(vs_shaderSourceFileName.c_str(), "VSMain", CompileShaderType::VertexShader);
				Shaderc::ShaderCompiler::CompileShader(ps_shaderSourceFileName.c_str(), "PSMain", CompileShaderType::PixelShader);
				//重新加载ShaderCache
				Shader::LoadShaderCacheByShaderName(FileSystem::GetShaderCacheAbsPath().c_str(), vs_shaderSourceName.c_str(), ShaderType::VertexShader);
				Shader::LoadShaderCacheByShaderName(FileSystem::GetShaderCacheAbsPath().c_str(), ps_shaderSourceName.c_str(), ShaderType::PixelShader);
				//需要重新加载对应材质
				auto allMaterials = ContentManager::Get()->GetAssets(AssetType::Material);
				for (auto& i : allMaterials)
				{
					auto matCache = i.second->GetAssetObject<Material>();
					if (
							matCache.lock()->GetPrimitive()->graphicsIndex.GetVSShaderName() == vs_shaderSourceName
						||	matCache.lock()->GetPrimitive()->graphicsIndex.GetPSShaderName() == ps_shaderSourceName
						)
					{
						i.second->NeedToReload();
						Material::LoadAsset(i.second->guid);
					}
				}
			});
		}
		//
		{

		}


		ui_ma.scroll_verticalLayout->addStretch(20);
	}

	//Material parameter
	{
	}
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
