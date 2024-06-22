#pragma once
#include <QTabWidget>
#include <memory>
#include "HString.h"
#include <QTabBar>
#include "Material.h"
#include "qdockwidget.h"

class MaterialDetailWindow :public QWidget
{
	Q_OBJECT
public:
	MaterialDetailWindow(std::weak_ptr<Material> mat, QWidget* parent = nullptr);
	~MaterialDetailWindow();

	virtual void paintEvent(QPaintEvent* event)override;
	virtual void resizeEvent(QResizeEvent* event)override;

	class MaterialEditor* _parent = nullptr;
	std::weak_ptr<Material> _material;
protected:
	virtual void closeEvent(QCloseEvent* event) override;
	class QSplitter* _splitter = nullptr;
	class QWidget* _left = nullptr;
	class QWidget* _right = nullptr;
	class QWidget* _left_up = nullptr;
	class QWidget* _left_bottom = nullptr;
};

class MaterialEditor : public QTabWidget
{
	Q_OBJECT

public:

	static MaterialEditor* _mainWindow;

	struct MaterialEditorItem {
		bool bTab = true;
		class MaterialDetailWindow* editor;
		//class QDockWidget* dock;
	};

	QList<MaterialEditorItem> _allDetailWindows;
	static MaterialDetailWindow* OpenMaterialEditor(std::weak_ptr<Material> mat , bool bTab); 

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
