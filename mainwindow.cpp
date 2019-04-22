#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include <QMessageBox>
#include <QStringListModel>

#include <iostream>

#if 1
auto operator<<(std::ostream& os, QString const& str) -> std::ostream&
{
    return os << str.toStdString();
}
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ------------------------------//

   // ui->ImageViewer1->setPixmap(pm);
    ui->ImageViewer1->setScaledContents(true);

    ui->textEntry1->setText("Text Entry1");

    static auto model = new QStringListModel(this);
    // static QStringList list;
    // model->setStringList(list);
    ui->ItemList1->setModel(model);

    QObject::connect(ui->textEntry1, &QLineEdit::returnPressed, [&]{
        std::cout << "User typed = "
                  << ui->textEntry1->text()
                  << std::endl;
        auto list = model->stringList();
        list << ui->textEntry1->text();
        model->setStringList(list);
        // list << ui->textEntry1->text();
        // QMessageBox::about(this, "About This program", "A dummy program");
    });


    // Create a file selection dialog when the button is clicked
    QObject::connect(ui->pushButton, &QPushButton::clicked, [this](){
       QString file = QFileDialog::getOpenFileName(this, tr("Select a picture"));
       std::cout << "I was clicked"  << std::endl;
       QPixmap pm(file);
       ui->ImageViewer1->setPixmap(pm);

       model->setStringList({});
    });

}


MainWindow::~MainWindow()
{
    delete ui;
}
