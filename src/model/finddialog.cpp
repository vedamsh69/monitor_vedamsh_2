#include <indedds-monitor/model/finddialog.h>
#include <QLabel>

FindDialog::FindDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Find");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Find:", this);
    layout->addWidget(label);

    lineEdit = new QLineEdit(this);
    layout->addWidget(lineEdit);

    checkbox1 = new QCheckBox("Case Sensitive", this);
    checkbox2 = new QCheckBox("Wrap Search", this);
    checkbox3 = new QCheckBox("Backward", this);
    layout->addWidget(checkbox1);
    layout->addWidget(checkbox2);
    layout->addWidget(checkbox3);

    findButton = new QPushButton("Find", this);
    closeButton = new QPushButton("Close", this);
    findButton->setEnabled(false);
    layout->addWidget(findButton);
    layout->addWidget(closeButton);

    connect(findButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(lineEdit, &QLineEdit::textChanged, this, [this]() {
        findButton->setEnabled(!lineEdit->text().isEmpty());
    });

    setLayout(layout);
}

QString FindDialog::getFilterText() const
{
    return lineEdit->text();
}

bool FindDialog::isCheckbox1Checked() const
{
    return checkbox1->isChecked();
}

bool FindDialog::isCheckbox2Checked() const
{
    return checkbox2->isChecked();
}

bool FindDialog::isCheckbox3Checked() const
{
    return checkbox3->isChecked();
}
