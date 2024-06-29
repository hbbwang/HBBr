#pragma once
#include <QWidget>
#include "ui_ComboBox.h"
#include "qlineedit.h"
#include "PropertyClass.h"
class ComboBox : public PropertyClass
{
	Q_OBJECT

public:
	ComboBox(QString name, QWidget *parent = Q_NULLPTR);
	ComboBox(QWidget* parent );
	~ComboBox();

	void Init();

	inline QComboBox* GetComboBox()const { return ui.ComboBox_0; }

	inline void AddItem(QString newItem , QString oldSelection="") 
	{ 
		ui.ComboBox_0->blockSignals(true);
		ui.ComboBox_0->addItem(newItem);
		ui.ComboBox_0->blockSignals(false);
		if (oldSelection.size() > 1)
		{
			ui.ComboBox_0->setCurrentText(oldSelection);
		}
	}

	inline void AddItems(QStringList newItems, QString oldSelection = "")
	{
		for (auto i : newItems)
		{
			ui.ComboBox_0->blockSignals(true);
			ui.ComboBox_0->addItem(i);
			ui.ComboBox_0->blockSignals(false);
		}
		if (oldSelection.size() > 1)
		{
			ui.ComboBox_0->setCurrentText(oldSelection);
		}
	}

	inline int GetItemIndex(QString Item)
	{
		//return items.value(Item);
		return ui.ComboBox_0->findText(Item);
	}

	inline QString GetItemIndex(int index)
	{
		return ui.ComboBox_0->itemText(index);
	}

	inline QString GetCurrentSelection()const { return ui.ComboBox_0->currentText(); }

	inline bool CurrentSelectionItemCheck(QString Item)const
	{
		return ui.ComboBox_0->currentIndex() == ui.ComboBox_0->findText(Item);
	}

	inline bool CurrentSelectionItemCheck(int index)const
	{
		return ui.ComboBox_0->currentIndex() == index;
	}

	inline void ClearItems()const
	{
		ui.ComboBox_0->clear();
	}

	inline void RemoveItem(int index)const
	{
		ui.ComboBox_0->removeItem(index);
	}

	inline bool RemoveItem(QString Item)const
	{
		ui.ComboBox_0->removeItem(ui.ComboBox_0->findText(Item));
	}

	inline void SetCurrentSelection(QString ns) { ui.ComboBox_0->setCurrentText(ns); }

	inline int GetSelectionNum()const { return ui.ComboBox_0->count(); }

	void AdjustItemWidth();

	void AdjustComboBixWidth();

	Ui::ComboBox ui;

	std::function<void(const int index,const char*text)> _bindCurrentTextChanged = [](const int index, const char*) {};

private slots:
	void currentTextChanged(const QString&);
};
