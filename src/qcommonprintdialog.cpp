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
    init_backend();

    QStringList destinationList;

    tabWidget = new QTabWidget;

    generalTab = new GeneralTab(parent);
    pageSetupTab = new PageSetupTab(parent);
    optionsTab = new OptionsTab(parent);
    jobsTab = new JobsTab(parent);

    tabWidget->addTab(generalTab, tr("General"));
    tabWidget->addTab(pageSetupTab, tr("Page Setup"));
    tabWidget->addTab(optionsTab, tr("Options"));
    tabWidget->addTab(jobsTab, tr("Jobs"));

    QPrinter *printer = new QPrinter;
    preview = new Preview(printer, uniqueID, this);

    QPushButton *printButton = new QPushButton(tr("Print"));
    printButton->setDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(printButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(tabWidget);
    leftLayout->addItem(buttonLayout);

    QPushButton *showPrevPageButton = new QPushButton("<");
    QPushButton *showNextPageButton = new QPushButton(">");
    QSpinBox *zoomSpinBox = new QSpinBox;
    zoomSpinBox->setRange(100,200);
    zoomSpinBox->setSuffix("%");
    zoomSpinBox->setSingleStep(10);
    QHBoxLayout *zoomSliderLayout = new QHBoxLayout;
    zoomSliderLayout->addWidget(showPrevPageButton, 1);
    zoomSliderLayout->addWidget(new QLabel(tr("Zoom")), 1, Qt::AlignHCenter);
    zoomSliderLayout->addWidget(zoomSpinBox, 2);
    zoomSliderLayout->addWidget(showNextPageButton, 1);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(preview->preview);
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
                     SIGNAL(removePrinterSignal(char *, char *, char *)),
                     this,
                     SLOT(removePrinter(char *, char *, char *)));

    QObject::connect(cancelButton,
                     SIGNAL(clicked()),
                     this,
                     SLOT(quit()));

    QObject::connect(generalTab->destinationComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(newPrinterSelected(int)));

    QObject::connect(generalTab->remotePrintersCheckBox,
                     SIGNAL(stateChanged(int)),
                     this,
                     SLOT(remotePrintersCheckBoxStateChanged(int)));

    QObject::connect(generalTab->collateCheckBox,
                     SIGNAL(stateChanged(int)),
                     this,
                     SLOT(collateCheckBoxStateChanged(int)));

    QObject::connect(generalTab->orientationComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(orientationChanged(int)));

    QObject::connect(zoomSpinBox,
                     SIGNAL(valueChanged(int)),
                     preview,
                     SLOT(setZoom(int)));

    QObject::connect(showPrevPageButton,
                     SIGNAL(clicked()),
                     preview,
                     SLOT(showPrevPage()));

    QObject::connect(showNextPageButton,
                     SIGNAL(clicked()),
                     preview,
                     SLOT(showNextPage()));
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

void QCommonPrintDialog::printPreview(QPrinter *printer)
{
    QPainter painter;
    painter.begin(printer);
    painter.setFont(QFont("Helvetica", 24, QFont::Bold, true));
    painter.drawText(printer->pageRect(), Qt::AlignCenter, tr("Sample Preivew"));
    painter.end();
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
    cbf::Instance()->removePrinterSignal(p->name, p->id, p->backend_name);
}

void QCommonPrintDialog::addPrinter(char *printer_name, char *printer_id, char *backend_name)
{
    qDebug("Add printer %s, %s, %s", printer_name, printer_id, backend_name);
    destinationList.append(QString("%1#%2").arg(printer_id).arg(backend_name));
    generalTab->destinationComboBox->addItem(printer_name);
}

void QCommonPrintDialog::removePrinter(char *printer_name, char *printer_id, char *backend_name)
{
    int i = destinationList.indexOf(QString("%1#%2").arg(printer_id).arg(backend_name));
    destinationList.removeAt(i);
    generalTab->destinationComboBox->removeItem(i);
}

void QCommonPrintDialog::quit()
{
    disconnect_from_dbus(f);
    reject();
}

void QCommonPrintDialog::remotePrintersCheckBoxStateChanged(int state)
{
    state == Qt::Checked ? unhide_remote_cups_printers(f) : hide_remote_cups_printers(f);
}

