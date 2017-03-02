#include "mainwindow.h"
#include "ui_mainwindow.h"
/*Global var*/
char *filepath = NULL;/*full path of the book file to read*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /*set the title of the MainWindow*/
    setWindowTitle("基于查找表的单词检索软件");

    /*connect the Actions with respective slots*/
    /*File bar  part*/
    connect(ui->OpenBotton, SIGNAL(triggered()), this, SLOT(openButtonSlot()));
    connect(ui->SaveBotton, SIGNAL(triggered()), this, SLOT(SaveBottonSlot()));
    connect(ui->ReadBotton, SIGNAL(triggered()), this, SLOT(ReadBottonSlot()));
    connect(ui->HelpBotton, SIGNAL(triggered()), this, SLOT(HelpBottonSlot()));
    connect(ui->QuitBotton, SIGNAL(triggered()), this, SLOT(QuitSlot()));
    /*Search Button part*/
    connect(ui->backButton,SIGNAL(clicked()),this, SLOT(backSlot()));
    connect(ui->preButton,SIGNAL(clicked()),this ,SLOT(preSlot()));
    connect(ui->nextButton,SIGNAL(clicked()),this, SLOT(nextSlot()));
    connect(ui->pushBotton, SIGNAL(clicked()), this, SLOT(PushSlot()));
    /*SSTree part*/
    connect(ui->TraSSTBotton, SIGNAL(triggered()), this, SLOT(TraSSTBottonSlot()));
    /*BSTree part*/
    connect(ui->AddBSTBotton, SIGNAL(triggered()), this, SLOT(AddBSTBottonSlot()));
    connect(ui->DelBSTBotton, SIGNAL(triggered()), this, SLOT(DelBSTBottonSlot()));
    connect(ui->TraBSTBotton, SIGNAL(triggered()), this, SLOT(TraBSTBottonSlot()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Open a book file and build the SST, BST, and text array.
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:12:23+0800
 */
void MainWindow::openButtonSlot()
{
    QByteArray ba;
    QString file;
    QFileDialog fileDialog;
    /*Dialog window to open the book file*/
    file = fileDialog.getOpenFileName(
                            this,
                            "选择文件",
                            "..\\EEbooks",
                            "TXT (*.txt)");
    if(!file.isEmpty())
    {
        /*translate the Qtring to the char array to get the file name*/
        ba = file.toLatin1();
        if(filepath != NULL)
            free(filepath);
        filepath = (char*)malloc(sizeof(char) * (strlen(ba.data())+1));
        strcpy(filepath,ba.data());
        text = QString(QLatin1String("Open File Successfully!"));
        ui->textwin->setText(text);
        /*When open new file, free the prior data strurts firstly*/
        FreeAll();
        /*build the new data structs*/
        if(Creat_SST(filepath) != OK)
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
         else
            ui->textwin->setText(text);
        if(InitBSTree(filepath) != OK)
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        if(creatTextlist(filepath, current_row) != OK)
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes); 
        /*lay the full text of the book on the @textwin*/
        book = new QFile(filepath);
        if(!book->open(QFile::ReadOnly | QFile::Text))
            return;
        in = new QTextStream(book);
        QString FileText = in->readAll();
        ui->textwin->setPlainText(FileText);
        book->close();
        in->reset();
    }
    else
        QMessageBox::information(this, "出错了", "文件打开失败！",QMessageBox::Yes , QMessageBox::Yes);
}
/**
 * Write the data information to the files
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:14:38+0800
 */
void MainWindow::SaveBottonSlot()
{
    if(WriteSSTableFile() != OK)
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
    else
    {
        text.sprintf("Save SSTable File successfully!");
        ui->textwin->setText(text);
    }
    if(WriteBSTreeFile() != OK)
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
    else
    {
        text.sprintf("Save SSTable File successfully!\nSave BSTree File successfully!");
        ui->textwin->setText(text);
    }
    if(saveTextFile() != OK)
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
}
/**
 * Write the data information to the files
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:15:01+0800
 */
void MainWindow::ReadBottonSlot()
{
    
    if(SST_head!=NULL || BST_head!=NULL)
    {
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr("读取文件"),
                                                      tr("内存数据将被覆盖\n  是否继续？"),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes)) {
            readFile();
            QMessageBox::information(this, tr("读取文件"), tr("读取成功"));
        }
        else
            QMessageBox::information(this, tr("取消读取"), tr("已经取消"));
    }
    else
        readFile();
}
/**
 * print the information about the author
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:15:19+0800
 */
void MainWindow::HelpBottonSlot()
{
    QString title, context;
    title.sprintf("关于");
    context.sprintf("\n作者：华中科技大学计算机学院计卓1501班黄文杰\n邮箱：hustyellow@qq.com\n任何问题欢迎与我联系交流\n2017.3.2");
    QMessageBox::about(this,title,context);
}
/**
 * quit from the program
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:15:50+0800
 */
void MainWindow::QuitSlot()
{
    /*free the room of all the data structs which are dynamicly allocationed*/
    FreeAll();
    /*terminate the MainWindow*/
    QApplication::quit();
}
/**
 * Write the data information to the files
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:33:53+0800
 */
