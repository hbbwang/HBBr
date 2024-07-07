#pragma once

#include <QMainWindow>
#include <Material.h>
#include <map>
#include <qsplitter.h>
#include <HGuid.h>
#include <qdockwidget.h>
#include "ui_MaterialDetailEditor_MaterialAttribute.h"
#include "ui_MaterialDetailEditor_MaterialParameter.h"
#include "ui_MaterialDetailEditor_Renderer.h"
class MaterialDetailEditor : public QMainWindow
{
	Q_OBJECT

public:
	MaterialDetailEditor(std::weak_ptr<Material> mat ,QWidget *parent = nullptr);
	~MaterialDetailEditor();
	class MaterialEditor* _parent = nullptr;

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

	static MaterialDetailEditor* OpenMaterialEditor(std::weak_ptr<Material> mat);

	static void CloseMaterialEditor(std::weak_ptr<Material> mat);

	static MaterialDetailEditor* RefreshMaterialEditor(std::weak_ptr<Material> mat);

	static void RefreshAllMaterialEditor();

	QList<int> _left_right_sizes;
	QList<int> _up_bottom_sizes;

protected:
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void closeEvent(QCloseEvent* event);
private:
	void Init();
	void InitMP();
	void InitMA();
	Ui::MaterialDetailEditor_MaterialAttribute ui_ma;
	Ui::MaterialDetailEditor_MaterialParameter ui_mp;
	Ui::MaterialDetailEditor_Renderer ui_r;

};
