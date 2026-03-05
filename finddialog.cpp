#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::on_pushButtonNext_clicked()
{
    QTextDocument::FindFlags flags;
    if (!ui->checkBoxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->radioUp->isChecked())
        flags |= QTextDocument::FindBackward;
    emit findRequested(ui->lineEditPattern->text(), flags);
}