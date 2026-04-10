#include <indedds-monitor/model/mainwindow.h>
#include "ui_mainwindow.h"
#include <indedds-monitor/model/finddialog.h>

#include <QHeaderView>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>
#include <QHostInfo>
#include <QDebug>

#include <fstream>
#include <sstream>
#include <unistd.h>      // sysconf, getpid
#include <sys/types.h>   // pid_t
#include <signal.h>      // kill

// ---------- helpers for /proc parsing (Linux only) ----------

namespace {

// Check if process exists using /proc filesystem
bool processExists(pid_t pid)
{
    std::stringstream path;
    path << "/proc/" << pid << "/stat";
    std::ifstream file(path.str());
    return file.good();
}

// total CPU jiffies (user+nice+system+idle+iowait+irq+softirq+steal)
unsigned long long readTotalCpuJiffies()
{
    std::ifstream file("/proc/stat");
    if (!file.is_open())
        return 0;

    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    if (!(file >> cpu >> user >> nice >> system >> idle
               >> iowait >> irq >> softirq >> steal)) {
        return 0;
    }
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

// per‑process CPU jiffies (utime+stime) and memory (vsize,rss) in kB
bool readProcessInfo(pid_t pid,
                     unsigned long long &procJiffies,
                     long &vmSizeKB,
                     long &rssKB)
{
    std::stringstream path;
    path << "/proc/" << pid << "/stat";

    std::ifstream statFile(path.str());
    if (!statFile.is_open())
        return false;

    int pidField;
    std::string comm;
    char state;
    long dummyLong;
    unsigned long utime, stime;
    long cutime, cstime;
    unsigned long vsize;
    long rss;

    statFile >> pidField >> comm >> state;

    // skip fields 4–13
    for (int i = 0; i < 7; ++i)
        statFile >> dummyLong;

    statFile >> utime >> stime >> cutime >> cstime;

    // skip fields 17–22
    for (int i = 0; i < 6; ++i)
        statFile >> dummyLong;

    statFile >> vsize >> rss;

    if (!statFile)
        return false;

    long pageSizeKB = sysconf(_SC_PAGESIZE) / 1024;
    vmSizeKB = static_cast<long>(vsize / 1024);
    rssKB    = static_cast<long>(rss * pageSizeKB);
    procJiffies = static_cast<unsigned long long>(utime) +
                  static_cast<unsigned long long>(stime);
    return true;
}

std::string readProcessName(pid_t pid)
{
    std::stringstream path;
    path << "/proc/" << pid << "/comm";
    std::ifstream commFile(path.str());
    if (!commFile.is_open())
        return {};

    std::string name;
    std::getline(commFile, name);
    return name;
}

} // namespace

// ---------- MainWindow implementation ----------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updateTable();

