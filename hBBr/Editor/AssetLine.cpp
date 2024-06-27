#include "AssetLine.h"
//��ק�¼�
#include <QDragEnterEvent>
//�����¼�
#include <QDropEvent>
#include <qevent.h>
#include <qmimedata.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include "EditorCommonFunction.h"
#include "FileSystem.h"
#include "AssetObject.h"
#include "ContentBrowser.h"
AssetLine::AssetLine(HString name, QWidget *parent, HString text, HString condition)
	: PropertyClass(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name.c_str());
	Init(parent, text, condition);
}

AssetLine::AssetLine(QWidget* parent, HString text, HString condition)
{
	ui.setupUi(this);
	Init(parent, text, condition);
	ui.horizontalLayout->removeItem(ui.horizontalSpacer);
	ui.Name->setHidden(true);
	ui.horizontalLayout->setStretch(0, 0);
	ui.horizontalLayout->setStretch(1, 0);
	ui.horizontalLayout->setStretch(2, 0);
	ui.horizontalLayout->setStretch(3, 0);
	ui.horizontalLayout->setStretch(4, 10);
	ui.horizontalLayout->setStretch(5, 0);
}

void AssetLine::Init(QWidget* parent, HString text, HString condition)
{
	mCondition = condition.c_str();
	//
	ui.LineEdit->setText(text.c_str());
	ui.LineEdit->setReadOnly(true);
	ui.LineEdit->setDragEnabled(true);
	ui.LineEdit->setAcceptDrops(true);
	this->setAcceptDrops(true);
	this->setMouseTracking(true);
	this->setObjectName("PropertyAssetLine");
	//
	ui.Name->setObjectName("PropertyName");
	ui.LineEdit->setObjectName("PropertyAssetLine_LineEdit");
	ui.FindButton->setObjectName("PropertyAssetLine_Button");
	ui.pushButton->setObjectName("PropertyAssetLine_Button");
	ui.FindButton->setMaximumSize(20, 20);
	ui.pushButton->setMaximumSize(20, 20);
	//
	highLight = new QLabel(this);
	highLight->setStyleSheet("background-color:transparent;border:3px dashed rgb(10,255,55);border-radius:4px;");
	highLight->hide();

	//
	connect(ui.FindButton, &QPushButton::clicked, this, [this]() {
		_bindFindButtonFunc(this->ui.LineEdit->text().toStdString().c_str());
		});
	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(clear()));
	connect(ui.LineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineChanged(QString)));
	UpdateSetting();
}

AssetLine::~AssetLine()
{
}

void AssetLine::dragEnterEvent(QDragEnterEvent* event)
{
	bool bOK = false;
	auto condition = mCondition.split(";");
	if (event->mimeData()->hasUrls())
	{
		QFileInfo fileinfo(event->mimeData()->urls().at(0).toLocalFile());
		for (auto cc : condition)
		{
			bOK = bOK || fileinfo.suffix().compare(cc, Qt::CaseInsensitive) == 0;
		}
	}
	else if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
		auto cb = ContentBrowser::GetContentBrowsers();
		auto source = dynamic_cast<VirtualFileListView*>(event->source());
		if (!encoded.isEmpty() && source && cb.size() > 0)
		{
			QDataStream stream(&encoded, QIODevice::ReadOnly);
			while (!stream.atEnd())
			{
				int row, col;
				QMap<int, QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if (roleDataMap.contains(Qt::DisplayRole))
				{
					//收集拖拽的AssetInfos
					if (event->source()->objectName().compare(source->objectName()) == 0)//List View
					{
						CustomListItem* Item = (CustomListItem*)source->item(row);
						for (auto cc : condition)
						{
							HString suffix = Item->_fullPath.toStdString().c_str();
							suffix = suffix.GetSuffix();
							bOK = bOK || suffix.IsSame(cc.toStdString().c_str() , false);
						}
						event->acceptProposedAction();
					}
				}
			}
		}
	}
	else
	{
		event->ignore();
	}

	if (bOK)
	{
		highLight->setStyleSheet("background-color:rgba(10,255,55,0.25);border:3px dashed rgb(10,255,55);border-radius:4px;");
		highLight->show();
		event->acceptProposedAction();
	}
	else
	{
		highLight->setStyleSheet("background-color:rgba(255,75,25,0.25);border:3px dashed rgb(255,55,10);border-radius:4px;");
		highLight->show();
	}

}

void AssetLine::dragLeaveEvent(QDragLeaveEvent* event)
{
	highLight->hide();
}

