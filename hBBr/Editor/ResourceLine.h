#pragma once
#include <QWidget>
#include "ui_ResourceLine.h"
#include <qlabel.h>
#include "PropertyClass.h"
#include "HString.h"
class ResourceLine : public PropertyClass
{
	Q_OBJECT

public:
	ResourceLine(QString name , QWidget *parent, HString text, QString condition = "");
	~ResourceLine();

	QString mCondition;

	QLabel* highLight;

	HString* _stringBind = NULL;

	std::vector<HString>* _stringArrayBind = NULL;

	int	_stringArrayBindIndex = 0;

	std::function<void(const char*)> _bindStringFunc = [](const char*) {};
	std::function<void(const char*)> _bindFindButtonFunc = [](const char*) {};
	void ShowClearButton(bool bShow);

protected:
	void UpdateSetting();

	virtual void resizeEvent(QResizeEvent* event);
	//复写”拖拽事件“函数
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent* event);
	//复写”放下事件“函数
	virtual void dropEvent(QDropEvent* event);

public:
	Ui::ResourceLine ui;
private slots:
	void clear();
	void lineChanged(QString newStr);
};