    // Initialize the combo box with checkboxes for each metric column
    auto *model = new QStandardItemModel(this);
    QStringList columnNames =
        {"Total CPU", "User CPU", "Kernel CPU", "Physical Memory", "Total Memory"};
    for (const QString &columnName : columnNames) {
        auto *item = new QStandardItem(columnName);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Checked, Qt::CheckStateRole);
        model->appendRow(item);
    }
    ui->comboBox->setModel(model);

    ui->tableWidget->setSortingEnabled(true);

    // Connect buttons to correct slots
    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::selectInPhysicalView);  // FIXED
    connect(ui->pushButton_2, &QPushButton::clicked,
            this, &MainWindow::filterTable);
    connect(ui->pushButton_Refresh, &QPushButton::clicked,
            this, &MainWindow::refreshTable);
    connect(model, &QStandardItemModel::dataChanged,
            this, &MainWindow::updateVisibleColumns);

    // Periodic refresh of process metrics every 1 second
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshProcessData);
    timer->start(1000);

    updateVisibleColumns();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTable()
{
    ui->tableWidget->setRowCount(static_cast<int>(processData.size()));
    ui->tableWidget->setColumnCount(8);

    QStringList headers = {"Host", "ID", "Process Name",
                           "Total CPU", "User CPU", "Kernel CPU",
                           "Physical Memory", "Total Memory"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    ui->tableWidget->setColumnWidth(0, 135); // Host
    ui->tableWidget->setColumnWidth(1, 70);  // ID
    ui->tableWidget->setColumnWidth(2, 135); // Process Name
    ui->tableWidget->setColumnWidth(3, 103); // Total CPU
    ui->tableWidget->setColumnWidth(4, 103); // User CPU
    ui->tableWidget->setColumnWidth(5, 102); // Kernel CPU
    ui->tableWidget->setColumnWidth(6, 135); // Physical Memory
    ui->tableWidget->setColumnWidth(7, 135); // Total Memory

    auto makeCpuItem = [](double value) {
        auto *item = new QTableWidgetItem(QString::number(value, 'f', 1) + "%");
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return item;
    };
    auto makeMemItem = [](long valueKB) {
        double mb = valueKB / 1024.0;
        auto *item = new QTableWidgetItem(QString::number(mb, 'f', 1) + " MB");
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return item;
    };

    for (std::size_t row = 0; row < processData.size(); ++row) {
        const auto &d = processData[row];
        int r = static_cast<int>(row);

        ui->tableWidget->setItem(r, 0,
            new QTableWidgetItem(QString::fromStdString(d.host)));
        ui->tableWidget->setItem(r, 1,
            new QTableWidgetItem(QString::number(d.id)));
        ui->tableWidget->setItem(r, 2,
            new QTableWidgetItem(QString::fromStdString(d.name)));

        ui->tableWidget->setItem(r, 3, makeCpuItem(d.totalCPU));
        ui->tableWidget->setItem(r, 4, makeCpuItem(d.userCPU));
        ui->tableWidget->setItem(r, 5, makeCpuItem(d.kernelCPU));
        ui->tableWidget->setItem(r, 6, makeMemItem(d.physicalMemory));

        auto *totalMemoryItem = makeMemItem(d.totalMemory);
        totalMemoryItem->setData(Qt::UserRole,
                                 static_cast<qint64>(d.totalMemory));
        ui->tableWidget->setItem(r, 7, totalMemoryItem);
    }
}

bool MainWindow::processAlreadyExists(int pid) const
{
    for (const auto &p : processData) {
        if (p.id == pid)
            return true;
    }
    return false;
}

void MainWindow::updateProcessTable(const ProcessData &data)
{
    // Prevent duplicate entries
    if (processAlreadyExists(data.id))
        return;

    processData.push_back(data);
    // actual metrics will be filled on next refreshProcessData()
}

void MainWindow::updateVisibleColumns()
{
    auto *model = qobject_cast<QStandardItemModel *>(ui->comboBox->model());
    if (!model)
        return;

    // Metric columns start at index 3 (after Host, ID, Process Name)
    for (int column = 3; column < ui->tableWidget->columnCount(); ++column) {
        QStandardItem *item = model->item(column - 3);
        bool isChecked = item && item->checkState() == Qt::Checked;
        ui->tableWidget->setColumnHidden(column, !isChecked);
    }
}

void MainWindow::openProcessTable()
{
    // Just show the window - no dialog
    show();
}

void MainWindow::selectInPhysicalView()
{
    // Handle "Select in Physical View" button click
    int row = ui->tableWidget->currentRow();
    if (row < 0 || row >= static_cast<int>(processData.size())) {
        QMessageBox::information(this, "No Selection",
                                 "Please select a process row first.");
        return;
    }

    const auto &d = processData[static_cast<std::size_t>(row)];
    
    // Emit signal to physical view (connect this in your main application)
    emit selectInPhysicalRequested(QString::fromStdString(d.host), 
                                   d.id, 
                                   QString::fromStdString(d.name));
    
    // Show confirmation message
    QMessageBox::information(this, "Select in Physical View",
                            QString("Selected:\nHost: %1\nPID: %2\nName: %3")
                            .arg(QString::fromStdString(d.host))
                            .arg(d.id)
                            .arg(QString::fromStdString(d.name)));
}

void MainWindow::filterTable()
{
    FindDialog findDialog(this);
    if (findDialog.exec() != QDialog::Accepted)
        return;

    QString filterText = findDialog.getFilterText();
    bool caseSensitive = findDialog.isCheckbox1Checked();
    Qt::CaseSensitivity sensitivity =
            caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        bool rowMatchesFilter = false;

        for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
            if (ui->tableWidget->isColumnHidden(column))
                continue;

            QTableWidgetItem *item = ui->tableWidget->item(row, column);
            if (!item)
                continue;

            if (item->text().contains(filterText, sensitivity)) {
                rowMatchesFilter = true;
                break;
            }
        }

        ui->tableWidget->setRowHidden(row, !rowMatchesFilter);
    }
}

