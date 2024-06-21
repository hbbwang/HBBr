#include "EditorCommonFunction.h"
#include "qdir.h"
#include "qmessageBox.h"
#include "QTextStream.h"
#include <QFileInfo>
#include <regex>
#include <QDirIterator>
#include "ImageTool.h"
#include "ConsoleDebug.h"
#include "qapplication.h"
#include "FileSystem.h"
#include "Serializable.h"
#include<QRegularExpression>
QString GetWidgetStyleSheetFromFile(QString objectName, QString path)
{
	QString result;
	bool bFound = false;
    path =QString(FileSystem::GetProgramPath().c_str()) + path;
	if (!path.isEmpty())
	{
		QDir dir(path);
		dir.toNativeSeparators(path);
		path = dir.path();

		QFile styleFile(path);

		if (styleFile.open(QFile::ReadOnly | QIODevice::Text))
		{
			QTextStream in(&styleFile);
			QString strLine;
            bool bFoundVariable = false;
            struct qssVarible
            {
                QString name;
                QString value;
            };
            QList<qssVarible> vars;
			while (!in.atEnd())
			{
				strLine = in.readLine();//逐行读取
				int length = objectName.length();
				QString objN = "#" + objectName;
				if (bFound == true && !result.isEmpty() && strLine.contains("};", Qt::CaseSensitive))//是否到底结束位置
				{
					result += strLine.left(strLine.length() - 1);
					break;
				}
				if (bFound == false && strcmp(strLine.left(length + 1).toStdString().c_str(), objN.toStdString().c_str()) == 0)
				{
					bFound = true;
				}
                //替换变量定义
                if (strLine.contains("VariableDefine", Qt::CaseSensitive))
                {
                    bFoundVariable = true;
                    strLine = "/*" + strLine;
                }
                if (bFoundVariable)
                {
                    auto l = strLine.split(':');
                    if (l.size() > 1)
                    {
                        QString name = l[0].remove(" ");
                        name = name.remove("\r");
                        name = name.remove("\t");
                        QString value = l[1].remove(';');
                        vars.append({ name , value });
                    }
                }
                if (bFoundVariable && strLine.contains("}", Qt::CaseSensitive))
                {
                    bFoundVariable = false;
                    strLine = strLine + "*/";
                }
				if (bFound)
				{
					result += strLine;
				}
			}
            styleFile.close();
            for (auto i : vars)
            {
                QString re = "\\b" + i.name + "\\b";
                auto reg = QRegularExpression(re);
                result.replace(reg, i.value);
            }
		}
		else
		{
			QMessageBox::information(nullptr, "Error", "Load [" + objectName + "] style sheet file failed");
		}
	}
	//QMessageBox::information(this, "Check", result);
    //ConsoleDebug::print_endl(result.toStdString());
	return result;
}

QString GetSingleStyleFromFile(QString Name, QString path)
{
    QString result;
    bool bFound = false;
    path = QString(FileSystem::GetProgramPath().c_str()) + path;
    if (!path.isEmpty())
    {
        QDir dir(path);
        dir.toNativeSeparators(path);
        path = dir.path();

        QFile styleFile(path);

        if (styleFile.open(QFile::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&styleFile);
            QString strLine;
            bool bFoundVariable = false;
            struct qssVarible
            {
                QString name;
                QString value;
            };
            QList<qssVarible> vars;
            while (!in.atEnd())
            {
                strLine = in.readLine();//逐行读取
                int length = Name.length();
                QString objN = "*" + Name;
                if (bFound == true && !result.isEmpty() && strLine.contains("};", Qt::CaseSensitive))//是否到底结束位置
                {
                    result += strLine.left(strLine.length() - 1);
                    break;
                }
                if (bFound == false && strcmp(strLine.left(length + 1).toStdString().c_str(), objN.toStdString().c_str()) == 0)
                {
                    bFound = true;
                    continue;
                }
                //替换变量定义
                if (strLine.contains("VariableDefine", Qt::CaseSensitive))
                {
                    bFoundVariable = true;
                    strLine = "/*" + strLine;
                }
                if (bFoundVariable)
                {
                    auto noSpace = strLine.remove(" ");
                    noSpace = noSpace.remove("\t");
                    auto l = noSpace.split(':');
                    if (l.size() > 1)
                    {
                        vars.append({ l[0] , l[1].remove(';') });
                    }
                }
                if (bFoundVariable && strLine.contains("}", Qt::CaseSensitive))
                {
                    bFoundVariable = false;
                    strLine = strLine + "*/";
                }
                if (bFound)
                {
                    result += strLine;
                }
            }
            styleFile.close();
            for (auto i : vars)
            {
                result.replace(i.name, i.value);
            }
        }
        else
        {
            QMessageBox::information(nullptr, "Error", "Load [" + Name + "] style sheet file failed");
        }
    }
    //QMessageBox::information(this, "Check", result);
    result = result.replace("{","");
    result = result.replace("}", "");
    return result;
}

