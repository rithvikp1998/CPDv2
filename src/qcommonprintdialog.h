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

struct _Option;
using Option = _Option;

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
    QComboBox *orientationComboBox;
    QComboBox *colorModeComboBox;

    explicit GeneralTab(QWidget *parent = nullptr);
};

class PageSetupTab : public QWidget
{
    Q_OBJECT

public:
    QComboBox *bothSidesComboBox;
    QComboBox *pagesPerSideComboBox;
    QComboBox *onlyPrintComboBox;
    QSpinBox *scaleSpinBox;
    QComboBox *paperSourceComboBox;
    QComboBox *pageRangeComboBox;

    explicit PageSetupTab(QWidget *parent = nullptr);
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
    QComboBox *qualityComboBox;
    QComboBox *outputBinComboBox;
    QComboBox *finishingsComboBox;
    QComboBox *ippAttributeFidelityComboBox;
    QFormLayout *layout;

    explicit OptionsTab(QWidget *parent = nullptr);
};

class JobsTab : public QWidget
{
    Q_OBJECT

public:
    QPushButton *refreshButton;
    QComboBox *startJobComboBox;
    QPushButton *saveJobButton;
    QComboBox *jobPriorityComboBox;
    QComboBox *jobSheetsComboBox;
    QComboBox *jobNameComboBox;
    QGridLayout *jobsLayout;
    QScrollArea *scrollArea;

    explicit JobsTab(QWidget *parent = nullptr);
};

class Preview : public QWidget
{
    Q_OBJECT
public:
    QPrintPreviewWidget *preview;
    QPrinter *printer;

    Preview(QPrinter *_printer, QString uniqueID, QWidget *parent = Q_NULLPTR);
    ~Preview();
    void setOrientation(const QString &orientation);
    void setPageSize(QString name, qreal width, qreal height, QString unit);
    void setNumCopies(int copies);
    void setCollateCopies(bool enabled);

public Q_SLOTS:
    void printPreview(QPrinter *printer);
    void setZoom(int zoomFactor);
    void showNextPage();
    void showPrevPage();

private:
    QPainter painter;
    int pageNumber = 0;
    int pageCount = 0;
    qreal baseZoomFactor = 0;
    bool zoomChanged = false;
};

class CallbackFunctions : public QObject
{
    Q_OBJECT
public:
    explicit CallbackFunctions(QObject *parent = nullptr);
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

    explicit QCommonPrintDialog(QWidget *parent = nullptr);
    ~QCommonPrintDialog();
    void init_backend();

private Q_SLOTS:
    void addPrinter(char *printer_name, char *printer_id, char *backend_name);
    void removePrinter(char *printer_name, char *printer_id, char *backend_name);
    void printJob();
    void quit();
    void tabChanged(int index);
    void newPrinterSelected(int index);
    void remotePrintersCheckBoxStateChanged(int state);
    void collateCheckBoxStateChanged(int state);
    void orientationChanged(int index);
    void printPreview(QPrinter *printer);
    void fillComboBox(QComboBox *comboBox, Option *value);
    void fillCollateCheckBox(Option *value);
    void fillMediaComboBox(Option *value);
    void fillCopiesOption(Option *value);
    void refreshJobs();
    void newPageSizeSelected(int index);
    void newColorModeSelected(int index);
    void newResolutionSelected(int index);
    void newDuplexOptionSelected(int index);
    void createNewComboBoxAndFill(char *optionName, Option *value);
    void changePrinterSetting(const QString &text);

private:
    GeneralTab *generalTab;
    PageSetupTab *pageSetupTab;
    OptionsTab *optionsTab;
    JobsTab *jobsTab;
    QTabWidget *tabWidget;
    Preview *preview;
    FrontendObj *f;
    PrinterObj *p;
    QString uniqueID;
};

typedef Singleton<CallbackFunctions> cbf;

#endif // QCOMMONPRINTDIALOG_H
