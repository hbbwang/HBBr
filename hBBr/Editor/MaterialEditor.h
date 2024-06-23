#pragma once
#include <QTabWidget>
#include <memory>
#include "HString.h"
#include <QTabBar>
#include "Material.h"
#include "qdockwidget.h"
#include "MaterialDetailEditor.h"

class MaterialEditor : public QTabWidget
{
	Q_OBJECT

public:

	static MaterialEditor* _mainWindow;

	struct MaterialEditorItem {
		class MaterialDetailEditor* editor;
		bool bTab = true;
		//class QDockWidget* dock;
	};

	QList<MaterialEditorItem> _allDetailWindows;
	static MaterialDetailEditor* OpenMaterialEditor(std::weak_ptr<Material> mat , bool bTab); 

protected:
	MaterialEditor(QWidget *parent = nullptr);
	~MaterialEditor();
	virtual void closeEvent(QCloseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event)override;
	virtual bool eventFilter(QObject* watched, QEvent* event)override;
private:
	QPoint startPos;
	int index;
	QWidget* currentDragWidget = nullptr;
	bool bInDrag = false;
	bool bResetDrag = false;
	//Ui::MaterialEditorClass ui;
};
