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

bool _bLeftButtonPress_ForResize = false;

class MyEventFilter : public QObject
{
protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        bool bChangeFocus = false;
        if (event->type() == QEvent::MouseButtonPress) 
        {
            //if (mouseEvent->button() & Qt::MouseButton::LeftButton)
            //{
            //    _bLeftButtonPress_ForResize = true;
            //}
            QWidget* newCurrentFocusWidget = QApplication::widgetAt(mouseEvent->globalPos());
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
       /* else if (event->type() == QEvent::MouseButtonRelease)
        {
            if (mouseEvent->button() & Qt::MouseButton::LeftButton)
            {
                if (_bLeftButtonPress_ForResize)
                {
                    for (auto& i : VulkanApp::GetForms())
                    {
                        i->bStopRender = false;
                    }
                }
                _bLeftButtonPress_ForResize = false;
            }      
        }*/
        //��ֹ������ֻ�н����������ŵ�ʱ��Ż�ָ���Ⱦ
        //else if (event->type() == QEvent::Resize)
        //{
        //    static QTimer* revert;
        //    if (revert == nullptr)
        //    {
        //        revert = new QTimer(this);
        //    }
        //    else
        //    {
        //        revert->stop();
        //    }
        //    revert->setSingleShot(true);
        //    revert->setInterval(5000);//5����û���κ�������Ϊ�����ж�Ϊ����
        //    connect(revert, &QTimer::timeout, this, []() 
        //        {
        //            if (_bLeftButtonPress_ForResize)
        //            {
        //                for (auto& i : VulkanApp::GetForms())
        //                {
        //                    i->bStopRender = false;
        //                }
        //                ConsoleDebug::print_endl("Resize widget focus end.");
        //            }
        //            _bLeftButtonPress_ForResize = false;
        //        });
        //    revert->start();

        //    if (_bLeftButtonPress_ForResize)
        //    {
        //        for (auto& i : VulkanApp::GetForms())
        //        {
        //            i->bStopRender = true;
        //        }
        //    }
        //}
        //if (event->type() == QEvent::MouseMove)
        //    qDebug() << QApplication::widgetAt(mouseEvent->globalPos())->objectName().toStdString().c_str();
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

    //ˢ��һ����ʽ
    w.setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

    int width = 1024, height = 768;
    GetEditorInternationalizationInt("MainWindow", "WindowWidth", width);
    GetEditorInternationalizationInt("MainWindow", "WindowHeight", height);
    w.resize(width, height);
    //SetWindowCenterPos(&w);

    //��ʾ����
    w.show();

    return a.exec();
}
