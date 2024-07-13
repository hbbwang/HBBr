#pragma once
#include <QWidget>
#include "ui_AssetLine.h"
#include <qlabel.h>
#include "PropertyClass.h"
#include "HString.h"
#include "ContentManager.h"

class AssetLine : public PropertyClass 
{
	Q_OBJECT

public:
	AssetLine(HString name , QWidget *parent, HString text, HString condition = "");
	AssetLine(QWidget* parent, HString text, HString condition = "");
	AssetLine(QWidget* parent, HString text, AssetType assetType);
	~AssetLine();

	QString mCondition;
	AssetType _assetType;

	QLabel* highLight;

	HString* _stringBind = nullptr;

	std::vector<HString>* _stringArrayBind = nullptr;

	int	_stringArrayBindIndex = 0;

	std::function<void(AssetLine* ,const char*)> _bindStringFunc = [](AssetLine*,const char*) {};
	std::function<void(const char*)> _bindAssetPath = [](const char*newPath) {};

	std::function<void(const char*)> _bindFindButtonFunc = [](const char*) {};
	void ShowClearButton(bool bShow);

protected:

	bool CheckDragItem(class CustomListItem* item);

	void Init(QWidget* parent, HString text, HString condition = "");

	void UpdateSetting();

	virtual void resizeEvent(QResizeEvent* event);
	//��д����ק�¼�������
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent* event);
	//��д�������¼�������
	virtual void dropEvent(QDropEvent* event);

public:
	Ui::AssetLine ui;
private slots:
	void clear();
	void lineChanged(QString newStr);
};
