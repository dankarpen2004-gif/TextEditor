#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

signals:
    void findRequested(const QString &pattern,
                       QTextDocument::FindFlags flags);

private slots:
    void on_pushButtonNext_clicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H