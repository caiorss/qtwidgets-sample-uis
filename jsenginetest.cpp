#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QJSEngine>
#include <QDebug>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QJSEngine engine;

    QJSValue out;

    out = engine.evaluate("3.45 * 5.6 / 2.0 - 50.0 * 10.0");
    std::cout << "out = " << out.toNumber() << std::endl;
    qDebug() << "out << " << out.toNumber();

    QJSValue jsfunc = engine.evaluate("function(a, b) { return a * a + b * b; }");
    QJSValueList args;
    args << 3.5 << 6.8;
    out = jsfunc.call(args);
    std::cout << "out = " << out.toNumber() << std::endl;



    return 0;
}
