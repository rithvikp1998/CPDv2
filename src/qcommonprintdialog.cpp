#include <QtWidgets>
#include <QPrintPreviewWidget>

#include "qcommonprintdialog.h"

extern "C" {
#include <cpdb-libs-frontend.h>
}

QCommonPrintDialog::QCommonPrintDialog(QWidget *parent) :
    QDialog (parent)
{
    resize(720, 480);

    tabWidget = new QTabWidget;

    tabWidget->addTab(new GeneralTab(parent), tr("General"));
    tabWidget->addTab(new PageSetupTab(parent), tr("Page Setup"));
    tabWidget->addTab(new OptionsTab(parent), tr("Options"));
    tabWidget->addTab(new JobsTab(parent), tr("Jobs"));

    QPrintPreviewWidget *preview = new QPrintPreviewWidget;

    QPushButton *printButton = new QPushButton(tr("Print"));
    printButton->setDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(printButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(tabWidget);
    leftLayout->addItem(buttonLayout);

    QSpinBox *zoomSpinBox = new QSpinBox;
    zoomSpinBox->setRange(100,200);
    zoomSpinBox->setSuffix("%");
    QHBoxLayout *zoomSliderLayout = new QHBoxLayout;
    zoomSliderLayout->addWidget(new QLabel(tr("Zoom")));
    zoomSliderLayout->addWidget(zoomSpinBox);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(preview);
    rightLayout->addItem(zoomSliderLayout);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addItem(leftLayout);
    layout->addItem(rightLayout);
    setLayout(layout);

    QObject::connect(cbf::Instance(),
                     SIGNAL(addPrinterSignal(char *, char *, char *)),
                     this,
                     SLOT(addPrinter(char *, char *, char *)));

    QObject::connect(cbf::Instance(),
                     SIGNAL(removePrinterSignal(char *)),
                     this,
                     SLOT(removePrinter(char *)));

    init_backend();
}

QCommonPrintDialog::~QCommonPrintDialog() = default;

void QCommonPrintDialog::init_backend()
{
    event_callback add_cb = (event_callback)CallbackFunctions::add_printer_callback;
    event_callback rem_cb = (event_callback)CallbackFunctions::remove_printer_callback;

    uniqueID = QUuid::createUuid().toString().remove('{').remove('}');
    f = get_new_FrontendObj(uniqueID.toLatin1().data(), add_cb, rem_cb);
    connect_to_dbus(f);
}

CallbackFunctions::CallbackFunctions(QObject *parent):
    QObject (parent)
{
}

void CallbackFunctions::add_printer_callback(PrinterObj *p)
{
    cbf::Instance()->addPrinterSignal(p->name, p->id, p->backend_name);
}

void CallbackFunctions::remove_printer_callback(PrinterObj *p)
{
    cbf::Instance()->removePrinterSignal(p->name);
}

void QCommonPrintDialog::addPrinter(char *printer_name, char *printer_id, char *backend_name)
{
    qDebug("Add printer %s", printer_name);
}

void QCommonPrintDialog::removePrinter(char *printer_name)
{
    qDebug("Remove Printer %s", printer_name);
}

GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    QComboBox *destinationComboBox = new QComboBox;
    QCheckBox *remotePrintersCheckBox = new QCheckBox;
    QComboBox *paperComboBox = new QComboBox;
    QComboBox *pagesComboBox = new QComboBox;
    QSpinBox *copiesSpinBox = new QSpinBox;
    copiesSpinBox->setMinimum(1);
    QCheckBox *collateCheckBox = new QCheckBox;

    QGroupBox *orientationGroupBox = new QGroupBox;
    QRadioButton *portraitButton = new QRadioButton(tr("Portrait"));
    QRadioButton *landscapeButton = new QRadioButton(tr("Landscape"));
    QHBoxLayout *orientationGroupBoxLayout = new QHBoxLayout;
    orientationGroupBoxLayout->addWidget(portraitButton);
    orientationGroupBoxLayout->addWidget(landscapeButton);
    portraitButton->setChecked(true);
    orientationGroupBox->setLayout(orientationGroupBoxLayout);

    QFormLayout *layout = new QFormLayout;

    layout->addRow(new QLabel(tr("Destination")), destinationComboBox);
    layout->addRow(new QLabel(tr("Remote Printers")), remotePrintersCheckBox);
    layout->addRow(new QLabel(tr("Paper")), paperComboBox);
    layout->addRow(new QLabel(tr("Pages")), pagesComboBox);
    layout->addRow(new QLabel(tr("Copies")), copiesSpinBox);
    layout->addRow(new QLabel(tr("Collate Pages")), collateCheckBox);
    layout->addRow(new QLabel(tr("Orientation")), orientationGroupBox);

    setLayout(layout);
}

