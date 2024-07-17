#pragma once

#include <QMainWindow>
#include <Texture2D.h>
#include <map>
#include <qsplitter.h>
#include <HGuid.h>
#include <qdockwidget.h>
#include "ui_DetailEditor_Attribute.h"
#include "ui_DetailEditor_Parameter.h"
#include "ui_DetailEditor_Renderer.h"
class TextureDetailEditor : public QMainWindow
{
	Q_OBJECT
public:
	TextureDetailEditor(std::weak_ptr<Texture2D> tex ,QWidget *parent = nullptr);
	~TextureDetailEditor();
	class SDLWidget* _renderer = nullptr;
	class QSplitter* left_right = nullptr;
	class QWidget* _mp = nullptr;

	std::weak_ptr<Texture2D> _texture;
	class GameObject* _gameObject = nullptr;

	static 	QList<TextureDetailEditor*> _allDetailWindows;

	static TextureDetailEditor* OpenEditor(std::weak_ptr<Texture2D> mat);

	static void CloseEditor(std::weak_ptr<Texture2D> mat);

	static TextureDetailEditor* RefreshEditor(std::weak_ptr<Texture2D> mat);

	static void RefreshAllEditor();

	QList<int> _left_right_sizes;

	void deleteItem(QLayout* layout);

protected:
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void closeEvent(QCloseEvent* event);
private:
	void Init();
	Ui::DetailEditor_Parameter ui_mp;
	Ui::DetailEditor_Renderer ui_r;

};
