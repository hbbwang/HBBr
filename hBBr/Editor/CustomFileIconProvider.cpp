#include "CustomFileIconProvider.h"
#include "EditorCommonFunction.h"
#include "HString.h"
#include "QIcon.h"
#include "FileSystem.h"
QIcon CustomFileIconProvider::icon(const QFileInfo& info) const
{
    if (_scope == IconProviderScope::ContentBrowserTreeView)
    {
        if (info.isDir())
        {
            return QIcon("null.png");
        }
    }
    else if (_scope == IconProviderScope::ContentBrowserListView)
    {
        QPixmap map;
        bool bSucceed = GetPreviewImage(info.filePath(), map);
        HString pp = FileSystem::GetProgramPath();
        if (info.isDir())
        {
            pp += "Config/Theme/Icons/ICON_DIR.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
        else if (
            info.suffix().compare("fx", Qt::CaseInsensitive)==0||
            info.suffix().compare("hlsl", Qt::CaseInsensitive)==0
            )
        {
            pp += "Config/Theme/Icons/ICON_SHADER_FILE.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
        else if (info.suffix().compare("mat", Qt::CaseInsensitive) == 0 )
        {
            pp += "Config/Theme/Icons/ICON_FILE_MAT.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
        else if (info.suffix().compare("xml", Qt::CaseInsensitive) == 0)
        {
            pp += "Config/Theme/Icons/ICON_FILE_XML.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
        else if (info.suffix().compare("ini", Qt::CaseInsensitive) == 0)
        {
            pp += "Config/Theme/Icons/ICON_FILE_INI.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
        else if (
            info.suffix().compare("tex", Qt::CaseInsensitive) == 0
            //||info.suffix().compare("dds", Qt::CaseInsensitive) == 0
            )
        {
            if (bSucceed)
            {
                map = map.scaled(512, 512, Qt::IgnoreAspectRatio);
                return QIcon(map);
            }
            else
            {
                pp += "Config/Theme/Icons/ICON_FILE.png";
                FileSystem::CorrectionPath(pp);
                return QIcon(pp.c_str());
            }
        }
        else
        {
            pp += "Config/Theme/Icons/ICON_FILE.png";
            FileSystem::CorrectionPath(pp);
            return QIcon(pp.c_str());
        }
    }
    return QFileIconProvider::icon(info);
}