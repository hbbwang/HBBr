#include "ColorSettting.h"
#include "qlayout.h"
#include "qmessagebox.h"
#include <math.h>
#include <cmath>
#include "QEvent.h"

#include <windows.h>
#include "windef.h"
ColorSettting::ColorSettting(QString name, QWidget *parent)
	: PropertyClass(parent)
{
	ui.setupUi(this);

    colorWindow = new ColorWindow(this);
    //colorWindow->setWindowFlags(colorWindow->windowFlags()|Qt::FramelessWindowHint);
    this->setObjectName("ColorMain");
    colorWindow->setObjectName("ColorMain");
    colorWindow->setStyleSheet("#ColorMain{background:rgb(20,20,20);border:10px solid rgb(255,230,230); border-radius: 10px; }#Name{color:white;}");
    colorWindow->hide();
    connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(PickColor()));
    colorWindow->UpdateColorButton();
    ui.Name->setText(name);
    ui.Name->setObjectName("PropertyName");
    QFont font("Microsoft YaHei", 11, 50); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗
    ui.Name->setFont(font);
    ui.label->setFont(font);
    ui.lineEdit->setFont(font);
    //
    connect(ui.lineEdit, &QLineEdit::editingFinished, this, &ColorSettting::SetHexadecimal);
}
ColorSettting::~ColorSettting()
{
}
void ColorSettting::SetHexadecimal()
{
    colorWindow->lineEditHex->setText(ui.lineEdit->text());
    colorWindow->SetHexadecimal();
}
void ColorSettting::PickColor()
{
    //colorWindow->RGB.setRgba(RGB.rgba());
    colorWindow->show();
    QPoint p;
    p = QPoint(colorWindow->width()+75, colorWindow->height()/2);
    colorWindow->move(mapToGlobal(ui.pushButton->pos()- p));
}
/// <summary>
/// ///////////////////////////////////////////////////////////////////////////
/// </summary>
/// <param name="parent"></param>
ColorWindow::ColorWindow(QWidget* parent)
    : QWidget(parent)
{

    label = new QLabel(this);
    labelPos = new QLabel(this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);

    slider = new QSlider(this);

    //layout->addWidget(label);
    //setWindowFlag(Qt::Window,true);
    setWindowFlags(Qt::Window|Qt::CustomizeWindowHint );
    resize(300,256);
    setMinimumSize(430,276);
    setMaximumSize(430,276);
    label->resize(256,256);
    label->move(0+10, 0+10);
    labelPos->resize(10, 10);
    labelPos->setStyleSheet("border-radius:30px;");
    slider->resize(305 - 266, 256);
    slider->move(277,10);
    slider->setMaximum(360);//最大值为
    slider->setInvertedAppearance(true);//反向
    slider->setStyleSheet(
        "QSlider::groove \
    {\
        background:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, \
        stop:0 rgba(255, 0, 0, 255), \
        stop:0.166 rgba(255, 255, 0, 255), \
        stop:0.333 rgba(0, 255, 0, 255), \
        stop:0.5 rgba(0, 255, 255, 255), \
        stop:0.666 rgba(0, 0, 255, 255), \
        stop:0.833 rgba(255, 0, 255, 255), \
        stop:1 rgba(255, 0, 0, 255)\
        );\
        border: 2px solid rgb(10,10,10);\
        border-radius:20px; \
    }\
        QSlider::handle\
    {\
        background:rgb(255, 255, 255);\
        height: 13px; \
        border: 1px solid rgb(50,50,50);\
        border - radius: 4px;\
    }"
    );

    R_edit = new QLineEdit(this);
    G_edit = new QLineEdit(this);
    B_edit = new QLineEdit(this);
    A_edit = new QLineEdit(this);
    R_Name = new QLabel(this);
    G_Name = new QLabel(this);
    B_Name = new QLabel(this);
    A_Name = new QLabel(this);
    H_edit = new QLineEdit(this);
    S_edit = new QLineEdit(this);
    V_edit = new QLineEdit(this);
    H_Name = new QLabel(this);
    S_Name = new QLabel(this);
    V_Name = new QLabel(this);
    lineEditHex = new QLineEdit(this);
    Hex_Name = new QLabel(this);
    QFont font("Microsoft YaHei", 11, 50); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗
    R_Name->resize(330 - 305, 26);
    R_Name->move(316, 0+10);
    R_Name->setText("R");
    R_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    R_Name->setFont(font);
    R_Name->setObjectName("Name");
    G_Name->resize(330 - 305, 26);
    G_Name->move(316, 26 +10);
    G_Name->setText("G");
    G_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    G_Name->setFont(font);
    G_Name->setObjectName("Name");
    B_Name->resize(330 - 305, 26);
    B_Name->move(316, 26*2 + 10);
    B_Name->setText("B");
    B_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    B_Name->setFont(font);
    B_Name->setObjectName("Name");
    A_Name->resize(330 - 305, 26);
    A_Name->move(316, 26 * 3 + 10);
    A_Name->setText("A");
    A_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    A_Name->setFont(font);
    A_Name->setObjectName("Name");
    R_edit->resize(390 - 330, 26);
    R_edit->move(345, 0 + 10);
    R_edit->setFont(font);
    R_edit->setAlignment(Qt::AlignRight);
    G_edit->resize(390 - 330, 26);
    G_edit->move(345, 26 + 10);
    G_edit->setFont(font);
    G_edit->setAlignment(Qt::AlignRight);
    B_edit->resize(390 - 330, 26);
    B_edit->move(345, 26*2 + 10);
    B_edit->setFont(font);
    B_edit->setAlignment(Qt::AlignRight);
    A_edit->resize(390 - 330, 26);
    A_edit->move(345, 26 * 3 + 10);
    A_edit->setFont(font);
    A_edit->setAlignment(Qt::AlignRight);
    //
    H_Name->resize(330 - 305, 26);
    H_Name->move(316, 26*4 + 6 + 10);
    H_Name->setText("H");
    H_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    H_Name->setFont(font);
    H_Name->setObjectName("Name");
    S_Name->resize(330 - 305, 26);
    S_Name->move(316, 26*5 + 6 + 10);
    S_Name->setText("S");
    S_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    S_Name->setFont(font);
    S_Name->setObjectName("Name");
    V_Name->resize(330 - 305, 26);
    V_Name->move(316, 26 * 6 + 6 + 10);
    V_Name->setText("V");
    V_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    V_Name->setFont(font);
    V_Name->setObjectName("Name");
    H_edit->resize(390 - 330, 26);
    H_edit->move(345, 26 * 4 + 6 + 10 );
    H_edit->setFont(font);
    H_edit->setAlignment(Qt::AlignRight);
    S_edit->resize(390 - 330, 26);
    S_edit->move(345, 26*5 + 6 + 10);
    S_edit->setFont(font);
    S_edit->setAlignment(Qt::AlignRight);
    V_edit->resize(390 - 330, 26);
    V_edit->move(345, 26 * 6 + 6 + 10);
    V_edit->setFont(font);
    V_edit->setAlignment(Qt::AlignRight);
    //
    Hex_Name->resize(330 - 305, 26);
    Hex_Name->move(316, 26 * 7 + 11 + 10);
    Hex_Name->setText("#");
    Hex_Name->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    Hex_Name->setFont(font);
    Hex_Name->setObjectName("Name");
    lineEditHex->resize(400 - 330, 26);
    lineEditHex->move(340, 26 * 7 + 11 + 10);
    lineEditHex->setFont(font);
    lineEditHex->setAlignment(Qt::AlignRight);
    //
    ok_Button =new QPushButton(this);
    ok_Button->setFont(font);
    ok_Button->setText("OK");
    ok_Button->resize(36 , 30);
    ok_Button->move(318, 26 * 8 + 15 + 10);
    ok_Button->setStyleSheet("border: 2px solid rgb(10,10,10);border-radius:6px; background-color:rgb(180,180,180);");
    cancel_Button = new QPushButton(this);
    cancel_Button->setFont(font);
    cancel_Button->setText("Cancel");
    cancel_Button->resize(65, 30);
    cancel_Button->move(354, 26 * 8 + 15 + 10);
    cancel_Button->setStyleSheet("border: 2px solid rgb(10,10,10);border-radius:6px; background-color:rgb(180,180,180);");
    int width = label->width();
    int height = label->height();
    ColorMap = QPixmap(width/(width / ColorMapPrecision), height/(height / ColorMapPrecision));

    //connect(slider, &QSlider::sliderMoved, this, &ColorWindow::HUBChanged);
    //connect(slider, &QSlider::sliderReleased, this, &ColorWindow::HUBChanged);
    connect(R_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetR);
    connect(G_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetG);
    connect(B_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetB);
    connect(A_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetA);
    connect(H_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetHSV);
    connect(S_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetHSV);
    connect(V_edit, &QLineEdit::editingFinished, this, &ColorWindow::SetHSV);
    //
    connect(ok_Button, &QPushButton::clicked, this, &ColorWindow::OKButton);
    connect(cancel_Button, &QPushButton::clicked, this, &ColorWindow::CancelButton);
    connect(lineEditHex, &QLineEdit::editingFinished, this, &ColorWindow::SetHexadecimal);
}
void ColorWindow::SetR()
{
    RGB.setRed(R_edit->text().toInt());
    slider->setValue(RGB.hue());
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::SetG()
{
    RGB.setGreen(G_edit->text().toInt());
    slider->setValue(RGB.hue());
    UpdateColorMap(); 
    UpdateLineEdit();
}
void ColorWindow::SetB()
{
    RGB.setBlue(B_edit->text().toInt());
    slider->setValue(RGB.hue());
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::SetA()
{
    RGB.setAlpha(A_edit->text().toInt());
    slider->setValue(RGB.hue());
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::SetHSV()
{
    RGB.setHsv(H_edit->text().toInt(), S_edit->text().toInt(), V_edit->text().toInt(), A_edit->text().toInt());
    slider->setValue(RGB.hue());
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::SetHexadecimal()
{
    float alpha = RGB.alpha();
    RGB = QColor(lineEditHex->text().toUInt(nullptr, 16));
    RGB.setAlpha(alpha);
    slider->setValue(RGB.hue());
    QPoint _pos = QPoint(((float)RGB.saturation() / (float)255) * label->width(), ((float)(255 - RGB.value()) / (float)255) * label->height());
    labelPos->move(_pos);
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::UpdateColorButton()
{
    ColorSettting* out = reinterpret_cast<ColorSettting*>(parentWidget());
    out->RGB.setRgba(RGB.rgba());
    QString borderCol = "rgb(";
    borderCol.append(QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ")");
    QString style;
    style = "background-color:rgb(";
    style.append(QString::number(RGB.red()) + "," + QString::number(RGB.green()) + "," + QString::number(RGB.blue()) + ");border:3px solid " + borderCol + ";border-radius:6px;height:25;");
    out->ui.pushButton->setStyleSheet(style);
    //
    QString hex = QString::number(RGB.rgb(), 16);
    out->ui.lineEdit->setText(hex.remove(0, 2));//移除最前面的FF（不透明度）
}
void ColorWindow::OKButton()
{
    ColorSettting* out = reinterpret_cast<ColorSettting*>(parentWidget());
    out->RGB.setRgba64(RGB.rgba64());
    QString borderCol = "rgb(";
    borderCol.append(QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ")");
    QString style;
    style = "background-color:rgb(";
    style.append(QString::number(RGB.red()) + "," + QString::number(RGB.green()) + "," + QString::number(RGB.blue()) + ");border:3px solid " + borderCol + ";border-radius:6px;height:25;");
    out->ui.pushButton->setStyleSheet(style);
    close();
}
void ColorWindow::CancelButton()
{
    //关闭，撤销回之前的设置
    RGB.setRgba64(backupRGB.rgba64());
    ColorSettting* out = reinterpret_cast<ColorSettting*>(parentWidget());
    out->RGB.setRgba64(RGB.rgba64());
    QString borderCol = "rgb(";
    borderCol.append(QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ")");
    QString style;
    style = "background-color:rgb(";
    style.append(QString::number(RGB.red()) + "," + QString::number(RGB.green()) + "," + QString::number(RGB.blue()) + ");border:3px solid " + borderCol + ";border-radius:6px;height:25;");
    out->ui.pushButton->setStyleSheet(style);

    cancel = true;
    close();
}
void ColorWindow::showEvent(QShowEvent* event)
{
    backupRGB.setRgba64(RGB.rgba64());
    slider->setValue(RGB.hue());
    ok_Button->clearFocus();
    cancel_Button->clearFocus();
    //读取吸取点
    QPoint _pos = QPoint(  ((float)RGB.saturation() / (float)255 )* label->width(), ((float)(255 - RGB.value()) / (float)255 ) * label->height());
    labelPos->move(_pos);
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::HUBChanged()
{
    RGB.setHsv(slider->value(), RGB.saturation(), RGB.value(), RGB.alpha());
    UpdateColorMap();
    UpdateLineEdit();
}
void ColorWindow::UpdateLineEdit()
{
    ColorSettting* colorSetting = reinterpret_cast<ColorSettting*>(parentWidget());
    R_edit->setText(QString::number(RGB.red()));
    G_edit->setText(QString::number(RGB.green()));
    B_edit->setText(QString::number(RGB.blue()));
    A_edit->setText(QString::number(RGB.alpha()));
    H_edit->setText(QString::number(slider->value()));
    S_edit->setText(QString::number(RGB.saturation()));
    V_edit->setText(QString::number(RGB.value()));
    QString hex = QString::number(RGB.rgb(), 16);
    lineEditHex->setText(hex.remove(0, 2));//移除最前面的FF（不透明度）
    colorSetting->ui.lineEdit->setText(hex);
}
void ColorWindow::mousePressEvent(QMouseEvent* ev)
{
    if (ev->pos().x() <= label->x() + label->width() && ev->pos().y() < label->y() + label->height())
    {
        R_edit->clearFocus();
        G_edit->clearFocus();
        B_edit->clearFocus();
        A_edit->clearFocus();
        H_edit->clearFocus();
        S_edit->clearFocus();
        V_edit->clearFocus();
        lineEditHex->clearFocus();
        slider->setFocus();
        bPickColor = true;
        RECT mainRect; //windef.h中被定义
        mainRect.left = (LONG)label->mapToGlobal(QPoint(0,0)).x();
        mainRect.right = (LONG)label->mapToGlobal(QPoint(0, 0)).x() + label->width();
        mainRect.top = (LONG)label->mapToGlobal(QPoint(0, 0)).y();
        mainRect.bottom = (LONG)label->mapToGlobal(QPoint(0, 0)).y() + label->height();
        ClipCursor(&mainRect);
        
    }
    if (bPickColor)
    {
        labelPos->setStyleSheet("background-color:transparent;border: 0px;");
    }
}
void ColorWindow::mouseReleaseEvent(QMouseEvent* ev)
{
    if (bPickColor == true)
    {
        bPickColor = false;
        PickColor();
        QWidget::mouseReleaseEvent(ev);
        ClipCursor(nullptr);
        QPoint p;
        p = ev->pos() - QPoint(labelPos->width() / 2, labelPos->height() / 2);
        labelPos->move(p);
        //
        QString style;
        style = "background-color:rgb(";
        style.append(QString::number(RGB.red()) + "," + QString::number(RGB.green()) + "," + QString::number(RGB.blue()) + ");");
        QString borderColor;
        borderColor = "rgb(";
        borderColor.append(QString::number(255 - RGB.red()) + "," + QString::number(255 - RGB.green()) + "," + QString::number(255 - RGB.blue()) + ")");
        labelPos->setStyleSheet(" background-color:transparent; border: 1px solid " + borderColor + ";border-radius:30px;");
    }
}
void ColorWindow::mouseMoveEvent(QMouseEvent* ev)
{
}
void ColorWindow::closeEvent(QCloseEvent* event)
{
    ok_Button->clearFocus();
    cancel_Button->clearFocus();
    bPickColor = false;
    cancel = false;
    UpdateLineEdit();
}
void ColorWindow::keyReleaseEvent(QKeyEvent* ev)
{
    //if (ev->key() == Qt::Key_Space || ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter)
    if (ev->key() == Qt::Key_Space || ev->key() == Qt::Key_Enter)
    {
        OKButton();
    }
    else if (ev->key() == Qt::Key_Escape)
    {
        CancelButton();
    }
}
void ColorWindow::PickColor()
{
    //获取鼠标x，y坐标
    int x = QCursor::pos().x();
    int y = QCursor::pos().y();

    //获取坐标像素点
    //QPixmap pixmap = label->grab(QRect(QCursor::pos(), QSize(1, 1)));
    //获取坐标像素点
    QWindow window;
    QPixmap pixmap = window.screen()->grabWindow(QApplication::desktop()->winId(), x, y, 1, 1);
    //获取坐标像素点
    //获取像素点RGB
    int red = 0, green = 0, blue = 0;
    if (!pixmap.isNull())
    {
        QImage image = pixmap.toImage();
        if (!image.isNull())
        {
            QColor color = image.pixel(0, 0);
            red = color.red();
            green = color.green();
            blue = color.blue();
            RGB = QColor(red, green, blue, RGB.alpha());
            //
        }
    }
}
ColorWindow::~ColorWindow()
{

}
void ColorWindow::resizeEvent(QResizeEvent* event)
{
    UpdateColorMap();
}
void ColorWindow::paintEvent(QPaintEvent* p)//paintEvent函数由系统自动调用，用不着我们人为的去调用。
{
    if (slider->hasFocus()&& !R_edit->hasFocus() && !G_edit->hasFocus() && !B_edit->hasFocus() && !A_edit->hasFocus()
        && !H_edit->hasFocus()&& !S_edit->hasFocus() &&!V_edit->hasFocus())
    {
        HUBChanged();
        H_edit->setText(QString::number(slider->value()));
    }
    if (bPickColor)
    {
        PickColor();
    }
}
void ColorWindow::UpdateColorMap()
{
    ColorSettting* colorSetting = reinterpret_cast<ColorSettting*>(parentWidget());
    {
        //获取像素点RGB
        int red, green, blue;
        QColor out = QColor(0, 0, 0, 255);
        QColor col = QColor(RGB);
        col.setHsv(RGB.hue(), 255, 255);
        red = col.red();
        green = col.green();
        blue = col.blue();
        //设置label标签的颜色显示
        QPainter pt(&ColorMap);
        int pre = ColorMapPrecision;
        pre -= 1;
        for (int i = 0; i <= pre; ++i)
        {
            for (int j = 0; j <= pre; ++j)
            {
                float _red = ((float)i / (float)pre) * ((float)j / (float)pre) * red / 255;
                float _green = ((float)i / (float)pre) * ((float)j / (float)pre) * green / 255;
                float _blue = ((float)i / (float)pre) * ((float)j / (float)pre) * blue / 255;
                _red += ((float)i / (float)pre) * (1 - ((float)j / (float)pre));
                _green += ((float)i / (float)pre) * (1 - ((float)j / (float)pre));
                _blue += ((float)i / (float)pre) * (1 - ((float)j / (float)pre));

                out.setRgb(_red * 255,
                    _green * 255,
                    _blue * 255
                );
                pt.setPen(out);
                pt.drawPoint(j, pre - i);
            }
        }
        label->setPixmap(ColorMap.scaled(label->width(), label->height(), Qt::KeepAspectRatio));
        //输出16进制的颜色
        //QString hRed = QString::number(RGB.red(),16).toUpper();
        //QString hGreen = QString::number(RGB.green(), 16).toUpper();
        //QString hBlue = QString::number(RGB.blue(), 16).toUpper();
        //lineEditHex->setText(tr("#%1%2%3").arg(hRed).arg(hGreen).arg(hBlue));
        QString hex = QString::number(RGB.rgb(), 16);
        lineEditHex->setText(hex.remove(0, 2));//移除最前面的FF（不透明度）
        colorSetting->ui.lineEdit->setText(hex);//移除最前面的FF（不透明度）
    }
    //更新外部按钮颜色
    QString style;
    colorSetting->RGB.setRgba(RGB.rgba());
    QString borderCol = "rgb(";
    borderCol.append(QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ", " + QString::number(255 - RGB.value()) + ")");
    style = "background-color:rgb(";
    style.append(QString::number(RGB.red()) + "," + QString::number(RGB.green()) + "," + QString::number(RGB.blue()) + ");border:3px solid " + borderCol + ";border-radius:6px;height:25;");
    colorSetting->ui.pushButton->setStyleSheet(style);
}