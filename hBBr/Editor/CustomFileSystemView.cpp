#include "CustomFileSystemView.h"
#include <qmessagebox.h>
#include <qaction.h>
#include <qmenu.h>
#include <QWheelEvent>
#include <qdebug.h>
#include <QModelIndex>
#include <qmimedata.h>
#include "CustomFileSystemModel.h"
#include <QFile>
#include <QLineEdit>
#include <QWidgetAction>
#include "EditorCommonFunction.h"
#include "Resource/ContentManager.h"
/*------------------------------------------------------List View*/
CustomListView::CustomListView(QWidget *parent)
	: QListView(parent)
{
	this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(CustomContextMenu(const QPoint&)));

	this->setIconSize(_iconSize);
	this->setGridSize(QSize(_iconSize.width() + 10, _iconSize.height()+30));

	_import = new QAction(QString::fromLocal8Bit("资源导入"), this);
	_createNewFolder = new QAction(QString::fromLocal8Bit("创建文件夹"), this);
	_rename = new QAction(QString::fromLocal8Bit("重命名"), this);
	_delete = new QAction(QString::fromLocal8Bit("删除"), this);
	_createMaterialInstance = new QAction(QString::fromLocal8Bit("创建材质实例(.mat)"), this);
	_openCurrentFolder = new QAction(QString::fromLocal8Bit("打开资源管理器"), this);
	//_widgetAction = new QWidgetAction(this);
	//_searchInput = new QLineEdit(this);
	//_widgetAction->setDefaultWidget(_searchInput);

	this->setMouseTracking(true);
	this->setToolTipDuration(0);
	//
	//void pressed(const QModelIndex & index);
	//void clicked(const QModelIndex & index);
	//void doubleClicked(const QModelIndex & index);
	//void activated(const QModelIndex & index);
	connect(this,SIGNAL(clicked(const QModelIndex&)),this,SLOT(SlotClicked(const QModelIndex&)));
	connect(this, SIGNAL(pressed(const QModelIndex&)), this, SLOT(SlotPressed(const QModelIndex&)));
	//
	connect(_delete,SIGNAL(triggered(bool)),this,SLOT(DeleteFile()));
	connect(_rename, SIGNAL(triggered(bool)), this, SLOT(RenameFile()));
}

CustomListView::~CustomListView()
{

}

void CustomListView::mousePressEvent(QMouseEvent* event)
{
	__super::mousePressEvent(event);
}

void CustomListView::mouseReleaseEvent(QMouseEvent* event)
{
	__super::mouseReleaseEvent(event);
}

void CustomListView::mouseDoubleClickEvent(QMouseEvent* event)
{
	__super::mouseDoubleClickEvent(event);
}

void CustomListView::wheelEvent(QWheelEvent* event)
{
	if (bCanResizeIcon)
	{
		if (event->delta() > 0)
		{
			if (_iconSize.width() <= _maxIconSize.width())
			{
				_iconSize = _iconSize + QSize(5, 5);
				this->setIconSize(_iconSize);
				this->setGridSize(QSize(_iconSize.width() + 10, _iconSize.height() + 30));
			}
		}
		else if (event->delta() < 0)
		{
			if (_iconSize.width() >= _minIconSize.width())
			{
				_iconSize = _iconSize - QSize(5, 5);
				this->setIconSize(_iconSize);
				this->setGridSize(QSize(_iconSize.width() + 10, _iconSize.height() + 30));
			}
		}
	}
	__super::wheelEvent(event);
}

void CustomListView::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Alt)
		bCanResizeIcon = true;
	__super::keyPressEvent(event);
}

void CustomListView::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Alt)
		bCanResizeIcon = false;
	__super::keyReleaseEvent(event);
}

void CustomListView::focusInEvent(QFocusEvent* event)
{
	__super::focusInEvent(event);
}

void CustomListView::focusOutEvent(QFocusEvent* event)
{
	bCanResizeIcon = false;
	bShowToolTip = false;
	_toolTipIndex = QModelIndex();
	__super::focusOutEvent(event);
}

void CustomListView::setRootIndex(const QModelIndex& index)
{
	QListView::setRootIndex(index);
	//获取当前目录的所有资产的文件信息
	{
		_fileInfos.clear();
		if (model())
		{
			auto mod = qobject_cast<QFileSystemModel*>(model());
			if (mod)
			{
				QDir dir(mod->filePath(index));
				if (dir.exists())
				{
					for (auto i : dir.entryInfoList())
					{
						HGUID guid;
						StringToGUID(i.baseName().toStdString().c_str(), &guid);
						auto assetInfo = ContentManager::Get()->GetAssetInfo(guid);
						if (assetInfo)
						{
							auto modelIndex = mod->index(i.absoluteFilePath());
							_fileInfos.insert(modelIndex, assetInfo);
						}
					}
				}
			}		
		}
	}
}

