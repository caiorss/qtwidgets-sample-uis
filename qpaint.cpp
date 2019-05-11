#include <iostream>
#include <functional>
#include <optional>
#include <map>

#include <QtWidgets>
#include <QPolygon>
#include <QApplication>

class Canvas: public QWidget
{


    void paintEvent(QPaintEvent* event)
    {

        // Graphics context
        QPainter qp{this};
        qp.setRenderHint(QPainter::Antialiasing);
        
        QPen pen(Qt::red, 2, Qt::SolidLine);

        qp.scale(1, -1);
        qp.translate(0, -this->height());

        qp.translate(200.5, 250.0);
        qp.scale(1, -1);
        qp.drawText(QPointF(0, 0), "hello world QT Painter");

        qp.setPen(pen);
        qp.drawLine(QPointF(100.5f, 200.4f), QPointF(90.12f, 56.24f));
        qp.drawRect(50, 180, 90, 140);

        // QPolygon pl(QPoint{10, 25}, QPoint{200, 60}, QPoint{90, 80}, QPoint{260, 80});
        //ctx.drawPolygon(pl);

    }

};


int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);

    Canvas window;
    window.resize(400, 500);
    window.show();

    return qapp.exec();
}
