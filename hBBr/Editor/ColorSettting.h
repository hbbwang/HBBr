#pragma once
#include <QWidget>
#include "ui_ColorSettting.h"
#include "qpainter.h"
#include "QWindow.h"
#include "QPixmap.h"
#include "qscreen.h"
#include "qdesktopwidget.h"
#include "qlabel.h"
#include "qslider.h"

#include "QLineEdit.h"
#include "qpushbutton.h"
#include "PropertyClass.h"
#define ColorMapPrecision 64

class ColorWindow : public QWidget
{
	Q_OBJECT

public:
	ColorWindow(QWidget* parent = Q_NULLPTR);
	~ColorWindow();
	virtual void resizeEvent(QResizeEvent* event);
	void paintEvent(QPaintEvent*);
	void UpdateColorMap();
	virtual void showEvent(QShowEvent* event);
	//virtual void hideEvent(QHideEvent* event);

	QColor RGB = QColor(255, 255, 255,255);
	//用于储存刚开始的颜色
	QColor backupRGB = QColor(255, 255, 255, 255);
	//QPoint m_pos;
	//
	QSlider* slider;
	QLabel* label;
	QLabel* labelPos;
	QPixmap ColorMap;

	QLineEdit* R_edit; 
	QLineEdit* G_edit;
	QLineEdit* B_edit;
	QLineEdit* A_edit;
	QLabel* R_Name;
	QLabel* G_Name;
	QLabel* B_Name;
	QLabel* A_Name;
	QLineEdit* H_edit;
	QLineEdit* S_edit;
	QLineEdit* V_edit;
	QLabel* H_Name;
	QLabel* S_Name;
	QLabel* V_Name;
	QLineEdit* lineEditHex;
	QLabel* Hex_Name;
	QPushButton* ok_Button;
	QPushButton* cancel_Button;
	void UpdateLineEdit();
	void UpdateColorButton();
	QPoint mapPos;

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	bool bPickColor = false;
	void PickColor();
private:
	//void ConvertRgbToHsv();
	//void ConvertHsvToRgb();
	bool cancel=false;
public slots:
	void HUBChanged();
	void SetR();
	void SetG();
	void SetB();
	void SetA();
	void SetHSV();
	void SetHexadecimal();
	void OKButton();
	void CancelButton();
};
//
class ColorSettting : public PropertyClass
{
	Q_OBJECT

public:
	ColorSettting(QString name, QWidget* parent = Q_NULLPTR);
	~ColorSettting();

	ColorWindow* colorWindow = nullptr;;
	//
	QColor RGB = QColor(255, 255, 255, 255);
	QColor GetRGBA() const {
		QColor out = QColor(RGB.red(), RGB.green(), RGB.blue(), RGB.alpha());
		return out;
	}
	
	Ui::ColorSettting ui;
public slots:
	void PickColor();
	void SetHexadecimal();
};

