#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
    class MainWindow;
}

struct impl;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QFileDialog fileDialog;

    Ui::MainWindow* GetUi()
    {
        return this->ui;
    }

private:
    Ui::MainWindow* ui;
    impl* pimpl;
};

#endif // MAINWINDOW_H