/*--------------------渲染器相关----------------------*/
#include "Asset/ContentManager.h"
#include "Asset/HGuid.h"
//
//bool DeleteAllFile(QString path, QList<QString>*allAssetGuid)
//{
//    QFileInfo checkInfo(path);
//    if (checkInfo.isFile())
//    {
//        ContentManager::Get()->DeleteAsset(path.toStdString().c_str());
//        if(allAssetGuid)
//            allAssetGuid->append(checkInfo.baseName());
//        QFile::remove(path);
//        return true;
//    }
//    QDir dir(path);
//    QFileInfoList fileList;
//    QFileInfo curFile;
//    if (!dir.exists()) { return false; }//文件不存，则返回false
//    fileList = dir.entryInfoList(QDir::Dirs | QDir::Files
//        | QDir::Readable | QDir::Writable
//        | QDir::Hidden | QDir::NoDotAndDotDot
//        , QDir::Name);
//    while (fileList.size() > 0)
//    {
//        int infoNum = fileList.size();
//        for (int i = infoNum - 1; i >= 0; i--)
//        {
//            curFile = fileList[i];
//            if (curFile.isFile())//如果是文件，删除文件
//            {
//                ContentManager::Get()->DeleteAsset(curFile.filePath().toStdString().c_str());
//                QFile fileTemp(curFile.filePath());
//                if (allAssetGuid)
//                    allAssetGuid->append(curFile.baseName());
//                fileTemp.remove();
//                fileList.removeAt(i);
//            }
//            if (curFile.isDir())//如果是文件夹
//            {
//                QDir dirTemp(curFile.filePath());
//                QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
//                    | QDir::Readable | QDir::Writable
//                    | QDir::Hidden | QDir::NoDotAndDotDot
//                    , QDir::Name);
//                if (fileList1.size() == 0)//下层没有文件或文件夹
//                {
//                    dirTemp.rmdir(".");
//                    fileList.removeAt(i);
//                }
//                else//下层有文件夹或文件
//                {
//                    for (int j = 0; j < fileList1.size(); j++)
//                    {
//                        if (!(fileList.contains(fileList1[j])))
//                            fileList.append(fileList1[j]);
//                    }
//                }
//            }
//        }
//    }
//    dir.removeRecursively();
//    return true;
//}

bool SearchDir(QString path, QString searchText, QList<SFileSearch>& ResultOutput)
{
    //path必须是个目录
    QFileInfo checkInfo(path);
    if (!checkInfo.isDir())
    {
        return false;
    }

    QDir dir(path);
    QFileInfoList fileList;
    if (!dir.exists()) { return false; }//文件不存，则返回false
    fileList = dir.entryInfoList(
        QDir::Dirs | QDir::Readable | QDir::Writable| QDir::Hidden | QDir::NoDotAndDotDot
        , QDir::Name);
    //
    bool bChildFind = false;
    bool result = false;
    for (auto i : fileList)
    {
        QFileInfo curFile = i;
        if (curFile.isDir())//必须为文件夹
        {
            QDir dirTemp(curFile.filePath());
            QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
                , QDir::Name);

            if (fileList1.size() > 0)//下层有文件夹，继续循环
            {
                bChildFind = SearchDir(curFile.filePath(), searchText, ResultOutput);
            }

            QString baseName = curFile.baseName();
            //就算没找到，但是如果子目录里面有符合的文件夹，那也把父目录一起加进去
            if (bChildFind || baseName.contains(QString::fromLocal8Bit(searchText.toStdString().c_str()), Qt::CaseInsensitive))
            {
                SFileSearch newSearch;
                newSearch.bLastFile = !bChildFind;
                newSearch.path = curFile.filePath();
                newSearch.fileInfo = curFile;
                ResultOutput.append(newSearch);
                result = true;
            }
        }
    }
    //
    return result;
}

