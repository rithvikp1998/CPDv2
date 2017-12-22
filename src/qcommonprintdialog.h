#ifndef QCOMMONPRINTDIALOG_H
#define QCOMMONPRINTDIALOG_H

#include <QMainWindow>

#include "common-print-dialog_global.h"

namespace Ui {
    class QCommonPrintDialog;
}

class QCommonPrintDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit QCommonPrintDialog(QWidget *parent = 0);
    ~QCommonPrintDialog();

private:
    Ui::QCommonPrintDialog *ui;
};

#endif // QCOMMONPRINTDIALOG_H
