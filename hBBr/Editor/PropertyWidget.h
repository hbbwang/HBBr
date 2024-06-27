#pragma once
#include <qsplitter.h>
#include <memory>
#include <map>
#include <QToolButton>
#include <qlabel.h>
class PropertyWidgetButton : public QWidget
{
	Q_OBJECT
public:
	PropertyWidgetButton(QLabel* image, QToolButton* button, QWidget* parent = nullptr);
	~PropertyWidgetButton();
	QToolButton* _button;
	QLabel* _image;
	bool bVisiable = true;
};

struct SItem {
	QWidget* name = nullptr;
	QWidget* value = nullptr;
	QWidget* group_name = nullptr;
	QWidget* group_value = nullptr;
};

struct SGroup {
	QString groupName = "";
	SGroup* parentGroup = nullptr;
	int depth = 0;
	int count = 0;
};

class PropertyWidget : public QWidget
{
	Q_OBJECT

public:
	PropertyWidget(QWidget* parent = nullptr);
	~PropertyWidget();
	QSplitter* _splitter = nullptr;
	class QVBoxLayout* _name_layout = nullptr;
	class QVBoxLayout* _value_layout = nullptr;
	//如果height = 0，则不会限制最大最小高度
	void AddItem(QString name, QWidget* widget, int Height = 30, SGroup* group = nullptr);

	SGroup* AddGroup(QString groupName, SGroup* parent = nullptr);

	void ClearItems();
	//显示最终结果
	void ShowItems();

	//<Group,Item>
	std::vector<std::shared_ptr<SGroup>> _groupCache;
	std::map<SGroup*, std::vector<SItem>> _items;

protected:
	void AddGroupButton(std::shared_ptr<SGroup> g);
	void ChildrenHidden(SGroup* group, bool bHidden);
};
