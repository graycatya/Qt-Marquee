#include "marquee.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Marquee w;
    w.setGeometry(100,100, 600, 50);
    w.AddText("nokia nokia nokia nokia nokia nokia nokia nokia nokia nokia nokia nokia");
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setBold(true);
    font.setPointSize(50);
    w.setFont(font);
    w.SetSpeed(Marquee::Quick);
    w.Start(0);
    w.show();

    return a.exec();
}