void MainWindow::refreshTable()
{
    qDebug() << "=== REFRESH BUTTON CLICKED ===";
    qDebug() << "Before cleanup: processData.size() =" << processData.size();
    
    // Remove dead processes from the vector
    auto it = std::remove_if(processData.begin(), processData.end(),
                             [](const ProcessData &d) {
                                 bool alive = processExists(static_cast<pid_t>(d.id));
                                 qDebug() << "Checking PID" << d.id 
                                          << QString::fromStdString(d.name) 
                                          << "- Alive:" << alive;
                                 return !alive;
                             });
    
    processData.erase(it, processData.end());
    
    qDebug() << "After cleanup: processData.size() =" << processData.size();

    // Update the table to reflect current process list
    updateTable();

    // Show all rows (reset filter)
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        ui->tableWidget->setRowHidden(row, false);

    // Apply column visibility from combo box
    auto *model = qobject_cast<QStandardItemModel *>(ui->comboBox->model());
    if (!model)
        return;

    for (int column = 3; column < ui->tableWidget->columnCount(); ++column) {
        QStandardItem *item = model->item(column - 3);
        bool isChecked = item && item->checkState() == Qt::Checked;
        ui->tableWidget->setColumnHidden(column, !isChecked);
    }
}

// called every second by QTimer: update CPU & memory, remove dead processes
void MainWindow::refreshProcessData()
{
    if (processData.empty())
        return;

    unsigned long long totalJiffiesNow = readTotalCpuJiffies();
    if (totalJiffiesNow == 0)
        return;

    // Use index-based loop to safely erase while iterating
    for (std::size_t i = 0; i < processData.size(); ) {
        auto &data = processData[i];

        // Check if process still exists
        if (!processExists(static_cast<pid_t>(data.id))) {
            qDebug() << "Auto-removing dead process: PID" << data.id 
                     << QString::fromStdString(data.name);
            processData.erase(processData.begin() + static_cast<long>(i));
            continue;  // don't increment i
        }

        unsigned long long procJiffiesNow = 0;
        long vmSizeKB = 0;
        long rssKB = 0;

        if (!readProcessInfo(static_cast<pid_t>(data.id),
                             procJiffiesNow, vmSizeKB, rssKB)) {
            // Process exists but /proc read failed temporarily
            ++i;
            continue;
        }

        // Update process name
        std::string pname = readProcessName(static_cast<pid_t>(data.id));
        if (!pname.empty())
            data.name = pname;

        if (data.lastTotalJiffies != 0 && data.lastProcJiffies != 0) {
            unsigned long long totalDelta =
                    totalJiffiesNow - data.lastTotalJiffies;
            unsigned long long procDelta =
                    procJiffiesNow - data.lastProcJiffies;

            double cpuPercent = 0.0;
            if (totalDelta > 0) {
                cpuPercent = 100.0 * procDelta /
                             static_cast<double>(totalDelta);
            }

            data.totalCPU  = cpuPercent;
            data.userCPU   = cpuPercent / 2.0;
            data.kernelCPU = cpuPercent - data.userCPU;
        }

        data.totalMemory    = vmSizeKB;
        data.physicalMemory = rssKB;

        data.lastTotalJiffies = totalJiffiesNow;
        data.lastProcJiffies  = procJiffiesNow;

        ++i;
    }

    updateTable();
}
