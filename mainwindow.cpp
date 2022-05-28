#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QRandomGenerator>
#include <QMediaPlayer>
#include <QKeyEvent>
#include <QTranslator>
#include <QScreen>
#include <QDateTime>
#include <QTextStream>
#include <QAudioRecorder>

#include "preferencemanager.h"
#include "recordwords.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->leSpelling->installEventFilter(this);

    // position where we left it
    QSize scr = QGuiApplication::primaryScreen()->availableSize();
    QSettings settings("TeamLamhauge", "daSpelling");
    resize(settings.value("winSize", QSize(620, 432)).toSize());
    move(settings.value("winPos", QPoint(scr.width()/2 - 310, scr.height()/2 - 216)).toPoint());

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
    connect(ui->btnFinishSpelling, &QPushButton::clicked, this, &MainWindow::finishSpelling);

    connect(ui->btnNext, &QPushButton::clicked, this, &MainWindow::nextWord);
    connect(ui->btnPrevious, &QPushButton::clicked, this, &MainWindow::previousWord);

    connect(ui->btnPlay, &QPushButton::clicked, this, &MainWindow::play);
    connect(ui->leSpelling, &QLineEdit::textChanged, this, &MainWindow::textChanged);

    connect(ui->btnPreferences, &QPushButton::clicked, this, &MainWindow::preferencesPressed);
}

MainWindow::~MainWindow()
{
    // set settings for next session
    QSettings settings("TeamLamhauge", "daSpelling");
    settings.setValue("winSize", size());
    settings.setValue("winPos", pos());

    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->leSpelling)
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent* k = static_cast<QKeyEvent*>(e);
            if (k->key() == Qt::Key_Left)
            {
                previousWord();
                return true;
            }
            if (k->key() == Qt::Key_Right)
            {
                nextWord();
                return true;
            }
            if (k->key() == Qt::Key_Down)
            {
                play();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, e);
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
    mActiveSound = -1;
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

    QAudioRecorder *recorder = new QAudioRecorder(this);
    QStringList codecs_list = recorder->supportedAudioCodecs();

    for( int i=0 ; i<codecs_list.count() ; i++ )
    {
        qDebug() << codecs_list[i];
    }
}

void MainWindow::preferencesPressed()
{
    QSettings settings("TeamLamhauge", "daSpelling");
    QString tmp = settings.value("lang").toString();
    QString helpLine = ui->labShuffledWord->text();

    prefs = new PreferenceManager();
    prefs->prefPosition(this->pos());
    prefs->exec();

    readSettings();

    // need to change UI language
    if (tmp != mLanguage)
    {
        QTranslator* translator = new QTranslator(this);
        translator->load(mLanguage);
        QCoreApplication::installTranslator(translator);
        ui->retranslateUi(this);
    }

    updateShuffledWord(mHideShuffledWord);
    ui->labShuffledWord->setText(helpLine);
    ui->leSpelling->setFocus();
}

void MainWindow::recordWordsPressed()
{
    recWords = new RecordWords();
    recWords->recWordsPosition(this->pos());
    recWords->exec();
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
    mLanguage = settings.value("lang", ":lang/lang/daspelling_da_DK").toString();
    mRecordKeystrokes = settings.value("recordKeystrokes", false).toBool();
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
    mActiveSound = 0;
    mOrgFileList = mFileList;
    mNumberOfSounds = mFileList.size();

    ui->btnStartSpelling->setEnabled(true);
    ui->btnShuffle->setEnabled(true);
    ui->btnResetList->setEnabled(true);
}

void MainWindow::play()
{
    if (mActiveSound < 0)
        return;
    // mediaplayer
    player->setMedia(QUrl::fromLocalFile(mLastDir+ "/" + mFileList.at(mActiveSound)));
    player->play();

    if (mRecordKeystrokes)
    {
        QFile fil(mKeystrokeFileName);
        if (fil.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&fil);
            out << "-> " << tr("Play:") << " " << mWord << " <-\n";
            fil.close();
        }
    }

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

    if (mRecordKeystrokes)
    {
        QFile fil(mKeystrokeFileName);
        if (fil.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&fil);
            out << ui->leSpelling->text() << "\n";
            fil.close();
        }
    }

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
    if (!ui->btnNext->isEnabled() || mActiveSound < 0)
        return;
    ui->btnPrevious->setEnabled(true);
    mActiveSound++;
    if (mActiveSound == mFileList.size() - 1)
        ui->btnNext->setEnabled(false);
    ui->leSpelling->clear();
    prepareSpelling(mActiveSound);

    if (mRecordKeystrokes)
    {
        QFile fil(mKeystrokeFileName);
        if (fil.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&fil);
            out << "-> " << mWord << " <-\n";
            fil.close();
        }
    }
    ui->leSpelling->setFocus();
}

void MainWindow::previousWord()
{
    if (!ui->btnPrevious->isEnabled() || mActiveSound < 0)
        return;
    ui->btnNext->setEnabled(true);
    mActiveSound--;
    if (mActiveSound == 0)
        ui->btnPrevious->setEnabled(false);
    ui->leSpelling->clear();
    prepareSpelling(mActiveSound);

    if (mRecordKeystrokes)
    {
        QFile fil(mKeystrokeFileName);
        if (fil.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&fil);
            out << "-> " << mWord << " <-\n";
            fil.close();
        }
    }
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
        if (mWord.contains(Qt::Key_Space))
        {
            do  // make sure shuffled word is different and spaces is inside shuffledword
            {
                mShuffledWord = shuffleWord(mWord);
            } while (mShuffledWord == mWord
                     || mShuffledWord.startsWith(Qt::Key_Space)
                     || mShuffledWord.endsWith(Qt::Key_Space));
        }
        else
        {
            do  // make sure shuffled word is different
            {
                mShuffledWord = shuffleWord(mWord);
            } while (mShuffledWord == mWord);
        }
    }

    mShuffledWordCopy = mShuffledWord;
    ui->labShuffledWord->setVisible(true);

    updateShuffledWord(mHideShuffledWord);

    ui->labMaxNumber->setText(QString::number(mNumberOfSounds));
    ui->labActiveNumber->setText(QString::number(mActiveSound + 1));
}

void MainWindow::updateShuffledWord(int index)
{
    switch (index) {
    case 0:
        ui->labShuffledWord->setText(mShuffledWord);
        ui->labShuffledWord->setVisible(true);
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

    if (mRecordKeystrokes)
    {
        QString name = qgetenv("USER");
        if (name.isEmpty())
            name = qgetenv("USERNAME");
        QString stamp = QDateTime().currentDateTime().toString("yyyyMMdd_hh_mm_ss");
        mKeystrokeFileName = mLastDir + "/" + name + "_" + stamp + ".txt";
        QFile fil(mKeystrokeFileName);
        if (fil.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&fil);
            out << "-> " << mWord << " <-\n";
            fil.close();
        }
    }

    ui->leSpelling->setFocus();
}

void MainWindow::finishSpelling()
{
    init();
}