void CustomListView::mouseMoveEvent(QMouseEvent* event)
{
	QModelIndex index = this->indexAt(event->pos() );
	//
	if (index.isValid() && _toolTipIndex != index)
	{
		if (bShowToolTip == false)
		{
			_toolTipIndex = index;
			CustomFileSystemModel* dmodel = reinterpret_cast<CustomFileSystemModel*>(this->model());
			if (dmodel)
			{
				auto it = _fileInfos.find(index);
				auto assetInfo = it.value();
				QFileInfo fileInfo(dmodel->filePath(index));
				QString fileName;
				QString byteSize;
				if (fileInfo.isDir())
				{
					fileName = fileInfo.fileName();
					byteSize = "ByteSize: " + fileInfo.size();
				}
				else if (!assetInfo)
				{
					fileName = fileInfo.fileName();
					byteSize = "ByteSize: ???";
				}
				else
				{
					fileName = assetInfo->name.c_str();
					byteSize = "ByteSize: " + QString::number(assetInfo->byteSize);
				}
				fileName = "Name: " + fileName;
				QString suffix = "Type: ";
				suffix += fileInfo.isDir() ? "folder" : fileInfo.suffix();
				QString filePath = "Path: " + fileInfo.absolutePath();
				QToolTip::showText(QCursor::pos(), fileName + "\n" + suffix + "\n" + byteSize + "\n" + filePath, this);
				bShowToolTip = true;
			}
			else
			{
				bShowToolTip = false;
				_toolTipIndex = QModelIndex();
			}
		}
	}
	else if (!index.isValid())
	{
		bShowToolTip = false;
		_toolTipIndex = QModelIndex();
	}
	else
	{
		bShowToolTip = false;
	}

	__super::mouseMoveEvent(event);
}

void CustomListView::dropEvent(QDropEvent* event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	CustomFileSystemModel* dmodel = reinterpret_cast<CustomFileSystemModel*>(this->model());
	QModelIndex index = indexAt(event->pos());
	if (dmodel)
	{
		if (index.isValid() && index != currentIndex())
		{
			QString allPath = "-----------\n";
			for (int i = 0; i < urls.count(); i++)
			{
				allPath += urls[i].toLocalFile() + "/\n";
			}
			allPath += "-----------\n";
			QMessageBox msgBox(QString::fromLocal8Bit("注意"),
				QString::fromLocal8Bit("注意,你正在进行文件夹的移动(请注意不要移动了关键文件),包含以下文件夹:\n") +
				allPath +
				QString::fromLocal8Bit("请确认."),
				QMessageBox::Icon::Question,
				QMessageBox::Yes, QMessageBox::No,
				QMessageBox::NoButton,
				this, Qt::FramelessWindowHint);
			int result = msgBox.exec();
			if (result == QMessageBox::Yes)
			{
				for (int i = 0; i < urls.count(); i++)
				{
					QString filePath = urls[i].toLocalFile();
					QFileInfo info(filePath);
					QString fileName = info.fileName();//with suffix
					//old path
					QString oldPath = QDir::toNativeSeparators(info.path() + QDir::separator());
					//new path 
					QString newPath = QDir::toNativeSeparators(dmodel->filePath(index) + QDir::separator());
					QFile::rename(oldPath + fileName, newPath + fileName);
				}
			}
		}
	}
}

void CustomListView::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
	else {
		event->ignore();
	}
}

void CustomListView::showEvent(QShowEvent* event)
{
	__super::showEvent(event);
	this->activateWindow();
}

void CustomListView::SlotClicked(const QModelIndex& index)
{

}

void CustomListView::SlotPressed(const QModelIndex& index)
{

}

void CustomListView::DeleteFile()
{
	CustomFileSystemModel* dmodel = reinterpret_cast<CustomFileSystemModel*>(this->model());
	if (dmodel)
	{
		QMessageBox msgBox(QString::fromLocal8Bit("注意"),
			QString::fromLocal8Bit("注意,你正在执行一个或者多个文件的删除操作,\n") +
			QString::fromLocal8Bit("请确认."),
			QMessageBox::Icon::Question,
			QMessageBox::Yes, QMessageBox::No,
			QMessageBox::NoButton,
			this, Qt::FramelessWindowHint);
		int result = msgBox.exec();
		QList<QString> assetsGuids;
		if (result == QMessageBox::Yes)
		{
			for (auto i : this->selectedIndexes())
			{
				QString filePath = dmodel->filePath(i);
				DeleteAllFile(filePath, &assetsGuids);
			}
		}
	}
}

void CustomListView::RenameFile()
{
	if (currentIndex().isValid())
		edit(currentIndex());
}

void CustomListView::CustomContextMenu(const QPoint& point)
{
	QMenu* _menu = new QMenu(this);

	//_menu->addAction(_widgetAction);
	//_menu->addSeparator();
	_menu->addAction(_import);
	_menu->addSeparator();
	_menu->addAction(_openCurrentFolder);
	_menu->addAction(_createNewFolder);
	QMenu* _subMenu_CreateAsset = new QMenu(QString::fromLocal8Bit("创建资产"),this);
	_menu->addMenu(_subMenu_CreateAsset);
	{
		_subMenu_CreateAsset->addAction(_createMaterialInstance);
	}
	_menu->addAction(_rename);
	_menu->addAction(_delete);
	_menu->exec(this->mapToGlobal(point));
}