void QCommonPrintDialog::newPrinterSelected(int index)
{
    QString printer = destinationList[index];
    QStringList list = printer.split('#');  // printer is in the format: <printer_id>#<backend_name>
    p = find_PrinterObj(f, list[0].toLatin1().data(), list[1].toLatin1().data());

    Options *options = get_all_options(p);

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, options->table);
    gpointer _key, _value;

    while (g_hash_table_iter_next(&iter, &_key, &_value)) {
        char *key = static_cast<char *>(_key);
        Option *value = static_cast<Option *>(_value);
        if (strncmp(key, "copies", 6) == 0) {
            QString copies(value->supported_values[0]);
            QStringList numCopies = copies.split("-"); // copies is in format 1-9999
            generalTab->copiesSpinBox->setRange(numCopies[0].toInt(), numCopies[1].toInt());
            generalTab->copiesSpinBox->setValue(numCopies[0].toInt());
        } else if (strncmp(key, "finishings", 10) == 0) {

        } else if (strncmp(key, "ipp-attribute-fidelity", 22) == 0) {

        } else if (strncmp(key, "job-hold-until", 14) == 0) {
            jobsTab->startJobComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                jobsTab->startJobComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    jobsTab->startJobComboBox->setCurrentIndex(jobsTab->startJobComboBox->count() - 1);
            }
        } else if (strncmp(key, "job-name", 8) == 0) {

        } else if (strncmp(key, "job-priority", 12) == 0) {

        } else if (strncmp(key, "job-sheets", 10) == 0) {

        } else if (strncmp(key, "media-col", 9) == 0) {

        } else if (strncmp(key, "media", 5) == 0) {
            generalTab->paperComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                generalTab->paperComboBox->addItem(pwg_to_readable(value->supported_values[i]));
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    generalTab->paperComboBox->setCurrentIndex(generalTab->paperComboBox->count() - 1);
            }
        } else if (strncmp(key, "multiple-document-handling", 26) == 0) {

        } else if (strncmp(key, "number-up", 9) == 0) {
            pageSetupTab->pagesPerSideComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                pageSetupTab->pagesPerSideComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    pageSetupTab->pagesPerSideComboBox->setCurrentIndex(pageSetupTab->pagesPerSideComboBox->count() - 1);
            }
        } else if (strncmp(key, "output-bin", 10) == 0) {
            optionsTab->outputBinComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                optionsTab->outputBinComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    optionsTab->outputBinComboBox->setCurrentIndex(optionsTab->outputBinComboBox->count() - 1);
            }
        } else if (strncmp(key, "orientation-requested", 21) == 0) {
            generalTab->orientationComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                generalTab->orientationComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    generalTab->orientationComboBox->setCurrentIndex(generalTab->orientationComboBox->count() - 1);
            }
        } else if (strncmp(key, "page-ranges", 11) == 0) {

        } else if (strncmp(key, "print-color-mode", 16) == 0) {
            generalTab->colorModeComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                generalTab->colorModeComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    generalTab->colorModeComboBox->setCurrentIndex(generalTab->colorModeComboBox->count() - 1);
            }
        } else if (strncmp(key, "print-quality", 13) == 0) {
            optionsTab->qualityComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                optionsTab->qualityComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    optionsTab->qualityComboBox->setCurrentIndex(optionsTab->qualityComboBox->count() - 1);
            }
        } else if (strncmp(key, "printer-resolution", 18) == 0) {
            optionsTab->resolutionComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                optionsTab->resolutionComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    optionsTab->resolutionComboBox->setCurrentIndex(optionsTab->resolutionComboBox->count() - 1);
            }
        } else if (strncmp(key, "sides", 5) == 0) {
            pageSetupTab->bothSidesComboBox->clear();
            for (int i = 0; i < value->num_supported; i++){
                pageSetupTab->bothSidesComboBox->addItem(value->supported_values[i]);
                if(strcmp(value->supported_values[i], value->default_value) == 0)
                    pageSetupTab->bothSidesComboBox->setCurrentIndex(pageSetupTab->bothSidesComboBox->count() - 1);
            }
        } else {
            qDebug() << "Unhandled Option:" << key;
        }
    }
}

void QCommonPrintDialog::collateCheckBoxStateChanged(int state)
{
    QString collateSetting = state == Qt::Checked ? "separate-documents-collated-copies" :
                                                    "separate-documents-uncollated-copies";
    add_setting_to_printer(p, QString("multiple-document-handling").toLatin1().data(),
                           collateSetting.toLatin1().data());
}

void QCommonPrintDialog::orientationChanged(int index)
{
    QString orientation = generalTab->orientationComboBox->itemText(index);
    add_setting_to_printer(p, QString("orientation-requested").toLatin1().data(),
                           orientation.toLatin1().data());
}

GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    destinationComboBox = new QComboBox;
    remotePrintersCheckBox = new QCheckBox;
    paperComboBox = new QComboBox;
    pagesComboBox = new QComboBox;
    copiesSpinBox = new QSpinBox;
    collateCheckBox = new QCheckBox;
    orientationComboBox = new QComboBox;
    colorModeComboBox = new QComboBox;

    pagesComboBox->addItem("All");

    QFormLayout *layout = new QFormLayout;

    layout->addRow(new QLabel(tr("Destination")), destinationComboBox);
    layout->addRow(new QLabel(tr("Remote Printers")), remotePrintersCheckBox);
    layout->addRow(new QLabel(tr("Paper")), paperComboBox);
    layout->addRow(new QLabel(tr("Pages")), pagesComboBox);
    layout->addRow(new QLabel(tr("Copies")), copiesSpinBox);
    layout->addRow(new QLabel(tr("Collate Pages")), collateCheckBox);
    layout->addRow(new QLabel(tr("Orientation")), orientationComboBox);
    layout->addRow(new QLabel(tr("Color Mode")), colorModeComboBox);

    setLayout(layout);
}

