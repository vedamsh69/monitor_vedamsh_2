#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(QWidget *parent = nullptr);
    QString getFilterText() const;
    bool isCheckbox1Checked() const;
    bool isCheckbox2Checked() const;
    bool isCheckbox3Checked() const;

private:
    QLineEdit *lineEdit;
    QCheckBox *checkbox1;
    QCheckBox *checkbox2;
    QCheckBox *checkbox3;
    QPushButton *findButton;
    QPushButton *closeButton;
};

#endif // FINDDIALOG_H
