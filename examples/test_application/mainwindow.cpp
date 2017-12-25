#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPrinter>
#include <QPainter>
#include <qcommonprintdialog.h>
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QCommonPrintDialog *dialog = new QCommonPrintDialog();
    if(dialog->exec() == QDialog::Rejected){
        qDebug("Printing cancelled");
        return;
    }

    qDebug("Printing successful");

    return;
}
