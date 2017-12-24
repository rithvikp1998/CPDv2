#ifndef QCOMMONPRINTDIALOG_H
#define QCOMMONPRINTDIALOG_H

#include <QMainWindow>
#include <QDialog>

#include "common-print-dialog_global.h"

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(QWidget *parent = 0);
};

class PageSetupTab : public QWidget
{
    Q_OBJECT

public:
    explicit PageSetupTab(QWidget *parent = 0);
};

class OptionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsTab(QWidget *parent = 0);
};

class JobsTab : public QWidget
{
    Q_OBJECT

public:
    explicit JobsTab(QWidget *parent = 0);
};

class QualityTab : public QWidget
{
    Q_OBJECT

public:
    explicit QualityTab(QWidget *parent = 0);
};

class QCommonPrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QCommonPrintDialog(QWidget *parent = 0);
    ~QCommonPrintDialog();

private:
    QTabWidget *tabWidget;
};

#endif // QCOMMONPRINTDIALOG_H
