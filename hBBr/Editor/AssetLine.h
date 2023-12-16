#pragma once
#include <QWidget>
#include "ui_AssetLine.h"
#include <qlabel.h>
#include "PropertyClass.h"
#include "HString.h"

class AssetLine : public PropertyClass 
{
	Q_OBJECT

public:
	AssetLine(HString name , QWidget *parent, HString text, HString condition = "");
	~AssetLine();

	QString mCondition;

	QLabel* highLight;

	HString* _stringBind = nullptr;

	class AssetObject* _objectBind = nullptr;

	std::vector<HString>* _stringArrayBind = nullptr;

	int	_stringArrayBindIndex = 0;

	std::function<void(AssetLine* ,const char*)> _bindStringFunc = [](AssetLine*,const char*) {};
	std::function<void(const char*)> _bindFindButtonFunc = [](const char*) {};
	void ShowClearButton(bool bShow);

protected:
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
