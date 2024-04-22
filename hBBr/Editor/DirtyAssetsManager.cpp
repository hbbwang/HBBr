#include "DirtyAssetsManager.h"
#include "ContentManager.h"
DirtyAssetsManager::DirtyAssetsManager(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	auto dirtyAssets = ContentManager::Get()->GetDirtyAssets();


	//	
	connect(ui.SaveButton, &QPushButton::clicked, this, [this]() 
		{

			ContentManager::Get()->ClearDirtyAssets();
		});
	connect(ui.CancelButton, &QPushButton::clicked, this, [this]()
		{

		});
	//
	exec();
}

DirtyAssetsManager::~DirtyAssetsManager()
{

}
