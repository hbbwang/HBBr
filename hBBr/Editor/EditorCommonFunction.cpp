#include "EditorCommonFunction.h"
#include "qdir.h"
#include "qmessageBox.h"
#include "QTextStream.h"
#include <QFileInfo>
#include <QDirIterator>
#include "ImageTool.h"
#include "ConsoleDebug.h"
#include "qapplication.h"
#include "FileSystem.h"
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
			while (!in.atEnd())
			{
				strLine = in.readLine();//���ж�ȡ
				int length = objectName.length();
				QString objN = "#" + objectName;
				if (bFound == true && !result.isEmpty() && strLine.contains("};", Qt::CaseSensitive))//�Ƿ񵽵׽���λ��
				{
					result += strLine.left(strLine.length() - 1);
					break;
				}
				if (bFound == false && strcmp(strLine.left(length + 1).toStdString().c_str(), objN.toStdString().c_str()) == 0)
				{
					bFound = true;
				}
				if (bFound)
				{
					result += strLine;
				}
			}
		}
		else
		{
			QMessageBox::information(NULL, "Error", "Load [" + objectName + "] style sheet file failed");
		}
		styleFile.close();
	}
	//QMessageBox::information(this, "Check", result);
	return result;
}

bool DeleteAllFile(QString path)
{
    QFileInfo checkInfo(path);
    if (checkInfo.isFile())
    {
        QFile::remove(path);
        return true;
    }
    QDir dir(path);
    QFileInfoList fileList;
    QFileInfo curFile;
    if (!dir.exists()) { return false; }//�ļ����棬�򷵻�false
    fileList = dir.entryInfoList(QDir::Dirs | QDir::Files
        | QDir::Readable | QDir::Writable
        | QDir::Hidden | QDir::NoDotAndDotDot
        , QDir::Name);
    while (fileList.size() > 0)
    {
        int infoNum = fileList.size();
        for (int i = infoNum - 1; i >= 0; i--)
        {
            curFile = fileList[i];
            if (curFile.isFile())//������ļ���ɾ���ļ�
            {
                QFile fileTemp(curFile.filePath());
                fileTemp.remove();
                fileList.removeAt(i);
            }
            if (curFile.isDir())//������ļ���
            {
                QDir dirTemp(curFile.filePath());
                QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
                    | QDir::Readable | QDir::Writable
                    | QDir::Hidden | QDir::NoDotAndDotDot
                    , QDir::Name);
                if (fileList1.size() == 0)//�²�û���ļ����ļ���
                {
                    dirTemp.rmdir(".");
                    fileList.removeAt(i);
                }
                else//�²����ļ��л��ļ�
                {
                    for (int j = 0; j < fileList1.size(); j++)
                    {
                        if (!(fileList.contains(fileList1[j])))
                            fileList.append(fileList1[j]);
                    }
                }
            }
        }
    }
    dir.removeRecursively();
    return true;
}

bool SearchDir(QString path, QString searchText, QList<SFileSearch>& ResultOutput)
{
    //path�����Ǹ�Ŀ¼
    QFileInfo checkInfo(path);
    if (!checkInfo.isDir())
    {
        return false;
    }

    QDir dir(path);
    QFileInfoList fileList;
    if (!dir.exists()) { return false; }//�ļ����棬�򷵻�false
    fileList = dir.entryInfoList(
        QDir::Dirs | QDir::Readable | QDir::Writable| QDir::Hidden | QDir::NoDotAndDotDot
        , QDir::Name);
    //
    bool bChildFind = false;
    bool result = false;
    for (auto i : fileList)
    {
        QFileInfo curFile = i;
        if (curFile.isDir())//����Ϊ�ļ���
        {
            QDir dirTemp(curFile.filePath());
            QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
                , QDir::Name);

            if (fileList1.size() > 0)//�²����ļ��У�����ѭ��
            {
                bChildFind = SearchDir(curFile.filePath(), searchText, ResultOutput);
            }

            QString baseName = curFile.baseName();
            //����û�ҵ������������Ŀ¼�����з��ϵ��ļ��У���Ҳ�Ѹ�Ŀ¼һ��ӽ�ȥ
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
    if (!dir.exists()) { return false; }//�ļ����棬�򷵻�false
    fileList = dir.entryInfoList(
        QDir::AllEntries | QDir::Files | QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
        , QDir::Name);
   
    bool bChildFind = false;
    for (auto i : fileList)
    {
        QFileInfo curFile = i;
        QString baseName = curFile.baseName();
        if (curFile.isFile())//���ļ���ֱ�ӽ��жԱ�
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
        else if (curFile.isDir())//���ļ���
        {
            QDir dirTemp(curFile.filePath());
            QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::AllEntries | QDir::Files | QDir::Dirs | QDir::Readable | QDir::Writable | QDir::Hidden | QDir::NoDotAndDotDot
                , QDir::Name);

            if (fileList1.size() > 0)//�²����ļ��У�����ѭ��
            {
                bChildFind = SearchFile(curFile.filePath(), searchText, ResultOutput);
            }
            //����û�ҵ������������Ŀ¼�����з��ϵ��ļ��У���Ҳ�Ѹ�Ŀ¼һ��ӽ�ȥ
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
    if (tgaImage == NULL)
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
 //   QString exePath = QDir::toNativeSeparators(HString::GetExePathWithoutFileName().c_str());
 //   ContentPath.remove(exePath);

 //   //savedPath += "\\Saved\\" + info.
	//QString previewImagePath = QString(info.path() + QDir::separator() + info.baseName() + "." + info.suffix() + "@Preview.jpg");   
 //   QFileInfo previewImage(previewImagePath);
 //   if (!previewImage.exists())
 //   {
 //       if (//info.suffix().compare("dds", Qt::CaseInsensitive) == 0||
 //           info.suffix().compare("tex", Qt::CaseInsensitive) == 0)
 //       {
 //           FileStreamClass::DecompressionImage2D(resourceFilePath.toStdString().c_str(), previewImagePath.toStdString().c_str(), NULL, 64, 64);
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