PageSetupTab::PageSetupTab(QWidget *parent)
{
    QCheckBox *bothSidesCheckBox = new QCheckBox;
    QComboBox *pagesPerSideComboBox = new QComboBox;
    QComboBox *onlyPrintComboBox = new QComboBox;
    QSpinBox *scaleSpinBox = new QSpinBox;
    scaleSpinBox->setRange(0, 200);
    scaleSpinBox->setValue(100);
    scaleSpinBox->setSuffix("%");
    QComboBox *paperSourceComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;

    layout->addRow(new QLabel(tr("Layout")));
    layout->addRow(new QLabel(tr("Print Both Sides")), bothSidesCheckBox);
    layout->addRow(new QLabel(tr("Pages Per Side")), pagesPerSideComboBox);
    layout->addRow(new QLabel(tr("Only Print")), onlyPrintComboBox);
    layout->addRow(new QLabel(tr("Scale")), scaleSpinBox);
    layout->addRow(new QLabel(tr("")));

    layout->addRow(new QLabel(tr("Paper")));
    layout->addRow(new QLabel(tr("Paper Source")), paperSourceComboBox);

    setLayout(layout);
}

OptionsTab::OptionsTab(QWidget *parent)
{
    QLineEdit *marginTopValue = new QLineEdit;
    QLineEdit *marginBottomValue = new QLineEdit;
    QLineEdit *marginLeftValue = new QLineEdit;
    QLineEdit *marginRightValue = new QLineEdit;
    QComboBox *resolutionComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;

    layout->addRow((new QLabel(tr("Margin"))));
    layout->addRow(new QLabel(tr("Top")), marginTopValue);
    layout->addRow(new QLabel(tr("Bottom")), marginBottomValue);
    layout->addRow(new QLabel(tr("Left")), marginLeftValue);
    layout->addRow(new QLabel(tr("Right")), marginRightValue);
    layout->addRow(new QLabel(tr("")));
    layout->addRow(new QLabel(tr("Resolution")), resolutionComboBox);

    setLayout(layout);
}

JobsTab::JobsTab(QWidget *parent)
{
    QWidget *jobsWidget = new QWidget;
    QGridLayout *jobsLayout = new QGridLayout;

    jobsLayout->addWidget(new QLabel(tr("Printer")), 1, 1);
    jobsLayout->addWidget(new QLabel(tr("Location")), 1, 2);
    jobsLayout->addWidget(new QLabel(tr("Status")), 1, 3);
    jobsWidget->setLayout(jobsLayout);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(jobsWidget);

    QPushButton *refreshButton = new QPushButton("Refresh");
    QComboBox *startJobComboBox = new QComboBox;
    QPushButton *saveJobButton = new QPushButton("Save");

    QFormLayout *layout = new QFormLayout;
    layout->addRow(scrollArea);
    layout->addRow(new QLabel(tr("Refresh")), refreshButton);
    layout->addRow(new QLabel(tr("Start Job")), startJobComboBox);
    layout->addRow(new QLabel(tr("Save Job")), saveJobButton);

    setLayout(layout);
}
