#pragma once
#include <QWidget>

class PropertyClass : public QWidget
{
	Q_OBJECT

public:
	PropertyClass(QWidget *parent = Q_NULLPTR);

	~PropertyClass();

	virtual void SetName(QString newName = "");
	
	virtual void Update() {
	}

	void* TempObject = NULL;

};
