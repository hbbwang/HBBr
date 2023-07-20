#include "EditorMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EditorMain w;
    w.show();
    return a.exec();
}
