#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QAction>
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

    mPlaySound = new QAction(this);
    mPlaySound->setShortcut(Qt::ALT + Qt::Key_Space);
    addAction(mPlaySound);
    mPreviousWord = new QAction(this);
    mPreviousWord->setShortcut(Qt::ALT + Qt::Key_A);
    addAction(mPreviousWord);
    mNextWord = new QAction(this);
    mNextWord->setShortcut(Qt::ALT + Qt::Key_Z);
    addAction(mNextWord);


    init();

    setWindowTitle("daspelling - version " APP_VERSION);

    // close app
    connect(ui->btnClose, &QPushButton::clicked, this, &MainWindow::close);

    // get path to sound files
    connect(ui->btnLoadFile, &QPushButton::clicked, this, &MainWindow::getWordList);

    // shuffle or reset list. Directory order is default
    connect(ui->btnShuffle, &QPushButton::clicked, this, &MainWindow::shuffle);
    connect(ui->btnResetList, &QPushButton::clicked, this, &MainWindow::resetList);

    connect(ui->btnStartSpelling, &QPushButton::clicked, this, &MainWindow::startSpelling);
    connect(ui->btnFinishSpelling, &QPushButton::clicked, this, &MainWindow::FinishSpelling);

    connect(ui->btnNext, &QPushButton::clicked, this, &MainWindow::nextWord);
    connect(mNextWord, &QAction::triggered, this, &MainWindow::nextWord);
    connect(ui->btnPrevious, &QPushButton::clicked, this, &MainWindow::previousWord);
    connect(mPreviousWord, &QAction::triggered, this, &MainWindow::previousWord);

    connect(ui->btnPlay, &QPushButton::clicked, this, &MainWindow::play);
    connect(mPlaySound, &QAction::triggered, this, &MainWindow::play);
    connect(ui->leSpelling, &QLineEdit::textChanged, this, &MainWindow::textChanged);

    connect(ui->btnPreferences, &QPushButton::clicked, this, &MainWindow::preferencesPressed);
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

    readSettings();

    QPalette palet = ui->leSpelling->palette();
    palet.setColor(QPalette::Base, LIGHT_GREEN);
    ui->leSpelling->setPalette(palet);
}

void MainWindow::preferencesPressed()
{
    prefs = new PreferenceManager();
    prefs->exec();

    readSettings();
}

void MainWindow::readSettings()
{
    QSettings settings("TeamLamhauge", "daSpelling");
    int index = settings.value("misspellIndex", 0).toInt();
    switch (index) {
    case 0: mMisSpellColor = mRED; break;
    case 1: mMisSpellColor = mORANGE; break;
    case 2: mMisSpellColor = mPURPLE; break;
    default: mMisSpellColor = mRED; break;
    }

    mHideShuffledWord = settings.value("hideShuffledWord", 0).toInt();
    mLastDir = settings.value("last_dir", "").toString();
}

void MainWindow::getWordList()
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

    mFileList = dir.entryList(QDir::Files, QDir::Time | QDir::Reversed);
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
        palet.setColor(QPalette::Base, mMisSpellColor);
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
    if (!ui->btnNext->isEnabled())
        return;
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
    if (!ui->btnPrevious->isEnabled())
        return;
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
    ui->labShuffledWord->setVisible(true);

    switch (mHideShuffledWord) {
    case 0:
        ui->labShuffledWord->setText(mShuffledWord);
        break;
    case 1:
        ui->labShuffledWord->setText(mShuffledWord);
        if (mShuffledWord.length() < 3)
            ui->labShuffledWord->setVisible(false);
        else
            ui->labShuffledWord->setVisible(true);
        break;
    case 2:
        ui->labShuffledWord->setText(mShuffledWord);
        if (mShuffledWord.length() < 4)
            ui->labShuffledWord->setVisible(false);
        else
            ui->labShuffledWord->setVisible(true);
        break;
    case 3:
        ui->labShuffledWord->setText(mShuffledWord);
        ui->labShuffledWord->setVisible(false);
        break;
    default:
        ui->labShuffledWord->setText(mShuffledWord);
        break;
    }

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
