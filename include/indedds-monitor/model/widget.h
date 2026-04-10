#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>

// Forward declaration
class SystemOverviewModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    
    // Set System Overview Model
    void setSystemOverviewModel(SystemOverviewModel* model);
    
    // Methods to add data to vectors
    // Method to populate the table with vector data
    void populateTable();

public slots:
    void openWidget();
    void refreshTable();

private slots:
    void updateBoxVisibility();
    void updateComboBoxes(int index);
    void onHighlightModeChanged();
    void onMeasurementChanged(const QString& measurement);
    void onScaleChanged(const QString& scale);
    void updateVisualization();

private:
    Ui::Widget *ui;
    SystemOverviewModel* system_overview_model_ = nullptr;
};

#endif // WIDGET_H