/*------------------------------------------------------Tree View*/

CustomTreeView::CustomTreeView(QWidget* parent) : QTreeView(parent)
{
	this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(CustomContextMenu(const QPoint&)));
	//
	_menu = new QMenu(this);
	_import = new QAction(QString::fromLocal8Bit("资源导入"),this);
	_createNewFolder = new QAction(QString::fromLocal8Bit("创建文件夹"), this);
	_rename = new QAction(QString::fromLocal8Bit("重命名"), this);
	_delete = new QAction(QString::fromLocal8Bit("删除"), this);
	_openCurrentFolder = new QAction(QString::fromLocal8Bit("打开资源管理器"), this);
	//
	connect(_delete, SIGNAL(triggered(bool)), this, SLOT(DeleteFile()));
	connect(_rename, SIGNAL(triggered(bool)), this, SLOT(RenameFile()));
	//
}

CustomTreeView::~CustomTreeView()
{
}

void CustomTreeView::mousePressEvent(QMouseEvent* event)
{
	__super::mousePressEvent(event);
	this->clicked(this->indexAt(event->pos()));
}

void CustomTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
	__super::mouseDoubleClickEvent(event);
}

void CustomTreeView::wheelEvent(QWheelEvent* event)
{
	if (bCanResizeIcon)
	{
		if (event->delta() > 0)
		{
			if (_iconSize.width() <= _maxIconSize.width())
			{
				_iconSize = _iconSize +QSize(5,5);

			}
		}
		else if (event->delta() < 0)
		{
			if (_iconSize .width() >= _minIconSize.width())
			{
				_iconSize = _iconSize - QSize(5, 5);
			}
		}
	}
	__super::wheelEvent(event);
}

void CustomTreeView::CustomContextMenu(const QPoint& point)
{
	if (_menu != NULL && currentIndex().isValid())
	{
		_menu->addAction(_import);
		_menu->addAction(_openCurrentFolder);
		_menu->addAction(_createNewFolder);
		_menu->addAction(_rename);
		_menu->addAction(_delete);
		_menu->exec(this->mapToGlobal(point));
	}
}

void CustomTreeView::dropEvent(QDropEvent* event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	CustomFileSystemModel* dmodel = reinterpret_cast<CustomFileSystemModel*>(this->model());
	QModelIndex index = indexAt(event->pos());
	if (dmodel )
	{
		if (index.isValid() && index != currentIndex())
		{
			QString allPath = "-----------\n";
			for (int i = 0; i < urls.count(); i++)
			{
				allPath += urls[i].toLocalFile() + "/\n";
			}
			allPath += "-----------\n";
			QMessageBox msgBox(QString::fromLocal8Bit("注意"),
				QString::fromLocal8Bit("注意,你正在进行文件夹的移动(请注意不要移动了关键文件),包含以下文件夹:\n") +
				allPath +
				QString::fromLocal8Bit("请确认."),
				QMessageBox::Icon::Question,
				QMessageBox::Yes, QMessageBox::No,
				QMessageBox::NoButton,
				this, Qt::FramelessWindowHint);
			int result = msgBox.exec();
			if (result == QMessageBox::Yes)
			{
				for (int i = 0; i < urls.count(); i++)
				{
					QString filePath = urls[i].toLocalFile();
					QFileInfo info(filePath);
					QString fileName = info.fileName();//with suffix
					//old path
					QString oldPath = QDir::toNativeSeparators(info.path() + QDir::separator());
					//new path 
					QString newPath = QDir::toNativeSeparators(dmodel->filePath(index) + QDir::separator());
					QFile::rename(oldPath + fileName, newPath + fileName);
				}
			}
		}
	}
}

void CustomTreeView::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
	else {
		event->ignore();
	}
}

void CustomTreeView::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Alt)
		bCanResizeIcon = true;
	__super::keyPressEvent(event);
}

void CustomTreeView::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Alt)
		bCanResizeIcon = false;
	__super::keyReleaseEvent(event);
}

void CustomTreeView::focusOutEvent(QFocusEvent* event)
{
	bCanResizeIcon = false;
	__super::focusOutEvent(event);
}

QSize CustomTreeView::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return _iconSize;
}

void CustomTreeView::DeleteFile()
{
	CustomFileSystemModel* dmodel = reinterpret_cast<CustomFileSystemModel*>(this->model());
	if (dmodel)
	{
		QMessageBox msgBox(QString::fromLocal8Bit("注意"),
			QString::fromLocal8Bit("注意,你正在执行一个或者多个文件的删除操作,\n") +
			QString::fromLocal8Bit("请确认."),
			QMessageBox::Icon::Question,
			QMessageBox::Yes, QMessageBox::No,
			QMessageBox::NoButton,
			this, Qt::FramelessWindowHint);
		int result = msgBox.exec();
		if (result == QMessageBox::Yes)
		{
			for (auto i : this->selectedIndexes())
			{
				QString filePath = dmodel->filePath(i);
				DeleteAllFile(filePath);
			}
		}
	}
}

void CustomTreeView::RenameFile()
{
	if(currentIndex().isValid())
		edit(currentIndex());
}