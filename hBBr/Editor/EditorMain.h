#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EditorMain.h"

class EditorMain : public QMainWindow
{
    Q_OBJECT

public:
    EditorMain(QWidget *parent = nullptr);
    ~EditorMain();

    /* Ö÷äÖÈ¾´°¿Ú */
    class RenderView* _mainRenderView;

    virtual void closeEvent(QCloseEvent* event);

    virtual void focusInEvent(QFocusEvent* event)override;

    virtual void focusOutEvent(QFocusEvent* event)override;

    virtual void mousePressEvent(QMouseEvent* event)override;

    virtual void mouseReleaseEvent(QMouseEvent* event)override;

    bool eventFilter(QObject*, QEvent*)override;

private:
    Ui::EditorMainClass ui;
};
