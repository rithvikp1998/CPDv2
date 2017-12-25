#ifndef QCOMMONPRINTDIALOG_H
#define QCOMMONPRINTDIALOG_H

#include <QMainWindow>
#include <QDialog>
#include <QtWidgets>
#include <QPrintPreviewWidget>

#include "common-print-dialog_global.h"
#include "singleton.h"

struct _FrontendObj;
using FrontendObj = _FrontendObj;

struct _PrinterObj;
using PrinterObj = _PrinterObj;

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    QComboBox *destinationComboBox;
    QCheckBox *remotePrintersCheckBox;
    QComboBox *paperComboBox;
    QComboBox *pagesComboBox;
    QSpinBox *copiesSpinBox;
    QCheckBox *collateCheckBox;
    QButtonGroup *orientationButtonGroup;

    explicit GeneralTab(QWidget *parent = 0);
};

class PageSetupTab : public QWidget
{
    Q_OBJECT

public:
    QCheckBox *bothSidesCheckBox;
    QComboBox *pagesPerSideComboBox;
    QComboBox *onlyPrintComboBox;
    QSpinBox *scaleSpinBox;
    QComboBox *paperSourceComboBox;

    explicit PageSetupTab(QWidget *parent = 0);
};

class OptionsTab : public QWidget
{
    Q_OBJECT

public:
    QLineEdit *marginTopValue;
    QLineEdit *marginBottomValue;
    QLineEdit *marginLeftValue;
    QLineEdit *marginRightValue;
    QComboBox *resolutionComboBox;

    explicit OptionsTab(QWidget *parent = 0);
};

class JobsTab : public QWidget
{
    Q_OBJECT

public:
    QPushButton *refreshButton;
    QComboBox *startJobComboBox;
    QPushButton *saveJobButton;

    explicit JobsTab(QWidget *parent = 0);
};

class CallbackFunctions : public QObject
{
    Q_OBJECT
public:
    explicit CallbackFunctions(QObject *parent = 0);
    static void add_printer_callback(PrinterObj *p);
    static void remove_printer_callback(PrinterObj *p);

Q_SIGNALS:
    void addPrinterSignal(char *printer_name, char *printer_id, char *backend_name);
    void removePrinterSignal(char *printer_name, char *printer_id, char *backend_name);

};

class QCommonPrintDialog : public QDialog
{
    Q_OBJECT

public:
    QStringList destinationList;

    explicit QCommonPrintDialog(QWidget *parent = 0);
    ~QCommonPrintDialog();
    void init_backend();

private Q_SLOTS:
    void addPrinter(char *printer_name, char *printer_id, char *backend_name);
    void removePrinter(char *printer_name, char *printer_id, char *backend_name);
    void quit();
    void newPrinterSelected(int);
    void remotePrintersCheckBoxStateChanged(int state);
    void collateCheckBoxStateChanged(int state);
    void orientationChanged(int buttonId);

private:
    GeneralTab *generalTab;
    PageSetupTab *pageSetupTab;
    OptionsTab *optionsTab;
    JobsTab *jobsTab;
    QTabWidget *tabWidget;
    QPrintPreviewWidget *preview;
    FrontendObj *f;
    PrinterObj *p;
    QString uniqueID;
};

typedef Singleton<CallbackFunctions> cbf;

#endif // QCOMMONPRINTDIALOG_H
