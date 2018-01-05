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

    QObject::connect(jobsTab->refreshButton,
                     SIGNAL(clicked()),
                     this,
                     SLOT(refreshJobs()));

    QObject::connect(generalTab->paperComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(newPageSizeSelected(int)));

    QObject::connect(generalTab->colorModeComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(newColorModeSelected(int)));

    QObject::connect(optionsTab->resolutionComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(newResolutionSelected(int)));

    QObject::connect(pageSetupTab->bothSidesComboBox,
                     SIGNAL(currentIndexChanged(int)),
                     this,
                     SLOT(newDuplexOptionSelected(int)));

    refreshJobs();
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

void QCommonPrintDialog::refreshJobs()
{
    QGridLayout *layout = jobsTab->jobsLayout;
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if(item->layout()){
            qDebug("Please don't");
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    layout->addWidget(new QLabel(tr("Printer")), 0, 0);
    layout->addWidget(new QLabel(tr("Location")), 0, 1);
    layout->addWidget(new QLabel(tr("Status")), 0, 2);

    Job *job;
    bool activeJobsOnly = false;
    int jobsCount = get_all_jobs(f, &job, activeJobsOnly);

    PrinterObj *pObj;
    for (int i = 0; i < jobsCount; i++) {
        pObj = find_PrinterObj(f, job[i].printer_id, job[i].backend_name);
        layout->addWidget(new QLabel(job[i].printer_id), i+1, 0);
        layout->addWidget(new QLabel(pObj->location), i+1, 1);
        layout->addWidget(new QLabel(job[i].state), i+1, 2);
    }
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
            fillCopiesOption(value);
        } else if (strncmp(key, "finishings", 10) == 0) {
            fillComboBox(optionsTab->finishingsComboBox, value);
        } else if (strncmp(key, "ipp-attribute-fidelity", 22) == 0) {
            fillComboBox(optionsTab->ippAttributeFidelityComboBox, value);
        } else if (strncmp(key, "job-hold-until", 14) == 0) {
            fillComboBox(jobsTab->startJobComboBox, value);
        } else if (strncmp(key, "job-name", 8) == 0) {
            fillComboBox(jobsTab->jobNameComboBox, value);
        } else if (strncmp(key, "job-priority", 12) == 0) {
            fillComboBox(jobsTab->jobPriorityComboBox, value);
        } else if (strncmp(key, "job-sheets", 10) == 0) {
            fillComboBox(jobsTab->jobSheetsComboBox, value);
        } else if (strncmp(key, "media-col", 9) == 0) {

        } else if (strncmp(key, "media", 5) == 0) {
            fillMediaComboBox(value);
        } else if (strncmp(key, "multiple-document-handling", 26) == 0) {
            fillCollateCheckBox(value);
        } else if (strncmp(key, "number-up", 9) == 0) {
            fillComboBox(pageSetupTab->pagesPerSideComboBox, value);
        } else if (strncmp(key, "output-bin", 10) == 0) {
            fillComboBox(optionsTab->outputBinComboBox, value);
        } else if (strncmp(key, "orientation-requested", 21) == 0) {
            fillComboBox(generalTab->orientationComboBox, value);
        } else if (strncmp(key, "page-ranges", 11) == 0) {
            fillComboBox(pageSetupTab->pageRangeComboBox, value);
        } else if (strncmp(key, "print-color-mode", 16) == 0) {
            fillComboBox(generalTab->colorModeComboBox, value);
        } else if (strncmp(key, "print-quality", 13) == 0) {
            fillComboBox(optionsTab->qualityComboBox, value);
        } else if (strncmp(key, "printer-resolution", 18) == 0) {
            fillComboBox(optionsTab->resolutionComboBox, value);
        } else if (strncmp(key, "sides", 5) == 0) {
            fillComboBox(pageSetupTab->bothSidesComboBox, value);
        } else {
            createNewComboBoxAndFill(key, value);
        }
    }
}

void QCommonPrintDialog::createNewComboBoxAndFill(char *optionName, Option *value)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setProperty("optionName", optionName);
    fillComboBox(comboBox, value);

    optionsTab->layout->addRow(new QLabel(tr(optionName)), comboBox);

    QObject::connect(comboBox,
                     SIGNAL(currentIndexChanged(const QString&)),
                     this,
                     SLOT(changePrinterSetting(const QString&)));
}

