#include "ContentBrowser.h"
#include <QStyleOption>
#include <qpainter.h>
#include <qsplitter.h>
#include <qtreeview.h>
#include <qlistview.h>
#include <QFileSystemModel>
#include <qheaderview.h>
#include <QMouseEvent>
#include <qdebug.h>
#include <QMessageBox>
#include <QAction>
#include "CustomSearchLine.h"
#include "EditorCommonFunction.h"
#include "HString.h"
#include <QApplication>
#include <QFileDialog>
#include "QDesktopServices.h"
#include "qabstractitemview.h"
#include "FileSystem.h"
#include "RendererCore/Core/VulkanRenderer.h"
#include "RendererCore/Form/FormMain.h"
#include "Asset/Material.h"
#include "Asset/ContentManager.h"
//
//QWidget* ContentBrowser::_currentFocusContentBrowser = nullptr;
//QList<QWidget*> ContentBrowser::_allContentBrowser;
//
//ContentBrowser::ContentBrowser(QWidget *parent)
//	: QWidget(parent)
//{
//	ui.setupUi(this);
//
//	this->setObjectName("ContentBrowser");
//	_mainWindow = (MainWindow*)parent;
//
//	_splitterBox = new QSplitter(Qt::Horizontal, this);
//	_splitterBox->setObjectName("ContentBrowserSplitter");
//
//	_tree_group = new QWidget(_splitterBox);
//	_tree_group->setObjectName("ContentBrowserTreeGroup");
//	QVBoxLayout* _tree_group_layout = new QVBoxLayout(_tree_group);
//	_tree_group->setLayout(_tree_group_layout);
//	_tree_group_layout->setMargin(0);
//	_tree_group_layout->setSpacing(0);
//
//	_list_group = new QWidget(_splitterBox);
//	_list_group->setObjectName("ContentBrowserListGroup");
//	QVBoxLayout* _list_group_layout = new QVBoxLayout(_list_group);
//	_list_group->setLayout(_list_group_layout);
//	_list_group_layout->setMargin(0);
//	_list_group_layout->setSpacing(0);
//
//	_treeSearchLine = new CustomSearchLine(this);
//	_treeSearchLine->ui.comboBox->setHidden(true);
//	_listSearchLine = new CustomSearchLine(this);
//	_tree_group_layout->addWidget(_treeSearchLine);
//	_list_group_layout->addWidget(_listSearchLine);
//
//	_treeWidget = new CustomTreeView(this);
//	_tree_group_layout->addWidget(_treeWidget);
//	_treeWidget->setObjectName("ContentBrowserTree");
//
//	_listWidget = new CustomListView(this);
//	_list_group_layout->addWidget(_listWidget);
//	_listWidget->setObjectName("ContentBrowserList");
//
//	//initialize
//	_treeWidget->setHeaderHidden(true);
//	
//	//Tree
//	_treeFileSystemModel = new CustomFileSystemModel(_treeWidget);
//	_treeFileSystemModel->_contentBrowserTreeView = _treeWidget;
//	_treeWidget->setModel(_treeFileSystemModel);
//	_treeFileSystemModel->setRootPath((FileSystem::GetProgramPath() + "Asset").c_str()  );
//	_treeWidget->setRootIndex(_treeFileSystemModel->index(_treeFileSystemModel->rootPath()));
//	_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//	for (int i = 1; i < _treeFileSystemModel->columnCount(); i++)
//	{
//		_treeWidget->header()->hideSection(i);
//	}
//	_treeFileSystemModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
//	_treeFileIconProvider._scope = IconProviderScope::ContentBrowserTreeView;
//	_treeFileSystemModel->setIconProvider(&_treeFileIconProvider);
//
//	_treeWidget->setDragDropMode(QListView::DragDrop);
//	_treeWidget->setDragEnabled(true);
//	_treeWidget->setAcceptDrops(true);
//	_treeWidget->setSelectionMode(QListView::ExtendedSelection);
//	_treeFileSystemModel->setReadOnly(false);
//	_treeWidget->setEditTriggers(QTreeView::NoEditTriggers);
//
//	//List
//	_listFileSystemModel = new CustomFileSystemModel(_listWidget);
//	_listFileSystemModel->_contentBrowserListView = _listWidget;
//	list_filters = QDir::Files | QDir::Dirs |QDir::AllDirs | QDir::NoDotAndDotDot;
//	_listFileSystemModel->setFilter(list_filters);
//	QStringList nameFilter;
//	nameFilter << "*.qss" << "*.cache" << "*.dds" << "*.tex" << "*.texCube" << "*.mat" << "*.fx" << "*.hlsl" << "*.ini" << "*.fbx" << "*.wrold" << "*.level" << "*.prefab";
//	list_filterCache = nameFilter;
//	_listFileSystemModel->setNameFilterDisables(false);
//	_listFileSystemModel->setNameFilters(list_filterCache);
//	_listWidget->setModel(_listFileSystemModel);
//	_listFileSystemModel->setRootPath((FileSystem::GetProgramPath() + "Asset").c_str());
//	_listWidget->setRootIndex(_listFileSystemModel->index(_listFileSystemModel->rootPath()));
//	_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//	_listWidget->setViewMode(QListView::IconMode);
//	_listWidget->setMovement(QListView::Snap);
//	_listWidget->setResizeMode(QListView::Adjust);
//
//	_listWidget->setSelectionMode(QListView::ExtendedSelection);
//	_listWidget->setDragDropMode(QListView::DragDrop);
//	_listWidget->setDragEnabled(true);
//	_listWidget->setAcceptDrops(true);
//	_listFileSystemModel->setReadOnly(false);
//	_listWidget->setEditTriggers(QTreeView::NoEditTriggers);
//
//	_listFileIconProvider._scope = IconProviderScope::ContentBrowserListView;
//	_listFileSystemModel->setIconProvider(&_listFileIconProvider);
//	//
//
//	_splitterBox->setStretchFactor(1, 4);
//	ui.ContentBrowserVBoxLayout->addWidget(_splitterBox);
//	ui.ContentBrowserVBoxLayout->setStretch(0,0);
//	ui.ContentBrowserVBoxLayout->setStretch(1, 1000);
//
//	//List Combo
//	_listSearchLine->ui.comboBox->addItems({"All","Model","Material","Level","Xml"});
//	//Path label
//	ui.PathLabel->setObjectName("PathLabel");
//
//	//this->setMouseTracking(true);
//	//this->setToolTipDuration(1000);
//	connect(_treeWidget,SIGNAL(clicked(const QModelIndex &)),this,SLOT(TreeClicked(const QModelIndex &)));
//	connect(_listWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(ListClicked(const QModelIndex&)));
//	connect(_listWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(ListDoubleClicked(const QModelIndex&)));
//	connect(_listWidget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous){
//		_listWidget->selectionModel()->select(current, QItemSelectionModel::SelectCurrent);
//	});
//	//
//	connect(_listWidget->_createNewFolder, SIGNAL(triggered(bool)), this, SLOT(CreateFolder()));
//	connect(_treeWidget->_createNewFolder, SIGNAL(triggered(bool)), this, SLOT(CreateFolder()));
//	connect(_treeSearchLine->ui.lineEdit, SIGNAL(returnPressed()),this,SLOT(TreeSearch()));
//	connect(_listSearchLine->ui.lineEdit, SIGNAL(returnPressed()), this, SLOT(ListSearch()));
//	connect(_listSearchLine->ui.comboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(ListFilter(const QString&)));
//	connect(ui.BackspaceButton,SIGNAL(clicked(bool)),this,SLOT(Backspace()));
//	connect(ui.ImportButton , SIGNAL(clicked(bool)), this, SLOT(AssetImport()));
//	connect(_treeWidget->_import, SIGNAL(triggered(bool)), this, SLOT(AssetImport()));
//	connect(_listWidget->_import, SIGNAL(triggered(bool)), this, SLOT(AssetImport()));
//	connect(_listWidget->_createMaterialInstance, &QAction::triggered, this, [this]() {
//		Material::CreateMaterial(_treeFileSystemModel->filePath(_treeWidget->currentIndex()).toStdString().c_str());
//	});
//	connect(_listWidget->_openCurrentFolder, SIGNAL(triggered(bool)), this, SLOT(OpenCurrentFolder_List()));
//	connect(_treeWidget->_openCurrentFolder, SIGNAL(triggered(bool)), this, SLOT(OpenCurrentFolder_List()));
//}
//
//ContentBrowser::~ContentBrowser()
//{
//}
//
//void ContentBrowser::focusInEvent(QFocusEvent* event)
//{
//	ContentBrowser::_currentFocusContentBrowser = this;
//}
//
//void ContentBrowser::showEvent(QShowEvent* event)
//{
//	_currentFocusContentBrowser = this;
//	auto it = qFind(ContentBrowser::_allContentBrowser.begin(), ContentBrowser::_allContentBrowser.end(), this);
//	if (it == ContentBrowser::_allContentBrowser.end())
//	{
//		ContentBrowser::_allContentBrowser.append(this);
//	}
//}
//
//void ContentBrowser::paintEvent(QPaintEvent* event)
//{
//	Q_UNUSED(event);
//	QStyleOption styleOpt;
//	styleOpt.init(this);
//	QPainter painter(this);
//	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
//
//	//Update path label
//	if (_treeWidget->currentIndex().isValid())
//	{
//		QString currentTreePath = _treeFileSystemModel->filePath(_treeWidget->currentIndex());
//		currentTreePath = QDir::toNativeSeparators(currentTreePath);
//
//		QString treeRootPath = _treeFileSystemModel->rootPath();
//		treeRootPath = QDir::toNativeSeparators(treeRootPath);
//
//		currentTreePath.remove(treeRootPath);
//		currentTreePath.replace(QDir::separator(),"  -  ");
//		ui.PathLabel->setText(currentTreePath);
//	}
//	else
//	{
//		ui.PathLabel->setText("  -  ");
//	}
//}
//
//void ContentBrowser::mouseMoveEvent(QMouseEvent* event)
//{
//	__super::mouseMoveEvent(event);
//}
//
//void ContentBrowser::mousePressEvent(QMouseEvent* event)
//{
//	__super::mousePressEvent(event);
//}
//
//void ContentBrowser::closeEvent(QCloseEvent* event)
//{
//	if (_currentFocusContentBrowser == this)
//	{
//		_allContentBrowser.removeOne(_currentFocusContentBrowser);
//		_currentFocusContentBrowser = nullptr;
//	}
//}
//
//void ContentBrowser::TreeClicked(const QModelIndex& index)
//{
//	if (_listWidget && _treeWidget)
//	{
//		if (_treeWidget->currentIndex().isValid())
//		{
//			QString currentTreePath = _treeFileSystemModel->filePath(_treeWidget->currentIndex());
//			_listFileSystemModel->setRootPath(currentTreePath);
//			_listWidget->setRootIndex(_listFileSystemModel->index(_listFileSystemModel->rootPath()));
//		}
//		else
//		{
//			_listFileSystemModel->setRootPath((FileSystem::GetProgramPath() + "Asset").c_str());
//			_listWidget->setRootIndex(_listFileSystemModel->index(_listFileSystemModel->rootPath()));
//		}
//	}
//}
//
//void ContentBrowser::ListClicked(const QModelIndex& index)
//{
//	if (_listWidget && _treeWidget)
//	{
//
//	}
//}
//
//void ContentBrowser::ListDoubleClicked(const QModelIndex& index)
//{
//	if (_listWidget && _treeWidget)
//	{
//		QString newPath = _listFileSystemModel->filePath(index);
//		QFileInfo fileInfo(newPath);
//		if (fileInfo.isDir())
//		{
//			_listFileSystemModel->setRootPath(newPath);
//			_listWidget->setRootIndex(_listFileSystemModel->index(newPath));
//			//
//			QModelIndex treeIndex = _treeFileSystemModel->index(newPath);
//			_treeWidget->setCurrentIndex(treeIndex);
//			_treeWidget->expand(treeIndex);
//		}
//		else
//		{
//			//Open material editor
//			if (fileInfo.suffix().compare("mat",Qt::CaseInsensitive) == 0 )
//			{
//				auto form = VulkanApp::CreateNewWindow(512,512,"Material",false);
//				VulkanApp::CreateRenderer(form);
//			}
//		}
//	}
//}
//
//void ContentBrowser::CreateFolder()
//{
//	if (_treeFileSystemModel && _treeWidget->currentIndex().isValid())
//	{
//		QString currentPath = _treeFileSystemModel->filePath(_treeWidget->currentIndex());
//		QDir dir(currentPath);
//		for (int i = 0 ; i<99 ; i++ )
//		{
//			QString newName = currentPath + "/NewFolder_" + QString::number(i);
//			QFile fff(newName);
//			if (!fff.exists())
//			{
//				dir.mkdir(newName);
//				return;
//			}
//		}
//	}
//}
//
//void ContentBrowser::TreeSearch()
//{
//	//_treeWidget->collapseAll();
//	_treeWidget->selectionModel()->clearSelection();
//	if (_treeSearchLine->ui.lineEdit->text().isEmpty())
//	{
//		return;
//	}
//	QString input = _treeSearchLine->ui.lineEdit->text();
//	QList<SFileSearch> searchList;
//	SearchDir(_treeFileSystemModel->rootPath(), input, searchList);
//
//	for (auto i : searchList)
//	{
//		QModelIndex index = _treeFileSystemModel->index(i.path);
//		if (index.isValid())
//		{
//			_treeWidget->setExpanded(index,true);
//			if(i.bLastFile)
//				_treeWidget->selectionModel()->select(index , QItemSelectionModel::Select);
//		}
//	}
//}
//
//void ContentBrowser::ListSearch()
//{
//	list_nameFilter_line.clear();
//	if (_listSearchLine->ui.lineEdit->text().isEmpty())
//	{
//		if (list_nameFilter_combo.size() <= 0)
//		{
//			_listFileSystemModel->setFilter(QDir::Files | QDir::Dirs | QDir::AllDirs | QDir::NoDotAndDotDot);
//			_listFileSystemModel->setNameFilters(list_filterCache);
//		}
//		else
//		{
//			_listFileSystemModel->setFilter(QDir::Files | QDir::CaseSensitive);
//			_listFileSystemModel->setNameFilters(list_nameFilter_combo);
//		}
//		return;
//	}
//	_listWidget->selectionModel()->clearSelection();
//
//	QString input = _listSearchLine->ui.lineEdit->text();
//	QList<SFileSearch> searchList;
//	QString rootPath = _listFileSystemModel->rootPath();
//	SearchFile(rootPath, input, searchList);
//
//	QStringList lineList;
//	for (auto i : searchList)
//	{
//		list_nameFilter_line << i.fileInfo.baseName();
//		if (list_nameFilter_combo.size() > 0)
//			for (auto c : list_nameFilter_combo)
//			{
//				lineList << i.fileInfo.baseName() + c;
//			}
//		else
//			lineList << i.fileInfo.baseName() +  "*" ;
//	}
//	_listFileSystemModel->setFilter(QDir::Files | QDir::CaseSensitive);
//	_listFileSystemModel->setNameFilters(lineList);
//}
//
//void ContentBrowser::SearchAssetFile(HString filePath, ContentBrowser* cb)
//{
//	HString fileName = filePath.GetFileName();
//	HString path = filePath.GetFilePath();
//	//����ͼ�л�
//	ContentBrowser* contentBrowser = cb;
//	if (!contentBrowser)
//		contentBrowser = dynamic_cast<ContentBrowser*>(_currentFocusContentBrowser);
//	if (!contentBrowser)
//		return;
//	if (contentBrowser->_treeWidget && contentBrowser->_listWidget)
//	{
//		if(contentBrowser->_treeWidget->currentIndex().isValid())
//			contentBrowser->_treeWidget->collapse(contentBrowser->_treeWidget->currentIndex());
//		QStringList pathList;
//		HString newFilePath = filePath;
//		HString firstPath;
//		for (int i = 0; i < 24; i++)
//		{
//			newFilePath = newFilePath.GetFilePath();
//			if (newFilePath.Length() < 7)
//				break;
//			newFilePath.Remove(newFilePath.Length()-1, newFilePath.Length());
//			if (i == 0)
//				firstPath = newFilePath;
//			if (pathList.contains(newFilePath.c_str()))
//				break;
//			pathList.append(newFilePath.c_str());
//		}
//		for (auto i  = pathList.size() -1;i >= 0; i--)
//		{
//			QModelIndex treeIndex = contentBrowser->_treeFileSystemModel->index(pathList[i]);
//			if (treeIndex.isValid())
//			{
//				QString fpp = firstPath.c_str();
//				contentBrowser->_treeWidget->expanded(treeIndex);
//				if (pathList[i].compare(fpp) == 0)
//				{
//					//tree
//					contentBrowser->_treeWidget->selectionModel()->clearSelection();
//					contentBrowser->_treeWidget->scrollTo(treeIndex);
//					contentBrowser->_treeWidget->setCurrentIndex(treeIndex);
//					contentBrowser->_treeWidget->selectionModel()->select(treeIndex, QItemSelectionModel::SelectCurrent);
//					contentBrowser->TreeClicked(treeIndex);
//					//list
//					contentBrowser->_listWidget->selectionModel()->clearSelection();
//					auto listIndex = contentBrowser->_listFileSystemModel->index(filePath.c_str());
//					if (listIndex.isValid())
//					{
//						contentBrowser->_listWidget->scrollTo(listIndex, QAbstractItemView::EnsureVisible);
//						contentBrowser->_listWidget->setCurrentIndex(listIndex);
//						contentBrowser->_listWidget->selectionModel()->select(listIndex, QItemSelectionModel::ClearAndSelect);
//					}
//				}
//			}
//		}
//	}
//}
//
//void ContentBrowser::ListFilter(const QString& newText)
//{
//	list_nameFilter_combo.clear();
//	if (newText.compare("all",Qt::CaseInsensitive) == 0 || newText.isEmpty())
//	{
//		if (list_nameFilter_line.size() <= 0)
//		{
//			_listFileSystemModel->setFilter(QDir::Files | QDir::Dirs | QDir::AllDirs | QDir::NoDotAndDotDot);
//			_listFileSystemModel->setNameFilters(list_filterCache);
//		}
//		else
//		{
//			QStringList lineList;
//			for (auto l : list_nameFilter_line)
//			{
//				lineList << l + "*";
//			}
//			_listFileSystemModel->setFilter(QDir::Files | QDir::CaseSensitive);
//			_listFileSystemModel->setNameFilters(lineList);
//		}
//		return;
//	}
//
//	_listWidget->selectionModel()->clearSelection();
//	_listFileSystemModel->setFilter(QDir::Files | QDir::CaseSensitive);
//
//	if (newText.compare("model", Qt::CaseInsensitive) == 0)
//	{
//		list_nameFilter_combo << "*.model" << "*.fbx";
//	}
//	else if (newText.compare("material", Qt::CaseInsensitive) == 0)
//	{
//		list_nameFilter_combo << "*.mat";
//	}
//	else if (newText.compare("level", Qt::CaseInsensitive) == 0)
//	{
//		list_nameFilter_combo << "*.level";
//	}
//	else if (newText.compare("xml", Qt::CaseInsensitive) == 0)
//	{
//		list_nameFilter_combo << "*.xml";
//	}
//
//	QStringList comboList;
//
//	for (auto c : list_nameFilter_combo)
//	{
//		if (list_nameFilter_line.size() > 0)
//		{
//			for (auto l : list_nameFilter_line)
//			{
//				comboList << l + c;
//			}
//		}
//		else
//		{
//			comboList << c;
//		}
//	}
//	_listFileSystemModel->setNameFilters(comboList);
//}
//
//void ContentBrowser::Backspace()
//{
//	if (_listWidget && _treeWidget)
//	{
//		if (_treeWidget->currentIndex().isValid() && _treeWidget->currentIndex() != _treeWidget->rootIndex())
//		{
//			QString currentTreePath = _treeFileSystemModel->filePath(_treeWidget->currentIndex());
//			currentTreePath = QDir::toNativeSeparators(currentTreePath);
//			int index = currentTreePath.lastIndexOf(QDir::separator());
//			currentTreePath = currentTreePath.left(index);
//			
//			_treeWidget->setCurrentIndex(_treeFileSystemModel->index(currentTreePath));
//
//			_listFileSystemModel->setRootPath(currentTreePath);
//			_listWidget->setRootIndex(_listFileSystemModel->index(_listFileSystemModel->rootPath()));
//		}
//	}
//}
//
//void ContentBrowser::AssetImport()
//{
//	QStringList file_names = QFileDialog::getOpenFileNames(this, "Import Assets", _listFileSystemModel->rootPath(),
//		" All (*.fbx *.png *.tga *.jpg *.jpeg *.hdr *.exr );;\
//		Model (*.fbx);;\
//		Images (*.png *.tga *.jpg *.jpeg *.hdr *.exr);;"
//	);
//		//参数1 父控件
//		//参数2 标题
//		//参数3  默认路径
//		//参数4 过滤文件格式
//		//返回值  文件全路径---"D:/ss/注意事项.txt"
//	//qDebug() << file_names;
//
//	for (auto i : file_names)
//	{
//		QFileInfo fileInfo(i);
//		QString typeText;
//
//		//删除预览图，如果存在，重新加载资源的时候重新生成预览图
//		QString previewImagePath = QString(fileInfo.path() + QDir::separator() + fileInfo.baseName() + "." +fileInfo.suffix() + "@Preview.jpg");
//		QFileInfo previewImage(previewImagePath);
//		if (previewImage.exists())
//			QFile::remove(previewImagePath);
//		//
//		QString dirPath = _treeFileSystemModel->filePath(_treeWidget->currentIndex());
//		QString dstPath = _treeFileSystemModel->filePath(_treeWidget->currentIndex()) + QDir::separator() + fileInfo.baseName();
//		std::weak_ptr<AssetInfoBase> assetInfo;
//		if (_treeWidget->currentIndex().isValid())
//		{
//			if (fileInfo.suffix().compare("fbx", Qt::CaseInsensitive) == 0)
//			{
//				//导入Asset info
//				dstPath += ".fbx";
//				assetInfo = ContentManager::Get()->CreateAssetInfo(dstPath.toStdString().c_str());
//				//复制fbx进content
//				HString guidStr = GUIDToString(assetInfo.lock()->guid);
//				dirPath += QDir::separator();
//				dirPath = dirPath + guidStr.c_str() + ".fbx";
//				FileSystem::FileCopy(i.toStdString().c_str(), dirPath.toStdString().c_str());
//			}
//			else if (fileInfo.suffix().compare("tga", Qt::CaseInsensitive) == 0 ||
//				fileInfo.suffix().compare("png", Qt::CaseInsensitive) == 0 ||
//				fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
//				fileInfo.suffix().compare("jpeg", Qt::CaseInsensitive) == 0
//				)
//			{
//		        //导入dds压缩纹理
//				dstPath += ".dds";
//				assetInfo = ContentManager::Get()->CreateAssetInfo(dstPath.toStdString().c_str());
//				//转换图像格式为dds
//				HString guidStr = GUIDToString(assetInfo.lock()->guid);
//				dirPath += QDir::separator();
//				dirPath = dirPath + guidStr.c_str() + ".dds";
//				Texture2D::CompressionImage2D(i.toStdString().c_str(), dirPath.toStdString().c_str(), true, nvtt::Format_BC3, false, true);
//			}
//			else if (fileInfo.suffix().compare("hdr", Qt::CaseInsensitive) == 0 ||
//				fileInfo.suffix().compare("exr", Qt::CaseInsensitive) == 0
//				)
//			{
//
//			}
//			if (!assetInfo.expired())
//				_listWidget->_fileInfos.insert(_listFileSystemModel->index(dirPath), assetInfo);
//		}
//	}
//}
//
//void ContentBrowser::OpenCurrentFolder_List()
//{
//	QDesktopServices::openUrl(QUrl(_listFileSystemModel->rootPath()));
//}
//
//void ContentBrowser::OpenCurrentFolder_Tree()
//{
//	QDesktopServices::openUrl(QUrl(_treeFileSystemModel->filePath(_treeWidget->currentIndex())));
//}


