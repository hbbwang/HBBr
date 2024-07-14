#include "VectorSetting.h"
#include "qmessagebox.h"
#include "GameObject.h"
#include "qtimer.h"

VectorSetting::VectorSetting(QString name, QWidget *parent,const int demensionality , float step , int precision )
	: PropertyClass(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name);
	ui.Name->setObjectName("PropertyName");
	Init(parent, demensionality, step, precision);
}

VectorSetting::VectorSetting(QWidget* parent, const int demensionality, float step, int precision) 
	: PropertyClass(parent)
{
	ui.setupUi(this);
	Init(parent, demensionality, step, precision);
	ui.horizontalLayout->removeItem(ui.horizontalSpacer);
	ui.Name->setHidden(true);
	ui.horizontalLayout->setStretch(0, 0);
	ui.horizontalLayout->setStretch(1, 0);
}

VectorSetting::~VectorSetting()
{
	BindValue = [](QList<FloatSetting*>) {};
	maxUpdatePoint = 0;
	_vec4_f[0] = nullptr;
	_vec4_f[1] = nullptr;
	_vec4_f[2] = nullptr;
	_vec4_f[3] = nullptr;
	_old_vec4_f[0] = 0;
	_old_vec4_f[1] = 0;
	_old_vec4_f[2] = 0;
	_old_vec4_f[3] = 0;
	_vec2_glm = nullptr;
	_vec3_glm = nullptr;
	_vec4_glm = nullptr;
}

void VectorSetting::Init(QWidget* parent, const int demensionality, float step, int precision)
{
	Demensionality = demensionality;
	ui.horizontalLayout->setStretch(0, 1);
	ui.horizontalLayout->setStretch(1, 1);
	setAttribute(Qt::WA_DeleteOnClose);
	_vec4_f[0] = nullptr;
	_vec4_f[1] = nullptr;
	_vec4_f[2] = nullptr;
	_vec4_f[3] = nullptr;
	_vec2_glm = nullptr;
	_vec3_glm = nullptr;
	_vec4_glm = nullptr;
	for (int i = 0; i < demensionality; i++)
	{
		FloatSetting* f = new FloatSetting(this, 0, step, precision);
		//f->setMaximumWidth(80);
		//f->setFont(font);
		floatSetting.append(f);
		ui.horizontalLayout->addWidget(f);
		ui.horizontalLayout->setStretch(2 + i, 0);
		//
		////f->setSuffix("|");
		if (demensionality > 1 && demensionality <= 4)
		{
			switch (i)
			{
			case 0:
				f->setObjectName("PropertySpinBox_X");
				connect(f, SIGNAL(valueChanged(double)), this, SLOT(setX(double)));
				break;
			case 1:
				f->setObjectName("PropertySpinBox_Y");
				connect(f, SIGNAL(valueChanged(double)), this, SLOT(setY(double)));
				break;
			case 2:
				f->setObjectName("PropertySpinBox_Z");
				connect(f, SIGNAL(valueChanged(double)), this, SLOT(setZ(double)));
				break;
			case 3:
				f->setObjectName("PropertySpinBox_W");
				connect(f, SIGNAL(valueChanged(double)), this, SLOT(setW(double)));
				break;
			}
		}
		else
		{
			f->setObjectName("PropertySpinBox_W");
			connect(f, SIGNAL(valueChanged(double)), this, SLOT(setX(double)));
		}
	}

	updatePoint = 0;
	//3帧更新一次值
	maxUpdatePoint = 3;
}

void VectorSetting::SetValue(float x, float y , float z , float w)
{
	for (int i = 0; i < Demensionality; i++)
	{
		switch (i)
		{
		case 0 :
			floatSetting[0]->setValue(x);
			break;
		case 1:
			floatSetting[1]->setValue(y);
			break;
		case 2:
			floatSetting[2]->setValue(z);
			break;
		case 3:
			floatSetting[3]->setValue(w);
			break;
		}
	}
}

void VectorSetting::SetValue(float v1)
{
	if (Demensionality >=1)
	{
		floatSetting[0]->setValue(v1);
	}
}

