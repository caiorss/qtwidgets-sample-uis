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


class ImageViewer: public QMainWindow
{
private:
    QFileSystemModel*  model         = new QFileSystemModel;
    QTreeView*         tree          = new QTreeView;
    QPushButton*       btnSelectDir  = new QPushButton("Open");
    QPushButton*       btnClose      = new QPushButton("Close");
    QLabel*            currentFile   = new QLabel;
    QLabel*            ImagePanel    = new QLabel;
    // QString            root_path     = "/";

    void SetLayout()
    {
        // View only image files
        QStringList  filters;
        filters << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp" << "*.tiff";
        model->setNameFilters(filters);
        tree->setModel(model);

        // Demonstrating look and feel features
        tree->setAnimated(false);
        tree->setIndentation(20);
        tree->setSortingEnabled(true);
        const QSize availableSize = QApplication::desktop()->availableGeometry(tree).size();
        tree->resize(availableSize / 2);
        tree->setColumnWidth(0, tree->width() / 3);
        tree->setWindowTitle(QObject::tr("Dir View"));

        ImagePanel->setScaledContents(false);
        // ImagePanel.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
        ImagePanel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        ImagePanel->setWindowTitle("Image Panel");

        auto buttonPanel = new QHBoxLayout;
        buttonPanel->addWidget(btnSelectDir);
        buttonPanel->addWidget(btnClose);

        auto hbox = new QHBoxLayout ;
        hbox->addWidget(tree);
        hbox->addWidget(ImagePanel);

        auto vbox = new QVBoxLayout;
        vbox->addWidget(currentFile);
        vbox->addLayout(buttonPanel);
        vbox->addLayout(hbox);

        this->setLayout(hbox);
        this->setCentralWidget(new QWidget);
        this->centralWidget()->setLayout(vbox);
        this->setMinimumHeight(500);
        this->setMinimumWidth(650);
       // this->resize(QDesktopWidget().availableGeometry(window).size() * 0.7);
        // this->showNormal();
    }
    void SetEvents()
    {
        QObject::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, [&]{
           auto index = tree->selectionModel()->currentIndex();
           auto filePath = model->filePath(index);
           std::cout << "Selection changed => File = " << index.data() << std::endl;
           std::cout << "Full path to file = " << filePath << std::endl;
           currentFile->setText(filePath);
           QPixmap pm(filePath); // Open image
           // Scale image to fit in the label
           if(!pm.isNull())
            ImagePanel->setPixmap(pm.scaled(
                                     ImagePanel->width(),
                                     ImagePanel->height(),
                                     Qt::KeepAspectRatio));
        });

        // QObject::connect(&btnClose, &QPushButton::clicked, []{ std::exit(0); });
        OnClick(btnClose, []{
           std::cout << " [INFO] Exiting application OK." << std::endl;
           std::exit(0);
        });

        OnClick(btnSelectDir, [&](){
           QString dir = QFileDialog::getExistingDirectory(nullptr, ("Open Directory"), ".",
                                                           QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks
                                                           );
           std::cout << "Selected directory = " << dir << std::endl;
           tree->setRootIndex(model->index(dir));
        });

    }

public:

    void SetRootDirectory(QString path)
    {
        model->setRootPath(path);
        tree->setRootIndex(model->index(path));
    }

    ImageViewer(QString path = "/")
    {
        this->setWindowTitle("Sample QT5 Image Viewer");
        // Set up user interface layout
        this->SetLayout();
        // Install events (signlas and slots)
        this->SetEvents();
        this->SetRootDirectory(path);
    }
};


int main(int argc, char *argv[])
{
    std::cout << " [INFO] Starting Application OK. " << std::endl;

    QApplication app(argc, argv);

    ImageViewer imageViewer;
    imageViewer.show();


    return app.exec();
}
