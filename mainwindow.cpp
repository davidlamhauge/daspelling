#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMessageBox>

#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QRandomGenerator>
#include <QMediaPlayer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set combobox with languages
    QSettings settings("TeamLamhauge", "daSpelling");
    int index = settings.value("langIndex", 0).toInt();
    ui->cbLanguages->setCurrentIndex(index);

    init();

    this->setWindowTitle("daspelling");

    // close app
    connect(ui->btnClose, &QPushButton::clicked, this, &MainWindow::close);

    // get path to sound files
    connect(ui->btnLoadFile, &QPushButton::clicked, this, &MainWindow::getFileList);

    // shuffle or reset list. Directory order is default
    connect(ui->btnShuffle, &QPushButton::clicked, this, &MainWindow::shuffle);
    connect(ui->btnResetList, &QPushButton::clicked, this, &MainWindow::resetList);

    connect(ui->btnStartSpelling, &QPushButton::clicked, this, &MainWindow::startSpelling);
    connect(ui->btnFinishSpelling, &QPushButton::clicked, this, &MainWindow::FinishSpelling);

    connect(ui->btnNext, &QPushButton::clicked, this, &MainWindow::nextWord);
    connect(ui->btnPrevious, &QPushButton::clicked, this, &MainWindow::previousWord);

    connect(ui->btnPlay, &QPushButton::clicked, this, &MainWindow::play);
    connect(ui->leSpelling, &QLineEdit::textChanged, this, &MainWindow::textChanged);

    connect(ui->cbLanguages, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeLanguage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    player = new QMediaPlayer;

    ui->btnLoadFile->setEnabled(true);
    ui->btnShuffle->setEnabled(false);
    ui->btnResetList->setEnabled(false);
    ui->btnStartSpelling->setEnabled(false);
    ui->btnFinishSpelling->setEnabled(false);
    ui->btnPlay->setEnabled(false);

    ui->btnNext->setEnabled(false);
    ui->btnPrevious->setEnabled(false);

    ui->leSpelling->clear();
    ui->leSpelling->setEnabled(false);

    ui->labActiveNumber->setText("-");
    ui->labMaxNumber->setText("-");
    mActiveSound = 0;
    mWord = "";
    mShuffledWord = "";
    mFileList.clear();
    ui->labShuffledWord->clear();
    ui->leSpelling->clear();

    ui->labActiveNumber->setText("-");
    ui->labMaxNumber->setText("-");

    QSettings settings("TeamLamhauge", "daSpelling");
    mLastDir = settings.value("last_dir", "").toString();

    QPalette palet = ui->leSpelling->palette();
    palet.setColor(QPalette::Base, LIGHT_GREEN);
    ui->leSpelling->setPalette(palet);
}

void MainWindow::getFileList()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open sound file"),
                                                    mLastDir,
                                                    tr("Sound Files (*.wav *.mp3)"));
    if (filename.isEmpty())
        return;

    QDir dir = QFileInfo(filename).dir();

    mLastDir = QFileInfo(filename).dir().path();
    QSettings settings("TeamLamhauge", "daSpelling");
    settings.setValue("last_dir", mLastDir);

    mFileList = dir.entryList();
    foreach (auto file, mFileList)
    {
        if (!(file.endsWith(".wav", Qt::CaseInsensitive) ||
              file.endsWith(".mp3", Qt::CaseInsensitive)))
            mFileList.removeOne(file);
    }
    mOrgFileList = mFileList;
    mNumberOfSounds = mFileList.size();

    ui->btnStartSpelling->setEnabled(true);
    ui->btnShuffle->setEnabled(true);
    ui->btnResetList->setEnabled(true);
}

void MainWindow::play()
{
    // mediaplayer
    player->setMedia(QUrl::fromLocalFile(mLastDir+ "/" + mFileList.at(mActiveSound)));
    player->play();

    ui->leSpelling->setFocus();
}

