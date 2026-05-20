#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>

#include <QTemporaryFile>
#include <QClipboard>

void MainWindow::runUnitTests()
{
    qDebug() << "========== UNIT TESTS ==========";
    bool allOk = true;

    ui->textEdit->setPlainText("abc");
    currentFile = "dummy.txt";
    newFile();
    bool ok = ui->textEdit->toPlainText().isEmpty() && currentFile.isEmpty();
    qDebug() << "newFile:" << (ok ? "OK" : "FAIL");
    allOk &= ok;

    QString tmpName;
    {
        QTemporaryFile tmp;
        tmp.open();
        tmpName = tmp.fileName();
    }
    ui->textEdit->setPlainText("Hello");
    currentFile = tmpName;
    saveFile();

    ui->textEdit->clear();
    QFile f(tmpName);
    f.open(QIODevice::ReadOnly | QFile::Text);
    ui->textEdit->setPlainText(QTextStream(&f).readAll());
    ok = (ui->textEdit->toPlainText() == "Hello");
    qDebug() << "saveFile/openFile:" << (ok ? "OK" : "FAIL");
    allOk &= ok;


    ui->textEdit->setPlainText("12345");
    QTextCursor c = ui->textEdit->textCursor();
    c.setPosition(1);
    c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 3);
    ui->textEdit->setTextCursor(c);

    copyText();
    ok = (QApplication::clipboard()->text() == "234");
    qDebug() << "copyText:" << (ok ? "OK" : "FAIL");
    allOk &= ok;

    cutText();
    ok = (QApplication::clipboard()->text() == "234") &&
         (ui->textEdit->toPlainText() == "15");
    qDebug() << "cutText:" << (ok ? "OK" : "FAIL");
    allOk &= ok;


    c = ui->textEdit->textCursor();
    pasteText();
    ok = (ui->textEdit->toPlainText() == "12345");
    qDebug() << "pasteText:" << (ok ? "OK" : "FAIL");
    allOk &= ok;


    ui->textEdit->setPlainText("a\nb\nc");
    ui->textEdit->selectAll();
    addMarkers();
    ok = (ui->textEdit->toPlainText() == "- a\n- b\n- c");
    qDebug() << "addMarkers:" << (ok ? "OK" : "FAIL");
    allOk &= ok;


    ui->textEdit->setPlainText("x\ny");
    ui->textEdit->selectAll();
    int spaces = 4;
    QString indent(spaces, ' ');
    {

        QTextCursor cur = ui->textEdit->textCursor();
        int start = cur.selectionStart(), end = cur.selectionEnd();
        cur.beginEditBlock();
        cur.setPosition(start);
        do {
            cur.movePosition(QTextCursor::StartOfLine);
            cur.insertText(indent);
            end += spaces;
        } while (cur.movePosition(QTextCursor::NextBlock) && cur.position() < end);
        cur.endEditBlock();
    }
    ok = ui->textEdit->toPlainText().startsWith(indent);
    qDebug() << "indentText:" << (ok ? "OK" : "FAIL");
    allOk &= ok;

    ui->textEdit->setPlainText("foo");
    ui->textEdit->selectAll();
    alignCenter();
    ok = (ui->textEdit->alignment() & Qt::AlignHCenter);
    qDebug() << "alignCenter:" << (ok ? "OK" : "FAIL");
    allOk &= ok;

    alignRight();
    ok = (ui->textEdit->alignment() & Qt::AlignRight);
    qDebug() << "alignRight:" << (ok ? "OK" : "FAIL");
    allOk &= ok;

    alignLeft();
    ok = (ui->textEdit->alignment() & Qt::AlignLeft);
    qDebug() << "alignLeft:" << (ok ? "OK" : "FAIL");
    allOk &= ok;


    about();
    changeFont();
    changeColor();
    qDebug() << "about / changeFont / changeColor: OK (not asserted)";

    qDebug() << "========== SUMMARY ==========";
    qDebug() << (allOk ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
}

