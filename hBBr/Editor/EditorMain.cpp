#include "EditorMain.h"
#include "RenderView.h"
#include "SceneOutline.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include <QDesktopServices>
#include <QUrl>
#include <qdir.h>
#include <qfileinfo.h>

//鼠标钩子,适配窗口焦点
#include <Windows.h>
HHOOK mouseHook = NULL;
LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCT* mhookstruct = (MOUSEHOOKSTRUCT*)lParam;
    POINT pt = mhookstruct->pt;
    if (wParam == WM_RBUTTONDOWN
        || wParam == WM_LBUTTONDOWN
        || wParam == WM_RBUTTONDOWN
        || wParam == WM_MBUTTONDOWN
        )
    {
        auto widget = QApplication::widgetAt(pt.x, pt.y);
        if (widget && widget != QApplication::focusWidget() 
            && !widget->objectName().contains("menu",Qt::CaseInsensitive)//避免菜单栏
            )
        {
            //qDebug(widget->objectName().toStdString().c_str());
            //取消所有焦点先
            SetFocus(NULL);
            //再重新赋予QT焦点
            widget->setFocus();
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);//否则，如果返回给下一个钩子子程处理
}
//

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);

    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

    _sceneOutline = new SceneOutline(_mainRenderView->_mainRenderer->renderer, this);
    _sceneOutline_dock = new QDockWidget(this);
    _sceneOutline_dock->setWidget(_sceneOutline);
    _sceneOutline_dock->setWindowTitle("Scene Outline");
    _sceneOutline_dock->setObjectName("SceneOutline");
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _sceneOutline_dock);

    setObjectName("EditorMain");
    setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

    connect(ui.ResetWindowStyle, &QAction::triggered, this, [this](bool bChecked) {
        setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));
    });
    connect(ui.OpenProgramDir, &QAction::triggered, this, [this](bool bChecked) {
        //QDir dir(FileSystem::GetProgramPath().c_str());
        QFileInfo info(QDir::toNativeSeparators(FileSystem::GetProgramPath().c_str()));
        if (info.isDir())
        {
            QDesktopServices::openUrl(QUrl(info.absoluteFilePath()));
        }
    });

    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, GetModuleHandle(NULL), 0);//注册鼠标钩子
}

EditorMain::~EditorMain()
{
    if(mouseHook)
        UnhookWindowsHookEx(mouseHook);
}

void EditorMain::showEvent(QShowEvent* event)
{
	
}

void EditorMain::focusInEvent(QFocusEvent* event)
{
}

void EditorMain::focusOutEvent(QFocusEvent* event)
{
}

void EditorMain::closeEvent(QCloseEvent* event)
{
    _sceneOutline->close();
    _mainRenderView->close();

}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}