void MainWindow::textChanged(QString s)
{
    QString tmp = mShuffledWordCopy;
    for (int i = 0; i < s.length(); i++)
    {
        int index = tmp.indexOf(s.at(i));
        if (index > -1)
            tmp.remove(index, 1);
    }
    ui->labShuffledWord->setText(tmp);

    // set background to red or green
    QPalette palet = ui->leSpelling->palette();
    if (mWord.startsWith(s))
    {
        palet.setColor(QPalette::Base, LIGHT_GREEN);
        ui->leSpelling->setPalette(palet);
    }
    else
    {
        palet.setColor(QPalette::Base, Qt::red);
        ui->leSpelling->setPalette(palet);
    }
}

void MainWindow::shuffle()
{
    int shuffles = QRandomGenerator::global()->bounded(30, 70);
    int len = mFileList.count();
    for (int i = 0; i < shuffles; i++)
    {
        int ran1 = QRandomGenerator::global()->bounded(0, len);
        int ran2 = QRandomGenerator::global()->bounded(0, len);
        mFileList.move(ran1, ran2);
    }
}

void MainWindow::resetList()
{
    mFileList = mOrgFileList;
}

void MainWindow::nextWord()
{
    ui->btnPrevious->setEnabled(true);
    mActiveSound++;
    if (mActiveSound == mFileList.size() - 1)
        ui->btnNext->setEnabled(false);
    ui->leSpelling->clear();
    prepareSpelling(mActiveSound);
    ui->leSpelling->setFocus();
}

void MainWindow::previousWord()
{
    ui->btnNext->setEnabled(true);
    mActiveSound--;
    if (mActiveSound == 0)
        ui->btnPrevious->setEnabled(false);
    ui->leSpelling->clear();
    prepareSpelling(mActiveSound);
    ui->leSpelling->setFocus();
}

void MainWindow::prepareSpelling(int active)
{
    mWord = mFileList.at(active);
    // allow '.' in words
    mWord.remove(mWord.lastIndexOf("."),4);
    if (mWord.length() == 1)
        mShuffledWord = mWord;
    else
    {
        do  // make sure shuffled word is different
        {
            mShuffledWord = shuffleWord(mWord);
        } while (mShuffledWord == mWord);
    }

    mShuffledWordCopy = mShuffledWord;

    ui->labShuffledWord->setText(mShuffledWord);

    ui->labMaxNumber->setText(QString::number(mNumberOfSounds));
    ui->labActiveNumber->setText(QString::number(mActiveSound + 1));
}

QString MainWindow::shuffleWord(QString s)
{
    int rota = QRandomGenerator::global()->bounded(30, 80);
    for (int i = 0; i < rota; i++)
    {
        int index = QRandomGenerator::global()->bounded(0, s.length());
        QString tmp = s.at(index);
        s.remove(index, 1);
        s = tmp + s;
    }
    return s;
}

void MainWindow::startSpelling()
{
    ui->btnLoadFile->setEnabled(false);
    ui->btnShuffle->setEnabled(false);
    ui->btnResetList->setEnabled(false);
    ui->btnStartSpelling->setEnabled(false);
    ui->btnFinishSpelling->setEnabled(true);
    ui->btnPlay->setEnabled(true);

    ui->btnNext->setEnabled(true);
    ui->btnPrevious->setEnabled(false);

    ui->leSpelling->setEnabled(true);

    prepareSpelling(mActiveSound);
    ui->leSpelling->setFocus();
}

void MainWindow::FinishSpelling()
{
    init();
}

void MainWindow::changeLanguage(int index)
{
    QSettings settings("TeamLamhauge", "daSpelling");

    switch (index)
    {
    case 0:
        settings.setValue("lang", ":lang/lang/daspelling_da_DK");
        settings.setValue("langIndex", 0);
        break;
    case 1:
        settings.setValue("lang", ":lang/lang/daspelling_en");
        settings.setValue("langIndex", 1);
        break;
    default:
        settings.setValue("lang", ":lang/lang/daspelling_da_DK");
        settings.setValue("langIndex", 0);
        break;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Language change will happen after app restart"));
    msgBox.exec();
}

