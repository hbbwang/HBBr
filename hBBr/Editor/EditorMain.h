#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EditorMain.h"
#include <qdockwidget.h>
#include <qtimer.h>
#include <qthread.h>

class CustomDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    CustomDockWidget(QWidget* parent);
};

class EditorMain : public QMainWindow
{
    Q_OBJECT

public:

    static  EditorMain* _self;

    EditorMain(QWidget *parent = nullptr);
    ~EditorMain();

    /* ����Ⱦ���� */
    class RenderView* _mainRenderView;

    //�������
    class SceneOutline* _sceneOutline = nullptr;
    class CustomDockWidget* _sceneOutline_dock = nullptr;

    //���ݹ�����
    class ContentBrowser* _contentBrowser = nullptr;
    class CustomDockWidget* _contentBrowser_dock = nullptr;

    //�����/�����༭��
    class Inspector* _inspector = nullptr;
    class CustomDockWidget* _inspector_dock = nullptr;

    class DirtyAssetsManager* ShowDirtyAssetsManager();

    virtual void closeEvent(QCloseEvent* event);

    virtual void resizeEvent(QResizeEvent* event)override;

    virtual void showEvent(QShowEvent* event)override;

    virtual void focusInEvent(QFocusEvent* event);

    virtual void focusOutEvent(QFocusEvent* event);

    //virtual void timerEvent(QTimerEvent* event)override;
    class CustomTitleBar* _customTitleBar = nullptr;
private:

    Ui::EditorMainClass ui;

    QTimer* _renderTimer;

private slots:

    void UpdateRender();
};

