#include <indedds-monitor/model/widget.h>
#include "ui_widget.h"
#include <vector>
#include <QString>
#include <QRandomGenerator>
#include <QTableWidgetItem>
#include <indedds-monitor/model/system_overview_model.h>
#include <iostream>
#include <QDebug>          // <-- add this line


static QString toDisplayMode(const QString& text)
{
    if (text == "Hide")       return "Hide";
    if (text == "No Name")    return "NoName";
    if (text == "Terse")      return "Terse";
    if (text == "Role Name")  return "RoleName";
    if (text == "Topic Name") return "TopicName";
    // Treat "Entity Name" like RoleName
    if (text == "Entity Name") return "RoleName";
    return "RoleName";
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Add items to the 'measurement' QComboBox
    ui->measurementcombobox->addItem("Samples Received Bytes");
    ui->measurementcombobox->addItem("Samples Received Bytes Throughput");
    ui->measurementcombobox->addItem("Samples Received Count");
    ui->measurementcombobox->addItem("Samples Received Count Throughput");
    ui->measurementcombobox->addItem("Samples Written Bytes");
    ui->measurementcombobox->addItem("Samples Written Bytes Throughput");
    ui->measurementcombobox->addItem("Samples Written Count");
    ui->measurementcombobox->addItem("Samples Written Count Throughput");

    ui->scalecombobox->addItem("No Scaling");
    ui->scalecombobox->addItem("Micros(10e-6)");
    ui->scalecombobox->addItem("Millis(10e-3)");
    ui->scalecombobox->addItem("Thousands");
    ui->scalecombobox->addItem("Millions");
    ui->scalecombobox->addItem("Kilo");
    ui->scalecombobox->addItem("Megi");
    ui->scalecombobox->addItem("Gigo");

    connect(ui->notificationsradio, &QRadioButton::toggled, this, &Widget::updateBoxVisibility);
    connect(ui->matchesradio, &QRadioButton::toggled, this, &Widget::updateBoxVisibility);
    connect(ui->measurementradio, &QRadioButton::toggled, this, &Widget::updateBoxVisibility);

    // Connect allcomboBox to update other combo boxes
    connect(ui->allcomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Widget::updateComboBoxes);

    // When "All" changes, update model display modes
    connect(ui->allcomboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int index)
        {
            if (!system_overview_model_)
                return;
            const QString mode = toDisplayMode(ui->allcomboBox->itemText(index));
            system_overview_model_->setDisplayAllMode(mode);
            system_overview_model_->setDisplayHostMode(mode);
            system_overview_model_->setDisplayProcessMode(mode);
        });

    // Connect the refresh button to the refreshTable slot
    connect(ui->RefreshButton, &QPushButton::clicked, this, &Widget::refreshTable);

    // Connect highlight mode radio buttons to System Overview Model
    connect(ui->notificationsradio, &QRadioButton::clicked, this, &Widget::onHighlightModeChanged);
    connect(ui->matchesradio, &QRadioButton::clicked, this, &Widget::onHighlightModeChanged);
    connect(ui->measurementradio, &QRadioButton::clicked, this, &Widget::onHighlightModeChanged);

    // Connect measurement combobox
    connect(ui->measurementcombobox, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
        this, &Widget::onMeasurementChanged);

    // Connect scale combobox
    connect(ui->scalecombobox, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
        this, &Widget::onScaleChanged);

    std::cout << "[Widget] ✓ System Overview signals connected" << std::endl;

    // Initialize the visibility of the boxes
    updateBoxVisibility();

    // Vector with values to add to the combo boxes
    std::vector<QString> comboBoxValues = {
        "Hide", "No Name", "Terse", "Entity Name", "Role Name", "Topic Name"
    };

    // Add values to the specified combo boxes
    for (const auto& value : comboBoxValues) {
        ui->allcomboBox->addItem(value);
        ui->comboBox->addItem(value);
        ui->comboBox_2->addItem(value);
        ui->comboBox_3->addItem(value);
        ui->comboBox_4->addItem(value);
        ui->comboBox_5->addItem(value);
        ui->comboBox_7->addItem(value);
        ui->comboBox_8->addItem(value);
        ui->comboBox_9->addItem(value);
    }

    // Set up the table widget
    ui->tableWidget->setColumnCount(6);
    QStringList tableHeaders = {"Topic", "Data Reader", "Data Writer", "DomainID", "Process ID", "Hostname"};
    ui->tableWidget->setHorizontalHeaderLabels(tableHeaders);

    // Set custom column widths
    ui->tableWidget->setColumnWidth(0, 150); // Topic
    ui->tableWidget->setColumnWidth(1, 120); // Data Reader
    ui->tableWidget->setColumnWidth(2, 120); // Data Writer
    ui->tableWidget->setColumnWidth(3, 100); // DomainID
    ui->tableWidget->setColumnWidth(4, 100); // Process ID
    ui->tableWidget->setColumnWidth(5, 150); // Hostname

}