void AssetLine::dropEvent(QDropEvent* event)
{
	QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
	auto cb = ContentBrowser::GetContentBrowsers();
	auto source = dynamic_cast<VirtualFileListView*>(event->source());
	if (!encoded.isEmpty() && source && cb.size() > 0)
	{
		std::vector<AssetInfoBase*> assets;
		QDataStream stream(&encoded, QIODevice::ReadOnly);
		while (!stream.atEnd())
		{
			if (assets.capacity() < assets.size())
			{
				assets.reserve(assets.capacity() + 25);
			}
			int row, col;
			QMap<int, QVariant> roleDataMap;
			stream >> row >> col >> roleDataMap;
			if (roleDataMap.contains(Qt::DisplayRole))
			{
				//收集拖拽的AssetInfos
				if (event->source()->objectName().compare(source->objectName()) == 0)//List View
				{
					CustomListItem* Item = (CustomListItem*)source->item(row);
					if (Item && !Item->_assetInfo.expired())
					{
						_bindAssetPath(Item->_assetInfo.lock()->virtualFilePath.c_str());
					}
				}
			}
		}
	}
	else
	{
		highLight->hide();
		QFileInfo fileinfo(event->mimeData()->urls().at(0).toLocalFile());
		auto condition = mCondition.split(";");
		bool bOK = false;
		for (auto cc : condition)
		{
			bOK = bOK || fileinfo.suffix().compare(cc, Qt::CaseInsensitive) == 0;
		}
		if (bOK)
		{
			//QMessageBox::information(0,0,0,0);
			const QMimeData* mimeData = event->mimeData();
			if (!mimeData->hasUrls())
			{
				return;
			}
			QList<QUrl> urlList = mimeData->urls();
			//���ͬʱ�����˶����Դ��ֻѡ��һ��
			QString fileName = urlList.at(0).toLocalFile();
			if (fileName.isEmpty())
			{
				return;
			}
			HString path = fileName.toStdString().c_str();
			path = FileSystem::GetRelativePath(path.c_str());
			ui.LineEdit->setText(path.c_str());
			UpdateSetting();
		}
	}

	highLight->hide();
}

void AssetLine::lineChanged(QString newStr)
{
	//QMessageBox::information(0,0,0,0);
	if (_stringBind)
	{
		_stringBind->assign(newStr.toStdString().c_str());
		ui.LineEdit->setText(_stringBind->c_str());
	}
	_bindStringFunc(this, newStr.toStdString().c_str());
	if (_objectBind )
	{
		ui.LineEdit->setText((_objectBind->_assetInfo.lock()->assetFilePath + _objectBind->_assetInfo.lock()->displayName).c_str());
	}
	if (_stringArrayBind)
		_stringArrayBind->at(_stringArrayBindIndex) = newStr.toStdString().c_str();
}

void AssetLine::clear()
{
	ui.LineEdit->setText("");
	UpdateSetting();
}

void AssetLine::ShowClearButton(bool bShow)
{
	ui.pushButton->setVisible(bShow);
}

void AssetLine::UpdateSetting()
{
	////����Ԥ��ͼ
	//QFileInfo info(ui.LineEdit->text());
	//if (info.isFile())
	//{		
	//	//��ȡԤ��ͼ��·��
	//	QString path = info.absoluteFilePath();
	//	path.remove(info.suffix());

	//	bool  bExists = false;
	//	bool bIsTga = false;
	//	QFileInfo pp(path + "jpg");
	//	bExists |= pp.exists();
	//	if (!bExists)
	//	{
	//		pp.setFile(path + "png");
	//		bExists |= pp.exists();
	//	}
	//	if (!bExists)
	//	{
	//		pp.setFile(path + "tga");
	//		bExists |= pp.exists();
	//		if (bExists)
	//			bIsTga = true;
	//	}

	//	if (bExists)
	//	{
	//		if (bIsTga)
	//		{			
	//			QPixmap map(QPixmap::fromImage(GetImageFromTGA(pp.filePath())));
	//			map = map.scaled(60, 60, Qt::KeepAspectRatioByExpanding);
	//			ui.picture->setPixmap(map);
	//		}
	//		else
	//		{
	//			QPixmap map(pp.filePath());
	//			map = map.scaled(60, 60, Qt::KeepAspectRatioByExpanding);
	//			ui.picture->setPixmap(map);
	//		}
	//	}
	//}
	//else
	//{
	//	QPixmap null_Map(0,0);
	//	ui.picture->setPixmap(null_Map);
	//}
}

void AssetLine::resizeEvent(QResizeEvent* event)
{
	highLight->resize(width(),height());
}
