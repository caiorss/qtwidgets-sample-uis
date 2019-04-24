#include <iostream>
#include <iomanip>
#include <functional>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>

auto LoadForm(QString filePath) -> QWidget*
{
    QUiLoader loader;
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    QWidget* form = loader.load(&file, nullptr);
    file.close();
    return form;
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget* form = LoadForm("/home/archbox/projects/qtwidgets/form1.ui");
    form->show();


    return app.exec();
}
