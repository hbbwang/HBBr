#pragma once

#include <QDialog>
#include "ui_WorldSelector.h"

class WorldSelector : public QDialog
{
	Q_OBJECT

public:
	WorldSelector(QWidget *parent = nullptr);
	~WorldSelector();
	class CustomListView* _listView = nullptr;
	void AddWorldItem();
	virtual int exec()override;
private:
	Ui::WorldSelectorClass ui;
};
