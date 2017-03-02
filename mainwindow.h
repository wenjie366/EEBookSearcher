#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "search.h"
#include <QInputDialog>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
public:
    QWidget* a;
    QLabel *label_word;
    QLabel *label_page;
    QLabel *label_row;
    QLabel *inword;
    QLabel *inpage;
    QLabel *inrow;
    char *inputstr;
    unsigned int inputpage;
    unsigned int inputrow;
    QFile *book;
    QTextStream *in;
private slots:
    void openButtonSlot();
    void SaveBottonSlot();
    void ReadBottonSlot();
    void HelpBottonSlot();
    void QuitSlot();

    void backSlot();
    void preSlot();
    void nextSlot();

    void PushSlot();

    void TraSSTBottonSlot();

    void AddBSTBottonSlot();
    void DelBSTBottonSlot();
    void TraBSTBottonSlot();
    void slotAddWord();
    void slotAddPage();
    void slotAddRow();
    void slotFinish();

    void readFile();
};
#endif // MAINWINDOW_H