//重做...
ContentBrowser::ContentBrowser(QWidget* parent )
	:QWidget(parent)
{
	ui.setupUi(this);

	this->setObjectName("ContentBrowser");
	
	_splitterBox = new QSplitter(Qt::Horizontal, this);
	_splitterBox->setObjectName("ContentBrowserSplitter");
	//
	_treeWidget = new QWidget(_splitterBox);
	_listWidget = new QWidget(_splitterBox);
	_splitterBox->addWidget(_treeWidget);
	_splitterBox->addWidget(_listWidget);

	//
	_splitterBox->setStretchFactor(1, 4);
	ui.ContentBrowserVBoxLayout->addWidget(_splitterBox);
	ui.ContentBrowserVBoxLayout->setStretch(0,0);
	ui.ContentBrowserVBoxLayout->setStretch(1, 1000);
	
	//Path label
	ui.PathLabel->setObjectName("PathLabel");
}

ContentBrowser::~ContentBrowser()
{

}

void ContentBrowser::focusInEvent(QFocusEvent* event)
{
}

void ContentBrowser::showEvent(QShowEvent* event)
{
}

void ContentBrowser::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);	
}

void ContentBrowser::mouseMoveEvent(QMouseEvent* event)
{
}

void ContentBrowser::mousePressEvent(QMouseEvent* event)
{
}

void ContentBrowser::closeEvent(QCloseEvent* event)
{
}
