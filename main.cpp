#include <QtGui>
#include <QtCore>
#include <QTextCodec>

#include "maingui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));

    MainGui *p_win = new MainGui;
    p_win->show();

//    MainGui win;
//    win.show();

    return (app.exec());
}