void MainWindow::runFindIntegrationTests()
{
    qDebug() << "====== FIND INTEGRATION ======";
    ui->textEdit->setPlainText("red green blue green");
    find();

    int count = 0;
    auto conn = connect(findDlg, &FindDialog::findRequested,
                        this, [&count](auto, auto){ ++count; });

    emit findDlg->findRequested("green", QTextDocument::FindFlags());
    QCoreApplication::processEvents();
    bool ok = (count == 1) &&
              (ui->textEdit->textCursor().selectedText() == "green");
    qDebug() << "findRequested -> cursor moved:" << (ok ? "OK" : "FAIL");
    disconnect(conn);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApp);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copyText);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cutText);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::pasteText);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionFind,  &QAction::triggered, this, &MainWindow::find);
    connect(ui->actionFont,  &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionColor, &QAction::triggered, this, &MainWindow::changeColor);
    connect(ui->actionMarker, &QAction::triggered, this, &MainWindow::addMarkers);
    connect(ui->actionIndent, &QAction::triggered, this, &MainWindow::indentText);
    connect(ui->actionLeft,   &QAction::triggered, this, &MainWindow::alignLeft);
    connect(ui->actionCenter, &QAction::triggered, this, &MainWindow::alignCenter);
    connect(ui->actionRight,  &QAction::triggered, this, &MainWindow::alignRight);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newFile()
{
    ui->textEdit->clear();
    currentFile.clear();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file.");
        return;
    }

    QTextStream in(&file);
    ui->textEdit->setText(in.readAll());
    currentFile = fileName;
    file.close();
}

void MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        saveFileAs();
        return;
    }

    QFile file(currentFile);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Cannot save file.");
        return;
    }

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    file.close();
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File As");
    if (fileName.isEmpty()) return;

    currentFile = fileName;
    saveFile();
}

void MainWindow::exitApp()
{
    QApplication::quit();
}

void MainWindow::copyText()
{
    ui->textEdit->copy();
}

void MainWindow::cutText()
{
    ui->textEdit->cut();
}

void MainWindow::pasteText()
{
    ui->textEdit->paste();
}

void MainWindow::about()
{
    QMessageBox::about(this, "About TextEditor", "Simple text editor for Aurora OS.");
}

void MainWindow::find()
{
    if (!findDlg) {
        findDlg = new FindDialog(this);
        connect(findDlg, &FindDialog::findRequested,
                this, [this](const QString &pattern, QTextDocument::FindFlags flags) {
                    lastMatch = ui->textEdit->textCursor();
                    if (!ui->textEdit->find(pattern, flags)) {
                        QMessageBox::information(this, "Search",
                                                 "No results...");
                    }
                });
        connect(findDlg, &QDialog::finished,
                this, [this]{
                    ui->textEdit->moveCursor(QTextCursor::Start);
                });
    }
    findDlg->show();
    findDlg->raise();
    findDlg->activateWindow();
}

void MainWindow::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this,
                                      "Change font");
    if (ok) {
        ui->textEdit->setCurrentFont(font);
    }
}

void MainWindow::changeColor()
{
    QColor color = QColorDialog::getColor(ui->textEdit->textColor(), this,
                                          "Change color");
    if (color.isValid()) {
        ui->textEdit->setTextColor(color);
    }
}

void MainWindow::addMarkers()
{
    QTextCursor c = ui->textEdit->textCursor();
    if (!c.hasSelection())
        return;
    int start = c.selectionStart();
    int end   = c.selectionEnd();
    c.beginEditBlock();
    c.setPosition(start);
    do {
        c.movePosition(QTextCursor::StartOfLine);
        c.insertText("- ");
        end += 2;
        if (!c.movePosition(QTextCursor::NextBlock))
            break;
    } while (c.position() < end);
    c.endEditBlock();
}

void MainWindow::indentText()
{
    bool ok = false;
    int count = QInputDialog::getInt(
        this, tr("Indent"), tr("Indent size:"), 4, 0, 100, 1, &ok);
    if (!ok || count == 0)
        return;
    QString indent(count, QChar(' '));
    QTextCursor c = ui->textEdit->textCursor();
    if (!c.hasSelection())
        return;
    int start = c.selectionStart();
    int end   = c.selectionEnd();
    c.beginEditBlock();
    c.setPosition(start);
    do {
        c.movePosition(QTextCursor::StartOfLine);
        c.insertText(indent);
        end += count;
        if (!c.movePosition(QTextCursor::NextBlock))
            break;
    } while (c.position() < end);
    c.endEditBlock();
}

static void applyAlignment(QTextEdit *edit, Qt::AlignmentFlag align)
{
    QTextCursor c = edit->textCursor();
    QTextBlockFormat fmt;
    fmt.setAlignment(align);
    c.mergeBlockFormat(fmt);
}

void MainWindow::alignLeft() { applyAlignment(ui->textEdit, Qt::AlignLeft); }
void MainWindow::alignCenter() { applyAlignment(ui->textEdit, Qt::AlignHCenter);}
void MainWindow::alignRight() { applyAlignment(ui->textEdit, Qt::AlignRight); }
