#include <iostream>

#include <QtWidgets>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QTreeView>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLabel>

/** Makes QString printable */
auto operator<<(std::ostream& os, QString const& str) -> std::ostream&
{
    return os << str.toStdString();
}

/** Makes QVariant printable */
auto operator<<(std::ostream& os, QVariant const& var) -> std::ostream&
{
    if(var.type() == QMetaType::QString)
        return os << " QVariant{ type = QString, value = " << var.toString() << " }";
    if(var.type() == QMetaType::Bool)
        return os << " QVariant{ type = Bool, value = " << var.toBool() << " }";
    if(var.type() == QMetaType::QChar)
        return os << " QVariant{ type = Char, value = " << QString(var.toChar()).toStdString() << " }";
    return os << " QVariant{ type = Char, value = " << "<UNKNOWN>" << " }";
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFileSystemModel model;
    QStringList filters;

    // View only image files
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp" << "*.tiff";
    model.setNameFilters(filters);
    model.setRootPath("/home/archbox");

    QTreeView tree;
    tree.setModel(&model);
    tree.setRootIndex(model.index("/home/archbox"));

    // Demonstrating look and feel features
    tree.setAnimated(false);
    tree.setIndentation(20);
    tree.setSortingEnabled(true);
    const QSize availableSize = QApplication::desktop()->availableGeometry(&tree).size();
    tree.resize(availableSize / 2);
    tree.setColumnWidth(0, tree.width() / 3);

    tree.setWindowTitle(QObject::tr("Dir View"));

    QLabel ImagePanel;
    ImagePanel.setScaledContents(false);
    ImagePanel.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    ImagePanel.setWindowTitle("Image Panel");
    ImagePanel.frameSize().setHeight(400);
    ImagePanel.frameSize().setWidth(500);
    ImagePanel.setFixedHeight(500);
    ImagePanel.setFixedWidth(600);


    // auto& geom = ImagePanel.;
    // geom.setHeight(400);
    // ImagePanel.geometry().setWidth(500);

    QObject::connect(tree.selectionModel(), &QItemSelectionModel::selectionChanged, [&]{
       auto index = tree.selectionModel()->currentIndex();
       auto filePath = model.filePath(index);
       std::cout << "Selection changed => File = " << index.data() << std::endl;
       std::cout << "Full path to file = " << filePath << std::endl;

       QPixmap pm(filePath); // Open image
       // Scale image to fit in the label
       ImagePanel.setPixmap(pm.scaled(ImagePanel.width(), ImagePanel.height(), Qt::KeepAspectRatio));
    });



    tree.show();
    ImagePanel.showNormal();

    return app.exec();
}