bool SearchFile(QString path, QString searchText, QList<SFileSearch>& ResultOutput)
{
    bool result = false;

    QDir dir(path);
    QFileInfoList fileList;
    if (!dir.exists()) { return false; }//文件不存，则返回false
    fileList = dir.entryInfoList(
        QDir::AllEntries | QDir::Files | QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
        , QDir::Name);
   
    bool bChildFind = false;
    for (auto i : fileList)
    {
        QFileInfo curFile = i;
        QString baseName = curFile.baseName();
        if (curFile.isFile())//是文件，直接进行对比
        {
            if (baseName.contains(QString::fromLocal8Bit(searchText.toStdString().c_str()), Qt::CaseInsensitive))
            {
                SFileSearch newSearch;
                newSearch.bLastFile = true;
                newSearch.path = curFile.filePath();
                newSearch.fileInfo = curFile;
                ResultOutput.append(newSearch);
                result = true;
            }
        }
        else if (curFile.isDir())//是文件夹
        {
            QDir dirTemp(curFile.filePath());
            QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::AllEntries | QDir::Files | QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
                , QDir::Name);

            if (fileList1.size() > 0)//下层有文件夹，继续循环
            {
                bChildFind = SearchFile(curFile.filePath(), searchText, ResultOutput);
            }
            //就算没找到，但是如果子目录里面有符合的文件夹，那也把父目录一起加进去
            if (bChildFind || baseName.contains(QString::fromLocal8Bit(searchText.toStdString().c_str()), Qt::CaseInsensitive))
            {
                SFileSearch newSearch;
                newSearch.bLastFile = !bChildFind;
                newSearch.path = curFile.filePath();
                newSearch.fileInfo = curFile;
                ResultOutput.append(newSearch);
                result = true;
            }
        }
    }
    return result;
}

QImage GetImageFromTGA(QString path)
{
    ConsoleDebug::print_endl("/// [ ImportImageDataToBuffer ] ///");
    QImage result = QImage();
    QFileInfo info(path);
    if (!info.exists() ||  info.suffix().compare("tga",Qt::CaseInsensitive) != 0)
    {
        return result;
    }

    auto tgaImage = ImageTool::ReadTgaImage(info.filePath().toStdString().c_str());
    if (tgaImage == nullptr)
    {
        return result;
    }
    QImage::Format format;
    switch (tgaImage->data_header.bitsPerPixel)
    {
        case 8: format = QImage::Format_Grayscale8; break;
        case 16: format = QImage::Format_RGB555; break;
        case 24: format = QImage::Format_RGB32; break;
        case 32: format = QImage::Format_ARGB32; break;
        default: return result;
    }

    QImage image(tgaImage->imageData.data(), tgaImage->data_header.width, tgaImage->data_header.height, format);
    image = image.mirrored(0, !(tgaImage->data_header.imageDescriptor & 16));
    delete tgaImage;

    return image;
}

bool GetPreviewImage(QString resourceFilePath, QPixmap& pixmap)
{
 //   QFileInfo info(resourceFilePath);

 //   QString ContentPath = QDir::toNativeSeparators(resourceFilePath);
 //   QString exePath = QDir::toNativeSeparators(FileSystem::GetProgramPath().c_str());
 //   ContentPath.remove(exePath);

 //   //savedPath += "\\Saved\\" + info.
	//QString previewImagePath = QString(info.path() + QDir::separator() + info.baseName() + "." + info.suffix() + "@Preview.jpg");   
 //   QFileInfo previewImage(previewImagePath);
 //   if (!previewImage.exists())
 //   {
 //       if (//info.suffix().compare("dds", Qt::CaseInsensitive) == 0||
 //           info.suffix().compare("tex", Qt::CaseInsensitive) == 0)
 //       {
 //           FileStreamClass::DecompressionImage2D(resourceFilePath.toStdString().c_str(), previewImagePath.toStdString().c_str(), nullptr, 64, 64);
 //       }
 //       //else if (info.suffix().compare("mat", Qt::CaseInsensitive))
 //       //{
 //           //FileStreamClass::DecompressionImage2D(resourceFilePath.toStdString().c_str(), previewImagePath.toStdString().c_str());
 //       //}
 //       else
 //       {
 //           return false;
 //       }
 //   }
 //   pixmap = QPixmap(previewImagePath);
    return true;
}

QString GetEditorInternationalization(QString Group, QString name)
{
    static nlohmann::json json;
    if (json.is_null())
    {
        Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor_localization.json", json);
    }
    if (!json.is_null())
    {
        auto it = json.find(Group.toStdString());
        if (it != json.end())
        {
            auto va_it = it.value().find(name.toStdString());
            if (va_it != it.value().end())
            {
                std::string result = va_it.value();
                return result.c_str();
            }
        }
    }
    return "????";
}

QString GetEditorConfig(QString Group, QString name)
{
    static nlohmann::json json;
    if (json.is_null())
    {
        Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor.json", json);
    }
    if (!json.is_null())
    {
       auto it =  json.find(Group.toStdString());
       if (it != json.end())
       {
           auto va_it = it.value().find(name.toStdString());
           if (va_it != it.value().end())
           {
               std::string result = va_it.value();
               return result.c_str();
           }
       }
    }
    return "????";
}
