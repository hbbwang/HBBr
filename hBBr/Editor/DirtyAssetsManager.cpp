#include "DirtyAssetsManager.h"
#include "ContentManager.h"
DirtyAssetsManager::DirtyAssetsManager(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.verticalLayout->setContentsMargins(8,8,8,8);
	setObjectName("DirtyAssetsManager");
	setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
	setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	setWindowFlags(Qt::Window);
	setWindowFlags(Qt::Tool);
	//setWindowFlag(Qt::FramelessWindowHint);
	
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	resize(800, 600);
	setWindowTitle("Save Assets");
	ui.treeWidget->setIndentation(0);
	ui.treeWidget->setColumnCount(3);
	ui.treeWidget->setHeaderLabels({
		"Name",
		"Type",
		"Virtual Path",
		"Repository"});

	auto dirtyAssets = ContentManager::Get()->GetDirtyAssets();

	for (auto& i : dirtyAssets)
	{
		if (!i.expired())
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(ui.treeWidget);

			//Column
			item->setText(0, i.lock()->displayName.c_str());//Name
			item->setText(1, (GetAssetTypeString(i.lock()->type)).c_str());//Type
			item->setText(2, i.lock()->virtualFilePath.c_str());//Virtual path
			item->setText(3, i.lock()->repository.c_str());//Virtual path

			ui.treeWidget->addTopLevelItem(item);
		}
	}

	//	
	connect(ui.SaveButton, &QPushButton::clicked, this, [this, dirtyAssets]()
		{
			for (auto& i : dirtyAssets)
			{
				if (!i.expired())
				{
					ContentManager::Get()->SaveAssetInfo(i.lock().get());
				}
			}
			ContentManager::Get()->ClearDirtyAssets();
			_finishExec();
			close();
		});
	connect(ui.CancelButton, &QPushButton::clicked, this, [this]()
		{
			close();
		});
}

DirtyAssetsManager::~DirtyAssetsManager()
{

}
