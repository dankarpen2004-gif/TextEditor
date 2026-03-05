#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCursor>
#include "finddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exitApp();
    void copyText();
    void cutText();
    void pasteText();
    void about();
    void find();
    void changeFont();
    void changeColor();
    void addMarkers();
    void indentText();
    void alignLeft();
    void alignCenter();
    void alignRight();

private:
    void runUnitTests();
    void runFindIntegrationTests();
    Ui::MainWindow *ui;
    QString currentFile;
    FindDialog *findDlg = nullptr;
    QTextCursor lastMatch;
};

#endif // MAINWINDOW_H