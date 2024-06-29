#include "WorldSelector.h"
#include "World.h"
#include "CustomView.h"
#include "qlayout.h"
#include "FileSystem.h"
#include "Serializable.h"
#include "EditorMain.h"
#include "RenderView.h"
#include "FormMain.h"
#include "VulkanRenderer.h"
WorldSelector::WorldSelector(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window);
	this->setObjectName("CustomListView_WorldSelector");
	setLayout(new QVBoxLayout(this));
	_listView = new CustomListView(this);
	layout()->addWidget(_listView);
	((QVBoxLayout*)layout())->setContentsMargins(0,0,0,0);
	((QVBoxLayout*)layout())->setSpacing(0);

	_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	AddWorldItem();

	connect(_listView, &QListWidget::itemDoubleClicked, this,
		[](QListWidgetItem* it) {
			if (EditorMain::_self->_mainRenderView->_mainRenderer)
			{
				if (EditorMain::_self->_mainRenderView->_mainRenderer->renderer)
				{
					auto renderer = EditorMain::_self->_mainRenderView->_mainRenderer->renderer;
					auto item = (CustomListItem*)it;
					//renderer->ReleaseWorld();
					renderer->LoadWorld(item->_path.toStdString());
				}
			}
		});
}

WorldSelector::~WorldSelector()
{

}

void WorldSelector::AddWorldItem()
{
	auto allWorldDirs = FileSystem::GetAllFolders(FileSystem::GetWorldAbsPath().c_str());
	for (auto& i : allWorldDirs)
	{
		HString savedPath = FileSystem::GetSavedAbsPath();
		HString worldSettingPath = FileSystem::Append(i.absPath, ".WorldSettings");
		nlohmann::json json;
		if (Serializable::LoadJson(worldSettingPath.c_str(), json))
		{
			std::string name = json["WorldName"];
			HString worldPreviewPath = FileSystem::Append(FileSystem::Append(savedPath, "PreviewImage"), "World");
			worldPreviewPath = FileSystem::Append(worldPreviewPath, i.baseName+".jpg");
			ToolTip toolTip;
			toolTip._tooltip.append(QString("Path : ") + i.relativePath.c_str());
			toolTip._tooltip.append(QString("GUID : ") + i.baseName.c_str());

			QFileInfo fInfo(worldPreviewPath.c_str());
			if (!fInfo.exists())
			{
				worldPreviewPath = FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme");
				worldPreviewPath = FileSystem::Append(worldPreviewPath, "Icons");
				worldPreviewPath = FileSystem::Append(worldPreviewPath, "ICON_FILE_SCENE.png");
			}
			auto newItem = _listView->AddItem(name.c_str(), worldPreviewPath.c_str(), toolTip);
			newItem->_path = i.baseName.c_str();
		}
	}
}

int WorldSelector::exec()
{
	return QDialog::exec();
}
