#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QString>
#include <QTableWidget>
#include <vector>
#include <string>

struct ProcessData {
    std::string host;
    std::string name;              // process name
    int id = 0;                    // PID
    double totalCPU = 0.0;
    double userCPU = 0.0;
    double kernelCPU = 0.0;
    long   physicalMemory = 0;     // RSS in kB
    long   totalMemory = 0;        // virtual memory in kB

    // internal fields for CPU calculation
    unsigned long long lastTotalJiffies = 0;
    unsigned long long lastProcJiffies  = 0;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // called from outside when a new process must be monitored
    void updateProcessTable(const ProcessData &data);

signals:
    void selectInPhysicalRequested(const QString &host, int pid, const QString &name);

public slots:
    void openProcessTable();           // Shows the window

private slots:
    void selectInPhysicalView();       // Handles "Select in Physical View" button
    void filterTable();
    void updateVisibleColumns();
    void refreshTable();               // resets filters/visibility
    void refreshProcessData();         // periodic data collection

private:
    Ui::MainWindow *ui;
    std::vector<ProcessData> processData;

    void updateTable();                // fills QTableWidget from processData
    bool processAlreadyExists(int pid) const;  // check for duplicates
};

#endif // MAINWINDOW_H