void VectorSetting::Update()
{
	if (updatePoint > maxUpdatePoint)
	{
		updatePoint = 0;
		if (Demensionality >= 1 && !floatSetting[0]->hasFocus())
		{
			if (_vec4_f[0] != nullptr && *_vec4_f[0] != _old_vec4_f[0])
			{
				floatSetting[0]->setValue(*_vec4_f[0]);
			}
			if (_vec2_glm != nullptr)
			{
				floatSetting[0]->setValue(_vec2_glm->x);
			}
			if (_vec3_glm != nullptr)
			{
				floatSetting[0]->setValue(_vec3_glm->x);
			}
			if (_vec4_glm != nullptr)
			{
				floatSetting[0]->setValue(_vec4_glm->x);
			}
		}
		if (Demensionality >= 2 && !floatSetting[1]->hasFocus())
		{
			if (_vec4_f[1] != nullptr && *_vec4_f[1] != _old_vec4_f[1])
			{
				floatSetting[1]->setValue(*_vec4_f[1]);
			}
			if (_vec2_glm != nullptr)
			{
				floatSetting[1]->setValue(_vec2_glm->y);
			}
			if (_vec3_glm != nullptr)
			{
				floatSetting[1]->setValue(_vec3_glm->y);
			}
			if (_vec4_glm != nullptr)
			{
				floatSetting[1]->setValue(_vec4_glm->y);
			}
		}
		if (Demensionality >= 3 && !floatSetting[2]->hasFocus())
		{
			if (_vec4_f[2] != nullptr && *_vec4_f[2] != _old_vec4_f[2])
			{
				floatSetting[2]->setValue(*_vec4_f[2]);
			}
			if (_vec3_glm != nullptr)
			{
				floatSetting[3]->setValue(_vec3_glm->z);
			}
			if (_vec4_glm != nullptr)
			{
				floatSetting[3]->setValue(_vec4_glm->z);
			}
		}
		if (Demensionality >= 4 && !floatSetting[3]->hasFocus())
		{
			if (_vec4_f[3] != nullptr && *_vec4_f[3] != _old_vec4_f[3])
			{
				floatSetting[3]->setValue(*_vec4_f[3]);
			}	
			if (_vec4_glm != nullptr)
			{
				floatSetting[3]->setValue(_vec4_glm->w);
			}
		}
	}
	else
	{
		updatePoint++;
	}
}

void VectorSetting::SetValue(glm::vec2 v2)
{
	if (Demensionality >= 2)
	{
		floatSetting[0]->setValue(v2.x);
		floatSetting[1]->setValue(v2.y);
	}
}

void VectorSetting::SetValue(glm::vec3 v3)
{
	if (Demensionality >= 3)
	{
		floatSetting[0]->setValue(v3.x);
		floatSetting[1]->setValue(v3.y);
		floatSetting[2]->setValue(v3.z);
	}
}

void VectorSetting::SetValue(glm::vec4 v4) 
{
	if (Demensionality >= 4)
	{
		floatSetting[0]->setValue(v4.x);
		floatSetting[1]->setValue(v4.y);
		floatSetting[2]->setValue(v4.z);
		floatSetting[3]->setValue(v4.w);
	}
}

void VectorSetting::paintEvent(QPaintEvent* event)
{
	__super::paintEvent(event);
}

void VectorSetting::setX(double val)
{
	if (Demensionality >= 1)
		floatSetting[0]->setValue(val);
	BindValue(floatSetting);
	if (Demensionality >= 1)
	{
		if (_vec4_f[0] != nullptr)
		{
			*_vec4_f[0] = floatSetting[0]->value();
			_old_vec4_f[0] = *_vec4_f[0];
		}
		if (_vec2_glm != nullptr)
		{
			_vec2_glm->x = floatSetting[0]->value();
		}
		if (_vec3_glm != nullptr)
		{
			_vec3_glm->x = floatSetting[0]->value();
		}
		if (_vec4_glm != nullptr)
		{
			_vec4_glm->x = floatSetting[0]->value();
		}
	}
}

void VectorSetting::setY(double val)
{
	if(Demensionality>=2)
		floatSetting[1]->setValue(val);
	BindValue(floatSetting);
	if (Demensionality >= 2)
	{
		if (_vec4_f[1] != nullptr)
		{
			*_vec4_f[1] = floatSetting[1]->value();
			_old_vec4_f[1] = *_vec4_f[1];
		}
		if (_vec2_glm != nullptr)
		{
			_vec2_glm->y = floatSetting[1]->value();
		}
		if (_vec3_glm != nullptr)
		{
			_vec3_glm->y = floatSetting[1]->value();
		}
		if (_vec4_glm != nullptr)
		{
			_vec4_glm->y = floatSetting[1]->value();
		}
	}
}

void VectorSetting::setZ(double val)
{
	if (Demensionality >= 3)
		floatSetting[2]->setValue(val);
	BindValue(floatSetting);
	if (Demensionality >= 3)
	{
		if (_vec4_f[2] != nullptr)
		{
			*_vec4_f[2] = floatSetting[2]->value();
			_old_vec4_f[2] = *_vec4_f[2];
		}
		if (_vec3_glm != nullptr)
		{
			_vec3_glm->z = floatSetting[2]->value();
		}
		if (_vec4_glm != nullptr)
		{
			_vec4_glm->z = floatSetting[2]->value();
		}
	}
}

void VectorSetting::setW(double val)
{
	if (Demensionality >= 4)
		floatSetting[3]->setValue(val);
	BindValue(floatSetting);
	if (Demensionality >= 4)
	{
		if (_vec4_f[3] != nullptr)
		{
			*_vec4_f[3] = floatSetting[3]->value();
			_old_vec4_f[3] = *_vec4_f[3];
		}
		if (_vec4_glm != nullptr)
		{
			_vec4_glm->w = floatSetting[3]->value();
		}
	}
}