PageSetupTab::PageSetupTab(QWidget *parent)
{
    bothSidesComboBox = new QComboBox;
    pagesPerSideComboBox = new QComboBox;
    onlyPrintComboBox = new QComboBox;
    scaleSpinBox = new QSpinBox;
    scaleSpinBox->setRange(0, 200);
    scaleSpinBox->setValue(100);
    scaleSpinBox->setSuffix("%");
    paperSourceComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;

    layout->addRow(new QLabel(tr("Layout")));
    layout->addRow(new QLabel(tr("Print Both Sides")), bothSidesComboBox);
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
    marginTopValue = new QLineEdit;
    marginBottomValue = new QLineEdit;
    marginLeftValue = new QLineEdit;
    marginRightValue = new QLineEdit;
    resolutionComboBox = new QComboBox;
    qualityComboBox = new QComboBox;
    outputBinComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;

    layout->addRow((new QLabel(tr("Margin"))));
    layout->addRow(new QLabel(tr("Top")), marginTopValue);
    layout->addRow(new QLabel(tr("Bottom")), marginBottomValue);
    layout->addRow(new QLabel(tr("Left")), marginLeftValue);
    layout->addRow(new QLabel(tr("Right")), marginRightValue);
    layout->addRow(new QLabel(tr("")));
    layout->addRow(new QLabel(tr("Resolution")), resolutionComboBox);
    layout->addRow(new QLabel(tr("Quality")), qualityComboBox);
    layout->addRow(new QLabel(tr("Output Bin")), outputBinComboBox);

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

    refreshButton = new QPushButton("Refresh");
    startJobComboBox = new QComboBox;
    saveJobButton = new QPushButton("Save");

    QFormLayout *layout = new QFormLayout;
    layout->addRow(scrollArea);
    layout->addRow(new QLabel(tr("Refresh")), refreshButton);
    layout->addRow(new QLabel(tr("Start Job")), startJobComboBox);
    layout->addRow(new QLabel(tr("Save Job")), saveJobButton);

    setLayout(layout);
}

Preview::Preview(QPrinter *_printer, QString uniqueID, QWidget *parent) :
    QWidget(parent),
    printer(_printer)
{
    preview = new QPrintPreviewWidget(printer);

    preview->setGeometry(0, 0, 380, 408);
    preview->fitInView();

    printer->setPaperSize(QPrinter::A4);
    printer->setOrientation(QPrinter::Portrait);

    uniqueID.prepend("/tmp/");
    uniqueID.append(".pdf");

    printer->setOutputFileName(uniqueID);
    printer->setOutputFormat(QPrinter::NativeFormat);

    QObject::connect(preview,
                     SIGNAL(paintRequested(QPrinter *)),
                     this,
                     SLOT(printPreview(QPrinter *)));
}

Preview::~Preview() = default;

void Preview::printPreview(QPrinter *printer)
{
    painter.begin(printer);

    painter.setFont(QFont("Helvetica", 24, QFont::Bold, true));
    painter.drawText(printer->pageRect(), Qt::AlignCenter, tr("Sample Preivew"));

    painter.end();
}

void Preview::setOrientation(const QString &orientation)
{
    if (orientation.compare("portrait") == 0)
        printer->setOrientation(QPrinter::Portrait);
    else if (orientation.compare("landscape") == 0)
        printer->setOrientation(QPrinter::Landscape);
    else qDebug() << "Unhandled Orientation:" << orientation;

    preview->updatePreview();
}

void Preview::setPageSize(QString name, qreal width, qreal height, QString unit)
{
    QPageSize::Unit pageSizeUnit = QPageSize::Unit::Inch;
    if (unit.compare("in") == 0)
        pageSizeUnit = QPageSize::Unit::Inch;
    else if (unit.compare("mm") == 0)
        pageSizeUnit = QPageSize::Unit::Millimeter;
    else
        qDebug() << "Unhandled Unit in Paper Size:" << unit;

    printer->setPageSize(QPageSize(QSizeF(width, height),
                                   pageSizeUnit,
                                   name,
                                   QPageSize::SizeMatchPolicy::FuzzyMatch));

    preview->updatePreview();
}

void Preview::setNumCopies(int copies)
{
    printer->setNumCopies(copies);
    preview->updatePreview();
}

void Preview::setCollateCopies(bool enabled)
{
    printer->setCollateCopies(enabled);
    preview->updatePreview();
}

void Preview::setZoom(int zoomFactor)
{
    if (!zoomChanged) {
        // Sets the base zoom factor if zoom has not been changed
        baseZoomFactor = preview->zoomFactor();
        zoomChanged = true;
    }
    /* Whenever zoom slider changes, it rescales it down to
     * baseZoomFactor and then zoom in to the desired amount.
     */
    preview->setZoomFactor(baseZoomFactor);
    preview->zoomIn(zoomFactor/100.0);
}

void Preview::showNextPage()
{
    pageNumber = pageNumber < (pageCount - 1) ? pageNumber + 1 : pageNumber;
    preview->updatePreview();
}

void Preview::showPrevPage()
{
    pageNumber = pageNumber > 0 ? pageNumber - 1 : pageNumber;
    preview->updatePreview();
}
