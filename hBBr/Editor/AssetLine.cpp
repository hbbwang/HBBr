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
AssetLine::AssetLine(HString name, QWidget *parent, HString text, HString condition)
	: PropertyClass(parent)
{
	ui.setupUi(this);
	mCondition = condition.c_str();
	//
	ui.Name->setText(name.c_str());
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
	ui.picture->setText("");
	//
	highLight = new QLabel(this);
	highLight->setStyleSheet("background-color:transparent;border:3px dashed rgb(10,255,55);border-radius:4px;");
	highLight->hide();

	//
	connect(ui.FindButton, &QPushButton::clicked, this, [this](){
		_bindFindButtonFunc(this->ui.LineEdit->text().toStdString().c_str());
	});
	connect(ui.pushButton,SIGNAL(clicked(bool)),this,SLOT(clear()));
	connect(ui.LineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineChanged(QString)));
	UpdateSetting();
}

AssetLine::~AssetLine()
{
}

void AssetLine::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		event->acceptProposedAction();
		QFileInfo fileinfo(event->mimeData()->urls().at(0).toLocalFile());
		auto condition = mCondition.split(";");
		bool bOK = false;
		for (auto cc : condition)
		{
			bOK = bOK || fileinfo.suffix().compare(cc, Qt::CaseInsensitive) == 0;
		}
		if (bOK)
		{
			highLight->setStyleSheet("background-color:rgba(10,255,55,0.25);border:3px dashed rgb(10,255,55);border-radius:4px;");
		}
		else
		{
			highLight->setStyleSheet("background-color:rgba(255,75,25,0.25);border:3px dashed rgb(255,55,10);border-radius:4px;");
		}
		highLight->show();
	}
	else
	{
		event->ignore();
	}
}

void AssetLine::dragLeaveEvent(QDragLeaveEvent* event)
{
	highLight->hide();
}

void AssetLine::dropEvent(QDropEvent* event)
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

void AssetLine::lineChanged(QString newStr)
{
	//QMessageBox::information(0,0,0,0);
	if(_stringBind)
		_stringBind->assign(newStr.toStdString().c_str());
	_bindStringFunc(this, newStr.toStdString().c_str());
	if (_objectBind && !_objectBind->expired() )
	{
		ui.LineEdit->setText((_objectBind->lock()->_assetInfo->relativePath + _objectBind->lock()->_assetInfo->name).c_str());
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
	//����Ԥ��ͼ
	QFileInfo info(ui.LineEdit->text());
	if (info.isFile())
	{		
		//��ȡԤ��ͼ��·��
		QString path = info.absoluteFilePath();
		path.remove(info.suffix());

		bool  bExists = false;
		bool bIsTga = false;
		QFileInfo pp(path + "jpg");
		bExists |= pp.exists();
		if (!bExists)
		{
			pp.setFile(path + "png");
			bExists |= pp.exists();
		}
		if (!bExists)
		{
			pp.setFile(path + "tga");
			bExists |= pp.exists();
			if (bExists)
				bIsTga = true;
		}

		if (bExists)
		{
			if (bIsTga)
			{			
				QPixmap map(QPixmap::fromImage(GetImageFromTGA(pp.filePath())));
				map = map.scaled(60, 60, Qt::KeepAspectRatioByExpanding);
				ui.picture->setPixmap(map);
			}
			else
			{
				QPixmap map(pp.filePath());
				map = map.scaled(60, 60, Qt::KeepAspectRatioByExpanding);
				ui.picture->setPixmap(map);
			}
		}
	}
	else
	{
		QPixmap null_Map(0,0);
		ui.picture->setPixmap(null_Map);
	}
}

void AssetLine::resizeEvent(QResizeEvent* event)
{
	highLight->resize(width(),height());
}