void QCommonPrintDialog::changePrinterSetting(const QString &text)
{
    QString optionName = qvariant_cast<QString>(sender()->property("optionName"));
    QString optionValue = text;
    qDebug("%s: %s", optionName.toLatin1().data(), text.toLatin1().data());
    add_setting_to_printer(p, optionName.toLatin1().data(), text.toLatin1().data());
}

void QCommonPrintDialog::fillCopiesOption(Option *value)
{
    QString copies(value->supported_values[0]);
    QStringList numCopies = copies.split("-"); // copies is in format 1-9999
    generalTab->copiesSpinBox->setRange(numCopies[0].toInt(), numCopies[1].toInt());
    generalTab->copiesSpinBox->setValue(numCopies[0].toInt());
}

void QCommonPrintDialog::fillMediaComboBox(Option *value)
{
    generalTab->paperComboBox->clear();
    for (int i = 0; i < value->num_supported; i++){
        generalTab->paperComboBox->addItem(pwg_to_readable(value->supported_values[i]));
        if(strcmp(value->supported_values[i], value->default_value) == 0)
            generalTab->paperComboBox->setCurrentIndex(generalTab->paperComboBox->count() - 1);
    }
}

void QCommonPrintDialog::fillComboBox(QComboBox *comboBox, Option *value)
{
    comboBox->clear();
    for (int i = 0; i < value->num_supported; i++){
        comboBox->addItem(value->supported_values[i]);
        if(strcmp(value->supported_values[i], value->default_value) == 0)
            comboBox->setCurrentIndex(comboBox->count() - 1);
    }
    if (comboBox->count() == 0)
        comboBox->setEnabled(false);
}

void QCommonPrintDialog::fillCollateCheckBox(Option *value)
{
    if (value->num_supported == 0) {
        generalTab->collateCheckBox->setEnabled(false);
    }
    else if (value->num_supported == 1){
        if (strcmp(value->supported_values[0], " separate-documents-collated-copies") == 0) {
            generalTab->collateCheckBox->setChecked(true);
            generalTab->collateCheckBox->setEnabled(false);
        } else {
            generalTab->collateCheckBox->setChecked(false);
            generalTab->collateCheckBox->setEnabled(false);
        }
    }
    else {
        if (strcmp(value->default_value, " separate-documents-collated-copies") == 0)
            generalTab->collateCheckBox->setChecked(true);
        else if (strcmp(value->default_value, " separate-documents-uncollated-copies") == 0)
            generalTab->collateCheckBox->setChecked(false);
        else
            generalTab->collateCheckBox->setChecked(false);
    }
}

void QCommonPrintDialog::collateCheckBoxStateChanged(int state)
{
    QString collateSetting = state == Qt::Checked ? "separate-documents-collated-copies" :
                                                    "separate-documents-uncollated-copies";
    add_setting_to_printer(p, QString("multiple-document-handling").toLatin1().data(),
                           collateSetting.toLatin1().data());
    preview->printer->setCollateCopies(state==Qt::Checked);
}

void QCommonPrintDialog::orientationChanged(int index)
{
    QString orientation = generalTab->orientationComboBox->itemText(index);
    add_setting_to_printer(p, QString("orientation-requested").toLatin1().data(),
                           orientation.toLatin1().data());
    preview->setOrientation(orientation);
}

void QCommonPrintDialog::newPageSizeSelected(int index)
{
    QString pageSize = readable_to_pwg(generalTab->paperComboBox->itemText(index).toLatin1().data());
    if(pageSize == "")
        return;
    QStringList pageSizeSplitList = pageSize.split("_");
    QString size = pageSizeSplitList[2];
    QStringList sizeSplitList = size.split("x");

    qreal width = sizeSplitList[0].toDouble();

    QString unit = sizeSplitList[1].right(2);
    sizeSplitList[1].remove(unit);

    qreal height = sizeSplitList[1].toDouble();

    preview->setPageSize(pageSizeSplitList[1], width, height, unit);
    add_setting_to_printer(p,
                           QString("media").toLatin1().data(),
                           pageSize.toLatin1().data());
}

