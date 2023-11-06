#include "VectorSetting.h"
#include "qmessagebox.h"
#include "GameObject.h"
#include "qtimer.h"

VectorSetting::VectorSetting(QString name, QWidget *parent,const int demensionality , float step , int precision )
	: PropertyClass(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name);
	//QFont font("Microsoft YaHei", 10, 50); 
	ui.Name->setObjectName("PropertyName");
	//ui.Name->setFont(font);
	//this->setStyleSheet("color:rgb(230,230,230);");
	Demensionality = demensionality;
	ui.horizontalLayout->setStretch(0 , 1);
	ui.horizontalLayout->setStretch(1,	1);
	setAttribute(Qt::WA_DeleteOnClose);
	for (int i = 0; i < demensionality; i++)
	{
		FloatSetting *f = new FloatSetting(this, 0, step, precision);
		//f->setMaximumWidth(80);
		//f->setFont(font);
		floatSetting.append (f);
		ui.horizontalLayout->addWidget(f);
		ui.horizontalLayout->setStretch(2+i ,0);
		//
		////f->setSuffix("|");
		if (demensionality > 1 && demensionality<=4)
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

	timer = new QTimer(this);
	timer->setInterval(200);
	timer->start();
	connect(timer, &QTimer::timeout, this, [this]() {
		if (Demensionality >= 1 && *_vec4_f[0] != _old_vec4_f[0] && !floatSetting[0]->hasFocus() && _vec4_f[0] != NULL)
		{
			floatSetting[0]->setValue(*_vec4_f[0]);
		}
		if (Demensionality >= 2 && *_vec4_f[1] != _old_vec4_f[1] && !floatSetting[1]->hasFocus() && _vec4_f[1] != NULL)
		{
			floatSetting[1]->setValue(*_vec4_f[1]);
		}
		if (Demensionality >= 3 && *_vec4_f[2] != _old_vec4_f[2] && !floatSetting[2]->hasFocus() && _vec4_f[2] != NULL)
		{
			floatSetting[2]->setValue(*_vec4_f[2]);
		}
		if (Demensionality >= 4 && *_vec4_f[3] != _old_vec4_f[3] && !floatSetting[3]->hasFocus() && _vec4_f[3] != NULL)
		{
			floatSetting[3]->setValue(*_vec4_f[3]);
		}
	});

}

VectorSetting::~VectorSetting()
{
	timer->stop();
	_vec4_f[0] = NULL;
	_vec4_f[1] = NULL;
	_vec4_f[2] = NULL;
	_vec4_f[3] = NULL;
	_v1 = NULL;
	_v4 = NULL;
	_v3 = NULL;
	_v2 = NULL;
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

void VectorSetting::updateValue()
{
	__super::updateValue();
	BindValue(floatSetting);
	switch (Demensionality)
	{
	case 1:
		if (_v1 != NULL)
		{
			*_v1 = floatSetting[0]->value();
		}
		break;
	case 2:
		if (_v2 != NULL)
		{
			_v2->x = floatSetting[0]->value();
			_v2->y = floatSetting[1]->value();
		}
		break;
	case 3:
		if (_v3 != NULL)
		{
			_v3->x = floatSetting[0]->value();
			_v3->y = floatSetting[1]->value();
			_v3->z = floatSetting[2]->value();
		}
		break;
	case 4:
		if (_v4 != NULL)
		{
			_v4->x = floatSetting[0]->value();
			_v4->y = floatSetting[1]->value();
			_v4->z = floatSetting[2]->value();
			_v4->w = floatSetting[3]->value();
		}
		break;
	}

	if (_vec4_f[0] != NULL && Demensionality >= 1 && floatSetting[0]->hasFocus())
	{
		*_vec4_f[0] = floatSetting[0]->value();
		_old_vec4_f[0] = *_vec4_f[0];
	}
	if (_vec4_f[1] != NULL && Demensionality >= 2 && floatSetting[1]->hasFocus())
	{
		*_vec4_f[1] = floatSetting[1]->value();
		_old_vec4_f[1] = *_vec4_f[1];
	}
	if (_vec4_f[2] != NULL && Demensionality >= 3 && floatSetting[2]->hasFocus())
	{
		*_vec4_f[2] = floatSetting[2]->value();
		_old_vec4_f[2] = *_vec4_f[2];
	}
	if (_vec4_f[3] != NULL && Demensionality >= 4 && floatSetting[3]->hasFocus())
	{
		*_vec4_f[3] = floatSetting[3]->value();
		_old_vec4_f[3] = *_vec4_f[3];
	}
}

void VectorSetting::paintEvent(QPaintEvent* event)
{
	__super::paintEvent(event);
}

void VectorSetting::closeEvent(QCloseEvent* event)
{
	__super::closeEvent(event);
	timer->stop();
	_vec4_f[0] = NULL;
	_vec4_f[1] = NULL;
	_vec4_f[2] = NULL;
	_vec4_f[3] = NULL;
	_v1 = NULL;
	_v4 = NULL;
	_v3 = NULL;
	_v2 = NULL;
}

void VectorSetting::BindV1(float* v1)
{
	if (v1 != NULL)
	{
		_v1 = v1;
	}
}

void VectorSetting::BindV2(glm::vec2* v2)
{
	if (v2 != NULL)
	{
		_v2 = v2;
	}
}

void VectorSetting::BindV3(glm::vec3* v3)
{
	if (v3 != NULL)
	{
		_v3 = v3;
	}
}

void VectorSetting::BindV4(glm::vec4* v4)
{
	if (v4 != NULL)
	{
		_v4 = v4;
	}
}

void VectorSetting::setX(double val)
{
	updateValue();
	if (Demensionality >= 1)
		floatSetting[0]->setValue(val);
	bindCallBack(0,val);
}

void VectorSetting::setY(double val)
{
	updateValue();
	if(Demensionality>=2)
		floatSetting[1]->setValue(val);
	bindCallBack(1, val);
}

void VectorSetting::setZ(double val)
{
	updateValue();
	if (Demensionality >= 3)
		floatSetting[2]->setValue(val);
	bindCallBack(2, val);
}

void VectorSetting::setW(double val)
{
	updateValue();
	if (Demensionality >= 4)
		floatSetting[3]->setValue(val);
	bindCallBack(3, val);
}
