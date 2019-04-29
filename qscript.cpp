#include <iostream>
#include <functional>
#include <cmath>

#include <QApplication>
#include <QDebug>
#include <QtScript/QScriptEngine>



// QScript Native Function
// See: https://het.as.utexas.edu/HET/Software/PyQt/qscriptengine.html
QScriptValue hypothenuse(QScriptContext *context, QScriptEngine *engine)
{
   QScriptValue qx = context->argument(0);
   QScriptValue qy = context->argument(1);
   double x = qx.toNumber();
   double y = qy.toNumber();
   std::cout << " [TRACE] x = " << x << " ; y = "  << y << std::endl;
   return std::sqrt(x * x + y * y);
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QScriptEngine engine;
    QScriptValue  result;
    QString       code;

    code = " 3.0 * Math.exp(-2.0) + Math.sqrt(5.0 * 8.0 / 100.0)";
    result = engine.evaluate(code);
    qDebug() << " Expr = " << code;
    qDebug() << " result = " << result.toNumber();
    qDebug() << "\n";

    code = " Math.sqrt(8.0 * 8.0 + 6.0 * 6.0)";
    result = engine.evaluate(code);
    qDebug() << " Expr = " << code;
    qDebug() << " result = " << result.toNumber();
    qDebug() << "";

    // Javascript Function
    code = "function(a, b){ return a * a + b * b; }";
    QScriptValue funct = engine.evaluate(code);
    QScriptValueList xs;
    xs << 3.0 << 4.0;
    result = funct.call(QScriptValue(), xs);
    qDebug() << " Function call result = " << result.toString();

    // ======== Set Scripting glabal variables =========//
    engine.globalObject().setProperty("PI", 3.1415);
    result = engine.evaluate("3.0 * PI - 5.0");
    qDebug() << "Result => 3.0 * PI - 5.0 = " << result.toNumber();


    //==== Expose Native Function to Script Code =============//
    QScriptValue fun = engine.newFunction(hypothenuse);
    engine.globalObject().setProperty("hypothenuse", fun);
    result = engine.evaluate("hypothenuse(6.0, 8.0)");
    qDebug() << "\n hypothenuse(6, 8) = " << result.toNumber();

    return app.exec();

}
