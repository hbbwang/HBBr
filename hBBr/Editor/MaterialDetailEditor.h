#pragma once

#include <QMainWindow>
#include <Material.h>
#include <map>
#include <qsplitter.h>
#include <HGuid.h>
#include <qdockwidget.h>
#include "ui_DetailEditor_Attribute.h"
#include "ui_DetailEditor_Parameter.h"
#include "ui_DetailEditor_Renderer.h"
class MaterialDetailEditor : public QMainWindow
{
	Q_OBJECT

public:
	MaterialDetailEditor(std::weak_ptr<Material> mat ,QWidget *parent = nullptr);
	~MaterialDetailEditor();

	class SDLWidget* _renderer = nullptr;
	//class QSplitter* left_right = nullptr;
	class QSplitter* up_bottom = nullptr;

	QWidget* r;
	QWidget* ma = nullptr;
	QDockWidget* mp = nullptr;
	class PropertyWidget* pw_ma ;

	std::weak_ptr<Material> _material;
	class GameObject* _gameObject = nullptr;

	static 	QList<MaterialDetailEditor*> _allDetailWindows;

	static MaterialDetailEditor* OpenEditor(std::weak_ptr<Material> mat);

	static void CloseEditor(std::weak_ptr<Material> mat);

	static MaterialDetailEditor* RefreshEditor(std::weak_ptr<Material> mat);

	static void RefreshAllEditor();

	QList<int> _left_right_sizes;
	QList<int> _up_bottom_sizes;

	void deleteItem(QLayout* layout);

protected:
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void closeEvent(QCloseEvent* event);
private:
	void Init();
	void InitMP();
	void InitMA();
	Ui::DetailEditor_Attribute ui_ma;
	Ui::DetailEditor_Parameter ui_mp;
	Ui::DetailEditor_Renderer ui_r;

};
