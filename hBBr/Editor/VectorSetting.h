#pragma once
#include <QWidget>
#include "ui_VectorSetting.h"
#include "FloatSetting.h"
#include "PropertyClass.h"
#include "glm/glm.hpp"
#include <functional>
class GameObject;

class VectorSetting : public PropertyClass
{
	Q_OBJECT

public:
	VectorSetting(QString name, QWidget* parent = Q_NULLPTR, const int demensionality = 1, float step = 0.01f, int precision = 3);
	~VectorSetting();

	QList<FloatSetting*> floatSetting ;
	std::function<void(QList<FloatSetting*>)> BindValue = [](QList<FloatSetting*>) {};
	float* _vec4_f[4] = {NULL,NULL,NULL,NULL};
	float _old_vec4_f[4] = { 0,0,0,0 };
	void SetValue(float x  ,float y  , float z  ,float w = 0);
	void SetValue(glm::vec4 v4 = glm::vec4(0));
	void SetValue(glm::vec3 v3 = glm::vec3(0));
	void SetValue(glm::vec2 v2 = glm::vec2(0));
	void SetValue(float v1 = 0);
	void Update()override;
	int Demensionality = 0;
	float GetX()const { return floatSetting[0]->GetValue(); }
	float GetY()const { if (Demensionality >= 2) return floatSetting[1]->GetValue(); return 0; }
	float GetZ()const { if (Demensionality >= 3) return floatSetting[2]->GetValue(); return 0;}
	float GetW()const { if (Demensionality >= 4) return floatSetting[3]->GetValue(); return 0;}

	Ui::VectorSetting ui;
	inline virtual void SetName(QString newName = "")
	{
		ui.Name->setText(newName);

	}
	virtual void paintEvent(QPaintEvent* event)override;
	int updatePoint;
	int maxUpdatePoint = 10;
public slots:
	void setX(double val);
	void setY(double val);
	void setZ(double val);
	void setW(double val);
};
