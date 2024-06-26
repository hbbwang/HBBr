#pragma once

#include <QMainWindow>
#include <Material.h>
#include <qsplitter.h>
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

	class VulkanForm* _matWindow = nullptr;
	class QSplitter* left_right = nullptr;

	std::weak_ptr<Material> _material;
	class GameObject* _gameObject = nullptr;

	static 	QList<MaterialDetailEditor*> _allDetailWindows;

	static MaterialDetailEditor* OpenMaterialEditor(std::weak_ptr<Material> mat);

	static void CloseMaterialEditor(std::weak_ptr<Material> mat);

	static MaterialDetailEditor* RefreshMaterialEditor(std::weak_ptr<Material> mat);

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void closeEvent(QCloseEvent* event);
private:
	Ui::MaterialDetailEditor_MaterialAttribute ui_ma;
	Ui::MaterialDetailEditor_MaterialParameter ui_mp;
	Ui::MaterialDetailEditor_Renderer ui_r;

};
