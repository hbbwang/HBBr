#include "EditorMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EditorMain w;

    w.show();
    w.resize(1024, 768);

    return a.exec();
}