Widget::~Widget()
{
    delete ui;
}

void Widget::openWidget() {
    // Widget* openWidgetwindow = new Widget();
    show();
}

void Widget::updateBoxVisibility()
{
    // Show or hide the boxes based on the radio button states
    ui->Notificationbox->setVisible(ui->notificationsradio->isChecked());
    ui->matchesbox->setVisible(ui->matchesradio->isChecked());
    ui->Measurementbox->setVisible(ui->measurementradio->isChecked());

    // Enable or disable measurement related widgets based on measurementradio state
    bool isMeasurementChecked = ui->measurementradio->isChecked();
    ui->measuremntlabel->setEnabled(isMeasurementChecked);
    ui->measurementcombobox->setEnabled(isMeasurementChecked);
    ui->scalelabel->setEnabled(isMeasurementChecked);
    ui->scalecombobox->setEnabled(isMeasurementChecked);

    // If none are checked, show the Notificationbox by default
    if (!ui->notificationsradio->isChecked() && !ui->matchesradio->isChecked() && !ui->measurementradio->isChecked()) {
        ui->Notificationbox->setVisible(true);
    }
}

void Widget::updateComboBoxes(int index)
{
    // Update all other combo boxes to match the selected index of allcomboBox
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox_2->setCurrentIndex(index);
    ui->comboBox_3->setCurrentIndex(index);
    ui->comboBox_4->setCurrentIndex(index);
    ui->comboBox_5->setCurrentIndex(index);
    ui->comboBox_7->setCurrentIndex(index);
    ui->comboBox_8->setCurrentIndex(index);
    ui->comboBox_9->setCurrentIndex(index);
}


void Widget::populateTable()
{
    // Simply rebuild from the model
    updateVisualization();
}


void Widget::refreshTable()
{
    populateTable(); // Call the populateTable function to refresh the data
}


// ADD THESE METHODS AT THE VERY END OF widget.cpp:

void Widget::setSystemOverviewModel(SystemOverviewModel* model)
{
    qDebug() << "[Widget] >>> setSystemOverviewModel called with" << model;

    system_overview_model_ = model;
    if (!system_overview_model_) {
        qWarning() << "[Widget] system_overview_model_ is NULL!";
        return;
    }

    // updateVisualization whenever hostEntities change
    connect(system_overview_model_, &SystemOverviewModel::hostEntitiesChanged,
            this, &Widget::updateVisualization);

    // Also connect highlight/measurement/scale from UI to model
    connect(ui->notificationsradio, &QRadioButton::toggled,
            this, [this](bool on){
                if (on && system_overview_model_)
                    system_overview_model_->setHighlightMode("Notifications");
            });

    connect(ui->matchesradio, &QRadioButton::toggled,
            this, [this](bool on){
                if (on && system_overview_model_) {
                    system_overview_model_->setHighlightMode("Matches");
                    system_overview_model_->updateMatches();
                }
            });

    connect(ui->measurementradio, &QRadioButton::toggled,
            this, [this](bool on){
                if (system_overview_model_) {
                    system_overview_model_->setHighlightMode(on ? "Measurement"
                                                                : "Notifications");
                }
            });

    connect(ui->measurementcombobox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index){
                if (!system_overview_model_) return;
                system_overview_model_->setSelectedMeasurement(
                    ui->measurementcombobox->itemText(index));
            });

    connect(ui->scalecombobox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index){
                if (!system_overview_model_) return;
                system_overview_model_->setSelectedScale(
                    ui->scalecombobox->itemText(index));
            });

    // "All" display combo -> model display mode
    connect(ui->allcomboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index){
                if (!system_overview_model_) return;
                QString mode = toDisplayMode(ui->allcomboBox->itemText(index));
                system_overview_model_->setDisplayAllMode(mode);
                system_overview_model_->setDisplayHostMode(mode);
                system_overview_model_->setDisplayProcessMode(mode);
            });

    // initial fill
    populateTable();
}



