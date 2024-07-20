#include "EditorMain.h"
#include <QtWidgets/QApplication>
#include <QMouseEvent>
#include <qdebug.h>
#include "FormMain.h"
#include "RenderView.h"
#include "ContentBrowser.h"
#include "qfontdatabase.h"
#include "ConsoleDebug.h"
#include "EditorCommonFunction.h"
#include <Windows.h> //Ϊ��֧��SetFocus(nullptr);
#include "CustomTitleBar.h"

bool _bLeftButtonPress_ForResize = false;
CustomTitleBar* _titleBar;

class MyEventFilter : public QObject
{
protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        bool bChangeFocus = false;
        if (event->type() == QEvent::MouseButtonPress && watched->isWidgetType())
        {
            //QWidget* newCurrentFocusWidget = QApplication::widgetAt(mouseEvent->globalPos());
            QWidget* newCurrentFocusWidget = (QWidget*)watched;
            auto cursorPos = mouseEvent->globalPos();
            if (newCurrentFocusWidget)
            {
                //����ObjectName����RenderView�Ż�ǿ�Ƹ��轹��
                if (newCurrentFocusWidget->objectName().contains("SDL", Qt::CaseSensitive))
                {
                    POINT point = {};
                    point.x = mouseEvent->globalPos().x();
                    point.y = mouseEvent->globalPos().y();
                    HWND windowAt = WindowFromPoint(point);

                    auto forms = VulkanApp::GetForms();
                    for (auto& i : forms)
                    {              
                        auto winHwnd = (HWND)VulkanApp::GetWindowHandle(i);
                        if (windowAt == winHwnd)
                        {
                            VulkanApp::SetFocusForm(i);
                            SetFocus((HWND)VulkanApp::GetWindowHandle(i));
                        }
                    }
                }
            }
            //ContentBrowser focus
            for (auto& cb : ContentBrowser::GetContentBrowsers())
            {
                if (cb)
                {
                    QPoint localCursorPos = cb->mapFromGlobal(mouseEvent->globalPos());
                    if (cb->rect().contains(localCursorPos))
                    {
                        ContentBrowser::SetCurrentBrowser(cb);
                        break;
                    }
                }
            }
        }
        if (event->type() == QEvent::MouseMove)
        {
            if (watched == _titleBar)
            {
                _titleBar->MouseMoveUpdate();
            }
            else if (watched->isWidgetType())
            {
                QWidget* widget = (QWidget*)watched;
                if (widget->cursor() != Qt::ArrowCursor)
                    widget->setCursor(Qt::ArrowCursor);
            }
        }
        return false; // �¼�δ��������������
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString rootPath = QCoreApplication::applicationDirPath();

    MyEventFilter filter;
    a.installEventFilter(&filter); // ��װ�¼�������

    //���촰��
    EditorMain w;

    //��������
    auto allFonts = FileSystem::GetAllFilesExceptFolders(FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Fonts").c_str());
    for (auto& i : allFonts)
    {
        QString fontPath = QString::fromStdWString(i.absPath.c_wstr());
        auto fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1)
        {
            QStringList fonts = QFontDatabase::applicationFontFamilies(fontId);
            foreach(QString font, fonts)
            {
                ConsoleDebug::printf_endl("Loaded font:%s", font.toStdString().c_str());
            }
            QString fontFamily = fonts.at(0);
            QFont defaultFont = a.font();
            defaultFont.setFamily(fontFamily);
            a.setFont(defaultFont);
        }
        else
        {
            ConsoleDebug::printf_endl("Failed to load the font: %s", fontPath.toStdString().c_str());
        }
    }

    //�Զ��������
    CustomTitleBar titleBar;
    _titleBar = &titleBar;
    titleBar.bEnableCustomClose = true;
    titleBar.SetChildWidget(&w);
    w._customTitleBar = &titleBar;
    LoadEditorWindowSetting(&titleBar, "MainWindow");
    titleBar.show();

    //ˢ��һ����ʽ
    titleBar.setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

    return a.exec();
}