void QCommonPrintDialog::newColorModeSelected(int index)
{
    QString colorMode = generalTab->colorModeComboBox->itemText(index);
    if(colorMode == "")
        return;
    add_setting_to_printer(p, "print-color-mode", colorMode.toLatin1().data());
    if(colorMode.compare("color") == 0)
        preview->printer->setColorMode(QPrinter::Color);
    else if(colorMode.compare("monochrome") == 0)
        preview->printer->setColorMode(QPrinter::GrayScale);
    else
        qDebug("Unhandled color mode option: %s", colorMode.toLatin1().data());
    preview->update();
}

void QCommonPrintDialog::newResolutionSelected(int index)
{
    QString resolution = optionsTab->resolutionComboBox->itemText(index);
    if(resolution == "")
        return;
    add_setting_to_printer(p, "print-resolution", resolution.toLatin1().data());
    int resolutionValue = resolution.replace("dpi", "").toInt();
    preview->printer->setResolution(resolutionValue);
    preview->update();
}

void QCommonPrintDialog::newDuplexOptionSelected(int index)
{
    QString duplexOption = pageSetupTab->bothSidesComboBox->itemText(index);
    if(duplexOption == "")
        return;
    add_setting_to_printer(p, "sides", duplexOption.toLatin1().data());
    if(duplexOption.compare("one-sided") == 0)
        preview->printer->setDuplex(QPrinter::DuplexNone);
    else if(duplexOption.compare("two-sided-long-edge") == 0)
        preview->printer->setDuplex(QPrinter::DuplexLongSide);
    else if(duplexOption.compare("two-sided-short-edge") == 0)
        preview->printer->setDuplex(QPrinter::DuplexShortSide);
    else
        qDebug("Unhandled duplex option: %s", duplexOption.toLatin1().data());
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
    pageRangeComboBox = new QComboBox;

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
    finishingsComboBox = new QComboBox;
    ippAttributeFidelityComboBox = new QComboBox;

    layout = new QFormLayout;

    layout->addRow((new QLabel(tr("Margin"))));
    layout->addRow(new QLabel(tr("Top")), marginTopValue);
    layout->addRow(new QLabel(tr("Bottom")), marginBottomValue);
    layout->addRow(new QLabel(tr("Left")), marginLeftValue);
    layout->addRow(new QLabel(tr("Right")), marginRightValue);
    layout->addRow(new QLabel(tr("")));
    layout->addRow(new QLabel(tr("Resolution")), resolutionComboBox);
    layout->addRow(new QLabel(tr("Quality")), qualityComboBox);
    layout->addRow(new QLabel(tr("Output Bin")), outputBinComboBox);
    layout->addRow(new QLabel(tr("Finishings")), finishingsComboBox);

    setLayout(layout);
}

JobsTab::JobsTab(QWidget *parent)
{
    QWidget *jobsWidget = new QWidget;
    jobsLayout = new QGridLayout;

    jobsLayout->addWidget(new QLabel(tr("Printer")), 0, 0);
    jobsLayout->addWidget(new QLabel(tr("Location")), 0, 1);
    jobsLayout->addWidget(new QLabel(tr("Status")), 0, 2);

    jobsWidget->setLayout(jobsLayout);

    scrollArea = new QScrollArea;
    scrollArea->setWidget(jobsWidget);

    refreshButton = new QPushButton("Refresh");
    startJobComboBox = new QComboBox;
    saveJobButton = new QPushButton("Save");
    jobNameComboBox = new QComboBox;
    jobPriorityComboBox = new QComboBox;
    jobSheetsComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;
    layout->addRow(scrollArea);
    layout->addRow(new QLabel(tr("Refresh")), refreshButton);
    layout->addRow(new QLabel(tr("Start Job")), startJobComboBox);
    layout->addRow(new QLabel(tr("Save Job")), saveJobButton);
    layout->addRow(new QLabel(tr("Job Name")), jobNameComboBox);
    layout->addRow(new QLabel(tr("Job Priority")), jobPriorityComboBox);
    layout->addRow(new QLabel(tr("Job Sheets")), jobSheetsComboBox);

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
