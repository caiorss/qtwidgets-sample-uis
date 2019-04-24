#include <iostream>
#include <functional>

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

template<typename Selectable, typename Callable>
void OnSelectionChange(Selectable* object, Callable&& action)
{
        QObject::connect( object->selectionModel(),
                          &QItemSelectionModel::selectionChanged,
                          action
                         );
}


class ImageViewer: public QMainWindow
{
private:
    QFileSystemModel*  model         = new QFileSystemModel;
    QTreeView*         tree          = new QTreeView;
    QPushButton*       btnSelectDir  = new QPushButton("Open");
    QPushButton*       btnClose      = new QPushButton("Close");
    QPushButton*       btnAbout      = new QPushButton("About");

    QLabel*            currentFile   = new QLabel;
    QLabel*            ImagePanel    = new QLabel;

    QMenu*             fileMenu      = new QMenu(this);
    QSystemTrayIcon*   trayIcon      = new QSystemTrayIcon(this);
    // QString            root_path     = "/";

    void SetLayout()
    {
        // View only image files
        QStringList  filters;
        filters << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp" << "*.tiff";
        model->setNameFilters(filters);
        tree->setModel(model);

        currentFile->setBackgroundRole(QPalette::Base);

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
        buttonPanel->addWidget(btnAbout);
        buttonPanel->addWidget(btnClose);

        auto hbox = new QHBoxLayout ;
        hbox->addWidget(tree);
        hbox->addWidget(ImagePanel);

        auto vbox = new QVBoxLayout;
        vbox->addWidget(currentFile);
        vbox->addLayout(buttonPanel);
        vbox->addLayout(hbox);

        // this->setLayout(hbox);
        this->setCentralWidget(new QWidget);
        this->centralWidget()->setLayout(vbox);
        this->setMinimumHeight(500);
        this->setMinimumWidth(650);
       // this->resize(QDesktopWidget().availableGeometry(window).size() * 0.7);
        // this->showNormal();

        //=========== Set Menus ============//

        auto openAct = new QAction(tr("&Open..."), this);
        openAct->setShortcut(tr("Ctrl+O"));
        fileMenu->addAction(openAct);

        QMenuBar* bar = new QMenuBar;
        vbox->setMenuBar(bar);
        bar->addMenu(fileMenu);
        bar->setVisible(true);
        bar->setNativeMenuBar(true);
        bar->show();
    }

    void SetMenus()
    {
       //QCoreApplication::​setAttribute(Qt::AA_DontUseNativeMenuBar);

       QMenuBar* bar = this->menuBar();
       bar->setNativeMenuBar(false);

       auto openAct = new QAction(tr("&Open..."), this);
       openAct->setShortcut(tr("Ctrl+O"));


       QObject::connect(openAct, &QAction::triggered, [&]{
          std::cout << " [INFO] Menu clicked OK." << std::endl;
       });

       QObject::connect(openAct, &QAction::triggered,
                        std::bind(&ImageViewer::OpenDirectory, this));

       // connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
       fileMenu->addAction(openAct);
       this->menuBar()->addMenu(fileMenu);
       bar->show();
       // bar->addAction(fileMenu->menuAction());
    }

    void SetEvents()
    {
        OnSelectionChange(tree, [&]{
           auto file = this->GetSelectedFile();
           std::cout << " [INFO] Display image = " << file << std::endl;
           this->DisplayImage(file);
        });

        // QObject::connect(&btnClose, &QPushButton::clicked, []{ std::exit(0); });
        OnClick(btnClose, []{
           std::cout << " [INFO] Exiting application OK." << std::endl;
           std::exit(0);
        });

        OnClick(btnSelectDir, std::bind(&ImageViewer::OpenDirectory, this));
        OnClick(btnAbout, std::bind(&ImageViewer::about, this));
    }

public:

    explicit ImageViewer(QString path = QDir::homePath())
    {
        this->setWindowTitle("Sample QT5 Image Viewer");

        // Make this Window always on Top
        this->setWindowFlags(Qt::WindowStaysOnTopHint);

        // this->SetMenus();
        // Set up user interface layout
        this->SetLayout();
        // Install events (signlas and slots)
        this->SetEvents();
        this->SetRootDirectory(path);

        // ====== Tray Icon =========//

        auto openAct = new QAction(tr("&Open..."), this);
        openAct->setShortcut(tr("Ctrl+O"));
        auto trayMenu = new QMenu(this);
        trayMenu->addAction(openAct);
        trayIcon->setContextMenu(trayMenu);
        trayIcon->setToolTip("Image Viewer Application Control");

        // Load Icon added by resource file imageView2.qrc
        // that points to the image /images/appicon.png
        static QIcon appIcon = QIcon(":/icons/appicon.png");
        // QIcon appIcon = QIcon("/home/archbox/projects/qtwidgets/images/appicon.png");
        this->setWindowIcon(appIcon);
        trayMenu->setIcon(appIcon);
        //trayMenu->add
        this->trayIcon->show();
    }

    ImageViewer&
    SetRootDirectory(QString path)
    {
        model->setRootPath(path);
        tree->setRootIndex(model->index(path));
        return *this;
    }

    void OpenDirectory()
    {
        QString dir = QFileDialog::getExistingDirectory(
                          this, "Open Directory", ".",
                        QFileDialog::ShowDirsOnly
                      | QFileDialog::DontResolveSymlinks
                    );
        this->SetRootDirectory(dir);
    }

    void DisplayImage(QString file)
    {
        currentFile->setText(file);
        QPixmap pm(file); // Open image
        // Scale image to fit in the label
        if(!pm.isNull())
         ImagePanel->setPixmap(pm.scaled(
                                  ImagePanel->width(),
                                  ImagePanel->height(),
                                  Qt::KeepAspectRatio));
    }

    QString GetSelectedFile() const
    {
        QModelIndex index = tree->selectionModel()->currentIndex();
        return  model->filePath(index);
    }

    QModelIndex GetSelectedItem() const
    {
        return tree->selectionModel()->currentIndex();
    }

    void about()
    {
        QMessageBox::about(this, tr("About this Application"),
                tr("<p> <b>Image Viewer</b> is a sample QT5 User Iterface created "
                   "without MOC - Meta Object Compiler. It shows how to take advtange "
                   "of the modern C++ features for building a minimal QT Widget image viewer application"
                   ));
    }

};


int main(int argc, char *argv[])
{    
    QApplication app(argc, argv);

    qDebug() << " [INFO] Application path = " << QCoreApplication::applicationFilePath();
    qDebug() << " [INFO] Application PID  = " << QCoreApplication::applicationPid();
    // QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    ImageViewer imageViewer;
    imageViewer.showNormal();


    return app.exec();
}
