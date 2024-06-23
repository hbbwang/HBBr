#pragma once
#include <qsplitter.h>
class PropertyWidget : public QWidget
{
	Q_OBJECT

public:
	PropertyWidget(QWidget* parent = nullptr);
	~PropertyWidget();
	QSplitter* _splitter = nullptr;
	class QVBoxLayout* _name_layout = nullptr;
	class QVBoxLayout* _value_layout = nullptr;
	void AddItem(QString name, QWidget* widget);
	void ClearItems();
protected:

};