void Widget::onHighlightModeChanged()
{
    if (!system_overview_model_) return;
    
    QString mode;
    if (ui->notificationsradio->isChecked())
    {
        mode = "Notifications";
    }
    else if (ui->matchesradio->isChecked())
    {
        mode = "Matches";
    }
    else if (ui->measurementradio->isChecked())
    {
        mode = "Measurement";
    }
    
    system_overview_model_->setHighlightMode(mode);
    std::cout << "[Widget] Highlight mode changed to: " << mode.toStdString() << std::endl;
    
    updateVisualization();
}

void Widget::onMeasurementChanged(const QString& measurement)
{
    if (!system_overview_model_) return;
    
    system_overview_model_->setSelectedMeasurement(measurement);
    std::cout << "[Widget] Measurement changed to: " << measurement.toStdString() << std::endl;
    
    updateVisualization();
}

void Widget::onScaleChanged(const QString& scale)
{
    if (!system_overview_model_) return;
    
    system_overview_model_->setSelectedScale(scale);
    std::cout << "[Widget] Scale changed to: " << scale.toStdString() << std::endl;
    
    updateVisualization();
}

void Widget::updateVisualization()
{
    qDebug() << "[Widget] ========== UPDATING VISUALIZATION ==========";

    ui->tableWidget->setRowCount(0);

    if (!system_overview_model_) {
        qWarning() << "[Widget] updateVisualization: model is NULL";
        return;
    }

    QVariantList hosts = system_overview_model_->hostEntities();
    int row = 0;

    qDebug() << "[Widget] Found" << hosts.size() << "hosts to display";

    for (const QVariant& hostVariant : hosts)
    {
        QVariantMap host = hostVariant.toMap();
        QString hostName = host.value("name").toString();
        qDebug() << "[Widget]   Host:" << hostName;

        QVariantList participants = host.value("children").toList();
        for (const QVariant& dpVariant : participants)
        {
            QVariantMap dp = dpVariant.toMap();
            int domainId  = dp.value("domainId").toInt();
            int processId = dp.value("processId").toInt();

            QVariantList pubsubs = dp.value("children").toList();
            for (const QVariant& psVariant : pubsubs)
            {
                QVariantMap ps = psVariant.toMap();
                QString type   = ps.value("type").toString(); // Publisher or Subscriber

                QVariantList endpoints = ps.value("children").toList();
                for (const QVariant& epVariant : endpoints)
                {
                    QVariantMap ep      = epVariant.toMap();
                    QString topicName   = ep.value("topicName").toString();
                    if (topicName.isEmpty())
                        continue; // skip endpoints without topic

                    ui->tableWidget->insertRow(row);
                    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(topicName));

                    if (type == "Subscriber") {
                        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("DR"));
                        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(""));
                    } else if (type == "Publisher") {
                        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(""));
                        ui->tableWidget->setItem(row, 2, new QTableWidgetItem("DW"));
                    }

                    ui->tableWidget->setItem(row, 3,
                        new QTableWidgetItem(QString::number(domainId)));
                    ui->tableWidget->setItem(row, 4,
                        new QTableWidgetItem(QString::number(processId)));
                    ui->tableWidget->setItem(row, 5,
                        new QTableWidgetItem(hostName));

                    ++row;
                }
            }
        }
    }

    std::cout << "[Widget] ✓✓✓ Visualization updated with "
              << row << " rows ✓✓✓" << std::endl;
}