void MainWindow::TraSSTBottonSlot()
{
    if(Traver_SST() != OK)
    {
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    QFile buff(BUFFER_FILENAME);
    if(!buff.open(QFile::ReadOnly | QFile::Text))
        return;
    QTextStream in(&buff);
    ui->textwin->setPlainText(in.readAll());
}
/**
 * Search the word in the ADTs and print the running time, and the text of the row in which the word appears
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:11:40+0800
 */
void MainWindow::PushSlot()
{
    QString qstr,runtimestr;
    QByteArray ba;
    char* str = NULL;
    double t1, t2, t3;
    qstr = ui->editwin->text();
    if(qstr.isEmpty())
    {
        ui->runtime->clear();
        backSlot();
        return;
    }
    ba = qstr.toLatin1();
    str = ba.data();
    /*search in the unsorted SSTable*/
    t1 = clock();
    for(int i=0;i<2000;i++)
    {
        if(Search_SST(str) != OK)
        {
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
            return;
        }
    }
    t1 = clock() - t1;
    /*Search in the sorted SSTable*/
    t2 = clock();
    for(int i=0;i<2000;i++)
    {
        if(SSearch_SST(str) != OK)
        {
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
            return;
        }
    }
    t2 = clock() - t2;
    /*Search in the BSTree*/
    t3 = clock();
    for(int i=0;i<2000;i++)
    {
        if(SearchBST(str) != OK)
        {
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
            return;
        }
    }
    t3 = clock() - t3;

    /*if founded, print the relevant information for this search process*/
    runtimestr.sprintf("顺序查找: %f\n二分查找: %f\n二叉查找: %f",t1,t2,t3);
    ui->runtime->setText(runtimestr);
    creatWpplist(Global_wp);
    QString findtext;
    findtext.sprintf("Word to Search : %s    Total: %d\n[%d].  page:%d row:%d\n %s ",Global_wp->word_name,Global_wp->count,global_wppp-global_wpparr+1,Global_wpp->page,Global_wpp->row,global_textarr[(Global_wpp->page-1)*Page_Step + Global_wpp->row - 1]);
    ui->textwin->setPlainText(findtext);
}
/**
 * Add a new word in the BSTree
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:38:34+0800
 */
void MainWindow::AddBSTBottonSlot()
{
    if(BST_head == NULL )
    {
        QMessageBox::information(this, "出错了", "BSTree not exists!\n",QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    a = new QWidget();
    a->setWindowTitle("新增单词");
    QFont font("Courier",24);
    QGridLayout *layout=new QGridLayout(a);
    inputstr = (char*)malloc(sizeof(char)*(strlen("example")+1));
    strcpy(inputstr,"example");
    inputpage = inputrow = 1;
    /*Set the labels*/
    QLabel *label_word = new QLabel(tr("Word:"));
    label_word->setFont(font);
    QLabel *label_page = new QLabel(tr("Page:"));
    label_page->setFont(font);
     QLabel *label_row = new QLabel(tr("Row:"));
    label_row->setFont(font);

    inword = new QLabel(tr("example"));
    inword->setFont(font);
    inpage = new QLabel(tr("1"));
    inpage->setFont(font);
    inrow = new QLabel(tr("1"));
    inrow->setFont(font);
    /*set the Buttons*/
    QPushButton *p1 = new QPushButton(tr("Modify"));
    QPushButton *p2 = new QPushButton(tr("Modify"));
    QPushButton *p3 = new QPushButton(tr("Modify"));
    QPushButton *p4 = new QPushButton(tr("Finish"));
    /*set the layout*/
    layout->addWidget(label_word,0,0);
    layout->addWidget(label_page,1,0);
    layout->addWidget(label_row,2,0);

    layout->addWidget(inword,0,1);
    layout->addWidget(inpage,1,1);
    layout->addWidget(inrow,2,1);

    layout->addWidget(p1,0,4);
    layout->addWidget(p2,1,4);
    layout->addWidget(p3,2,4);
    layout->addWidget(p4,3,2);
    /*connect the Buttons with the respective slots*/
    connect(p1,SIGNAL(clicked()),this,SLOT(slotAddWord()));
    connect(p2,SIGNAL(clicked()),this,SLOT(slotAddPage()));
    connect(p3,SIGNAL(clicked()),this,SLOT(slotAddRow()));
    connect(p4,SIGNAL(clicked()),this,SLOT(slotFinish()));
    a->show();
}
/**
 * input the name of the word when adding a new word in the BSTree
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:40:55+0800
 */
void MainWindow::slotAddWord()
{
    bool ok;
    QByteArray ba;
    QString name=QInputDialog::getText(this,tr("Word"),
                                       tr("Please input new word"),
                                       QLineEdit::Normal,inword->text(),&ok);
    if(ok && !name.isEmpty()){
        inword->setText(name);
        ba = name.toLatin1();
        if(inputstr != NULL)
            free(inputstr);
        inputstr = (char*)malloc(sizeof(char) * (strlen(ba.data())+1));
        strcpy(inputstr,ba.data());
    }
    a->raise();
}
/**
 * input the number of the page where the new word which is to be inserted in the BSTree appears
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:41:25+0800
 */
void MainWindow::slotAddPage()
{
    bool ok;
    unsigned int i=QInputDialog::getInt(this,tr("Page"),
                                       tr("Please input the page"),
                                       1,1,999999,1,&ok);
    if(ok){
        text.sprintf("%u",i);
        inpage->setText(text);
        inputpage = i;
    }
    a->raise();
}
/**
 * input the number of the row where the new word which is to be inserted in the BSTree appears
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:42:34+0800
 */
void MainWindow::slotAddRow()
{
    bool ok;
    unsigned int i=QInputDialog::getInt(this,tr("Row"),
                                       tr("Please input the row"),
                                       1,1,Page_Step,1,&ok);
    if(ok){
        text.sprintf("%u",i);
        inrow->setText(text);
        inputrow = i;
    }
    a->raise();
}
/**
 * Finish inputing the information of the added word and close the input window
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:42:57+0800
 */
void MainWindow::slotFinish()
{
    if(InsertBSTree(inputstr,inputpage,inputrow) != OK)
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes, QMessageBox::Yes);
    a->close ();
    a->setAttribute(Qt::WA_DeleteOnClose);
}
/**
 * Delete a word from the BSTree
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:43:47+0800
 */
void MainWindow::DelBSTBottonSlot()
{
    bool ok;
    QByteArray ba;
    QString name=QInputDialog::getText(this,tr("Word"),
                                       tr("Please input word to delete"),
                                       QLineEdit::Normal,"",&ok);
    if(ok && !name.isEmpty()){
        ba = name.toLatin1();
        if(DeleteBSTree(ba.data()) != OK)
            QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes, QMessageBox::Yes);
    }
}
/**
 * Travel through the BSTree and lay the full content on the @textwin
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:44:20+0800
 */
void MainWindow::TraBSTBottonSlot()
{
    if(TraverseBSTree() != OK)
    {
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    QFile buff(BUFFER_FILENAME);
    if(!buff.open(QFile::ReadOnly | QFile::Text))
        return;
    QTextStream in(&buff);
    ui->textwin->setPlainText(in.readAll());
}
/**
 * Stop showing the search result and be back to the Full-text window
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:44:55+0800
 */
void MainWindow::backSlot()
{
    book = new QFile(filepath);
    if(!book->open(QFile::ReadOnly | QFile::Text))
        return;
    in = new QTextStream(book);
    QString FileText = in->readAll();
    ui->textwin->setPlainText(FileText);
    book->close();
    in->reset();
    if(global_wpparr != NULL)
        free(global_wpparr);
    global_wpparr = NULL;
    global_wppp = NULL;
} 
/**
 * show the prior word_place information about the word found
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:45:58+0800
 */
void MainWindow::preSlot()
{
    /*if not searching a word*/
    if(!global_wpparr)
        return;
    /*if current word_place is the first place this word appear*/
    if(global_wppp - global_wpparr == 0)
        return;
    QString findtext;
    global_wppp--;
    Global_wpp = *global_wppp;
    findtext.sprintf("Word to Search : %s    Total: %d\n[%d].  page:%d row:%d\n %s ",Global_wp->word_name,Global_wp->count,global_wppp-global_wpparr+1,Global_wpp->page,Global_wpp->row,global_textarr[(Global_wpp->page-1)*Page_Step + Global_wpp->row - 1]);    ui->textwin->setPlainText(findtext);
}
/**
 * show the next word_place information about the word found
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:46:52+0800
 */
void MainWindow::nextSlot()
{
    /*if not searching a word*/
    if(!global_wpparr)
        return;
    /*if current word_place is the last place this word appear*/
    if(global_wppp - global_wpparr == Global_wp->count-1)
        return;
    QString findtext;
    global_wppp++;
    Global_wpp = *global_wppp;
    findtext.sprintf("Word to Search : %s    Total: %d\n[%d].  page:%d row:%d\n %s ",Global_wp->word_name,Global_wp->count,global_wppp-global_wpparr+1,Global_wpp->page,Global_wpp->row,global_textarr[(Global_wpp->page-1)*Page_Step + Global_wpp->row - 1]);    ui->textwin->setPlainText(findtext);
}
/**
 * Read files and rebuild the data structs
 * @auther   HuangWenJie
 * @datetime 2017-03-02T15:47:24+0800
 */
void MainWindow::readFile()
{
    if(ReadSSTableFile() != OK)
    {
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    if(ReadBSTreeFile() != OK)
    {
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    if(creatText() != OK)
    {
        QMessageBox::information(this, "出错了", errbuff,QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    book = new QFile(filepath);
    if(!book->open(QFile::ReadOnly | QFile::Text))
        return;
    in = new QTextStream(book);
    QString FileText = in->readAll();
    ui->textwin->setPlainText(FileText);
    book->close();
    in->reset();
}
