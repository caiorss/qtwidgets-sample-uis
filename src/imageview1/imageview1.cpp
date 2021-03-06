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


template<typename Clickable, typename Callable>
void OnClick(Clickable* object, Callable action)
{
    QObject::connect(object, &Clickable::clicked, action);
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

    QLabel currentFile;

    QLabel ImagePanel;
    ImagePanel.setScaledContents(false);
    // ImagePanel.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ImagePanel.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    ImagePanel.setWindowTitle("Image Panel");

    QObject::connect(tree.selectionModel(), &QItemSelectionModel::selectionChanged, [&]{
       auto index = tree.selectionModel()->currentIndex();
       auto filePath = model.filePath(index);
       std::cout << "Selection changed => File = " << index.data() << std::endl;
       std::cout << "Full path to file = " << filePath << std::endl;
       currentFile.setText(filePath);
       QPixmap pm(filePath); // Open image
       // Scale image to fit in the label
       if(!pm.isNull())
        ImagePanel.setPixmap(pm.scaled(ImagePanel.width(), ImagePanel.height(), Qt::KeepAspectRatio));
    });


    QPushButton btnSelectDir("Open");
    QPushButton btnClose("Close");

    // QObject::connect(&btnClose, &QPushButton::clicked, []{ std::exit(0); });
    OnClick(&btnClose, []{
       std::cout << " [INFO] Exiting application OK." << std::endl;
       std::exit(0);
    });

    QObject::connect(&btnSelectDir, &QPushButton::clicked, [&](){
       QString dir = QFileDialog::getExistingDirectory(nullptr, ("Open Directory"), ".",
                                                       QFileDialog::ShowDirsOnly
                                                       | QFileDialog::DontResolveSymlinks
                                                       );
       std::cout << "Selected directory = " << dir << std::endl;
       tree.setRootIndex(model.index(dir));
    });


    QHBoxLayout buttonPanel;
    buttonPanel.addWidget(&btnSelectDir);
    buttonPanel.addWidget(&btnClose);

    QHBoxLayout hbox;
    hbox.addWidget(&tree);
    hbox.addWidget(&ImagePanel);

    QVBoxLayout vbox;
    vbox.addWidget(&currentFile);
    vbox.addLayout(&buttonPanel);
    vbox.addLayout(&hbox);

    QMainWindow window;
    window.setLayout(&hbox);
    window.setCentralWidget(new QWidget);
    window.centralWidget()->setLayout(&vbox);
    window.setMinimumHeight(500);
    window.setMinimumWidth(650);
    window.resize(QDesktopWidget().availableGeometry(&window).size() * 0.7);
    window.showNormal();

    return app.exec();
}
