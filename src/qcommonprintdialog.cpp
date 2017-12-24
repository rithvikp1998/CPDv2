#include <QtWidgets>
#include <QPrintPreviewWidget>

#include "qcommonprintdialog.h"

QCommonPrintDialog::QCommonPrintDialog(QWidget *parent) :
    QDialog (parent)
{
    resize(640, 480);

    tabWidget = new QTabWidget;
    tabWidget->addTab(new GeneralTab(parent), tr("General"));
    tabWidget->addTab(new PageSetupTab(parent), tr("Page Setup"));
    tabWidget->addTab(new OptionsTab(parent), tr("Options"));
    tabWidget->addTab(new JobsTab(parent), tr("Jobs"));

    QPrintPreviewWidget *preview = new QPrintPreviewWidget;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(tabWidget);
    layout->addWidget(preview);
    setLayout(layout);
}

QCommonPrintDialog::~QCommonPrintDialog() = default;

GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{

}

PageSetupTab::PageSetupTab(QWidget *parent)
{
    QCheckBox *bothSidesCheckBox = new QCheckBox;
    QComboBox *pagesPerSideComboBox = new QComboBox;
    QComboBox *onlyPrintComboBox = new QComboBox;
    QSpinBox *scaleSpinBox = new QSpinBox;
    QComboBox *paperSourceComboBox = new QComboBox;

    QFormLayout *layout = new QFormLayout;
    layout->setLabelAlignment(Qt::AlignLeft);

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
    layout->setLabelAlignment(Qt::AlignLeft);

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

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(scrollArea);
    setLayout(layout);

}
