#include "qcommonprintdialog.h"
#include "ui_qcommonprintdialog.h"

QCommonPrintDialog::QCommonPrintDialog(QWidget *parent) :
    QMainWindow (parent),
    ui(new Ui::QCommonPrintDialog)
{
    ui->setupUi(this);
}

QCommonPrintDialog::~QCommonPrintDialog()
{
    delete ui;
}
