#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EditorMain.h"
#include <qtimer.h>

class EditorMain : public QMainWindow
{
    Q_OBJECT

public:
    EditorMain(QWidget *parent = nullptr);
    ~EditorMain();

    /* Ö÷äÖÈ¾´°¿Ú */
    class RenderView* _mainRenderView;

    virtual void closeEvent(QCloseEvent* event);

    virtual void resizeEvent(QResizeEvent* event)override;

    virtual void showEvent(QShowEvent* event)override;

    QTimer* _renderTimer;

    QWidget* _mainRenderer=NULL;

private:
    Ui::EditorMainClass ui;

private slots:

    void UpdateRender();

};

