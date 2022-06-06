#include "recordwords.h"
#include "qevent.h"
#include "ui_recordwords.h"
#include "stdio.h"

#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QAudioRecorder>
#include <QMediaPlayer>
#include <QSound>
#include <QMessageBox>

#include <QDebug>

RecordWords::RecordWords(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordWords)
{
    ui->setupUi(this);

    connect(ui->btnSelectFolder, &QPushButton::clicked, this, &RecordWords::selectFolderPressed);
    connect(ui->btnFinished, &QPushButton::clicked, this, &RecordWords::closePressed);

    connect(ui->btnRecord, &QPushButton::clicked, this, &RecordWords::recordPressed);
    connect(ui->btnStopRecording, &QPushButton::clicked, this, &RecordWords::stopRecordingPressed);
    connect(ui->btnPlay, &QPushButton::clicked, this, &RecordWords::playSoundPressed);
    connect(this, &RecordWords::selectionChanged, this, &RecordWords::selectionChangedSent);
    connect(ui->btnSaveSelection, &QPushButton::clicked, this, &RecordWords::saveSelection);

    connect(ui->leWordPrefix, &QLineEdit::textChanged, this, &RecordWords::textChanged);

    setButtonsEnabled(false);
    ui->leWordPrefix->setEnabled(false);

    recorder = new QAudioRecorder(this);

    QStringList input_list = recorder->audioInputs();
    if (input_list.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("No microphone detected. Exits dialog..."));
        msgBox.exec();
        close();
    }
    recorder->setAudioInput(input_list.at(0));

    QStringList codecs_list = recorder->supportedAudioCodecs();
    if (!codecs_list.contains("audio/pcm"))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("No audio codecs available. Exits dialog..."));
        msgBox.move(this->pos());
        msgBox.exec();
        close();
    }

    QMessageBox msgBox;
    msgBox.setText(tr("NB!!!\nWorks only on Windows!"));
    msgBox.exec();

    ui->btnSaveSelection->setEnabled(false);
    scene = new QGraphicsScene(0, 0, 1000, 100);
    mRectItem = new QGraphicsRectItem();
    mPlayer = new QMediaPlayer(this);
    setMouseTracking(true);
}

RecordWords::~RecordWords()
{
    delete ui;
}

void RecordWords::recWordsPosition(QPoint pos, QString lastDir)
{
    move(pos);
    mLastDir = lastDir;
}

void RecordWords::selectFolderPressed()
{
    QDir dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
                                                          mLastDir,
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
    if (dir.exists())
    {
        mLastDir = dir.path();
        ui->labWordPath->setText(mLastDir);
        QSettings settings("TeamLamhauge", "daSpelling");
        settings.setValue("last_dir", mLastDir);
        ui->leWordPrefix->setEnabled(true);
    }
}

void RecordWords::newWordListPressed()
{
    ui->labWordPath->setText("");
    ui->leWordPrefix->clear();
    ui->leWordPrefix->setEnabled(false);
    setButtonsEnabled(false);
}

void RecordWords::closePressed()
{
    close();
}

void RecordWords::recordPressed()
{
    scene->clear();
    ui->gvWave->setScene(scene);

    mRecordFileName = mLastDir + "/" + ui->leWordPrefix->text() + ".wav";

    QFile file(mRecordFileName);
    if (file.exists(mRecordFileName))
        file.remove(mRecordFileName);

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setQuality(QMultimedia::NormalQuality);
    audioSettings.setChannelCount(1);
    audioSettings.setSampleRate(44100);
    audioSettings.setBitRate(32);

    recorder->setEncodingSettings(audioSettings);

    recorder->setOutputLocation(QUrl::fromLocalFile(mRecordFileName));
    recorder->record();
}

void RecordWords::stopRecordingPressed()
{
    if (recorder->state() != QMediaRecorder::RecordingState)
        return;
    recorder->stop();

    QFile file(mRecordFileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    headerArray.clear();
    mDataArray.clear();

    headerArray.append(file.read(44));

//    qDebug() << "header: " << headerArray;
    while(!file.atEnd())
    {
        mDataArray.append(file.read(4));
    }
    file.close();

    scene->clear();
    scene = drawScene(mDataArray, QRect(), scene);
    ui->gvWave->setScene(scene);
    selectionChangedSent(false);
    mPlayer->setMedia(QUrl::fromLocalFile(mRecordFileName));
    playSoundPressed();
    ui->gvWave->setFocus();
}

void RecordWords::saveSelection()
{
    if (!mSoundSelected)
        return;

    int startByte = static_cast<int>((mStartPoint.x() / 1000.0) * mDataArray.size());
    startByte -= startByte % 4;
    int numBytes = static_cast<int>((mEndPoint.x() - mStartPoint.x()) / 1000.0 * mDataArray.size());
    numBytes -= numBytes % 4;

    QByteArray newArray;
    newArray.clear();
    for (int i = startByte; i < startByte + numBytes; i++)
        newArray.append(mDataArray.at(i));

    QFile nyFil(mRecordFileName);
    if (nyFil.exists(mRecordFileName))
        nyFil.remove(mRecordFileName);

    if (!nyFil.open(QIODevice::ReadWrite))
        return;

    quint32 intValue = newArray.size();
    QByteArray bytesSum = QByteArray::fromRawData(reinterpret_cast<const char *>(&intValue), sizeof (intValue));
    headerArray.replace(40, 4, bytesSum);

    nyFil.write(headerArray);
    nyFil.write(newArray);
    nyFil.close();

    mDataArray = newArray;
    mSoundSelected = false;
    scene->clear();
    drawScene(mDataArray, QRect(), scene);
    ui->gvWave->setScene(scene);
    selectionChangedSent(false);
    mPlayer->setMedia(QUrl::fromLocalFile(mRecordFileName));
    playSoundPressed();
    ui->gvWave->setFocus();
}

QGraphicsScene* RecordWords::drawScene(QByteArray array, QRect rect, QGraphicsScene *scene)
{
    QPen pen(Qt::blue, 1.0);
    QBrush brush(QColor(255, 0, 0, 128));
    int chunk = array.size() / 1000;
    int sumUp = 0;
    int amp = 0;
    int pos = 0;
    int avg = 0;
    QPoint p1;
    QPoint p2;
    for (int i = 0; i < 1000; i++)
    {
        p1 = QPoint(i, 100);
        for (int hori = 0; hori < chunk; hori++)
        {
            pos = i * chunk + hori;
            amp = int(array.at(pos));
            if (amp < 0)
                sumUp += -amp;
            else
                sumUp += amp;
        }
        avg = sumUp * 2 / chunk;
        p2 = QPoint(i, 100 - avg);
        scene->addLine(QLine(p1, p2), pen);
        sumUp = 0;
    }
    if (rect.width() > 0)
        scene->addRect(rect, QPen(Qt::red), brush);
    return scene;
}

void RecordWords::playSoundPressed()
{
    mPlayer->setMedia(QMediaContent());
    mPlayer->setMedia(QUrl::fromLocalFile(mRecordFileName));
    if (!mSoundSelected)
    {
//        qDebug() << "NOT selected..: " << mPlayer->isAvailable() << " * " << mRecordFileName;
        mPlayer->play();
    }
    else
    {
        int ms = static_cast<int>(mDataArray.size() / 88.2);
//        qDebug() << "IS selected... ms: " << ms << " start: " << ms * mStartPoint.x() / 1000;
        connect(mPlayer, &QMediaPlayer::positionChanged, this, &RecordWords::stopAudio);
        mStopAt = ms * mEndPoint.x() / 1000;
        mPlayer->setNotifyInterval(10);
        mPlayer->play();
        mPlayer->setPosition(ms * mStartPoint.x() / 1000);
    }
    ui->gvWave->setFocus();
}

void RecordWords::stopAudio(int ms)
{
    if (ms >= mStopAt)
    {
        mPlayer->stop();
        disconnect(mPlayer, &QMediaPlayer::positionChanged, this, &RecordWords::stopAudio);
    }
}

void RecordWords::textChanged(QString s)
{
    QDir dir = ui->labWordPath->text();
    if (s.length() > 0 && dir.exists())
        setButtonsEnabled(true);
    else
        setButtonsEnabled(false);
}

void RecordWords::setButtonsEnabled(bool b)
{
    ui->btnRecord->setEnabled(b);
    ui->btnStopRecording->setEnabled(b);
    ui->btnPlay->setEnabled(b);
}

void RecordWords::selectionChangedSent(bool b)
{
    ui->btnSaveSelection->setEnabled(b);
    int ms = static_cast<int>(mDataArray.size() / 88.2);
    QString txt = "";
    if (!mSoundSelected)
    {
        txt = QString::number(static_cast<qreal>(ms/1000.0));
    }
    else
    {
        qreal percent = static_cast<qreal>((mEndPoint.x() - mStartPoint.x()) / 1000.0);
        txt = QString::number(static_cast<qreal>(ms * percent / 1000.0));
    }
    ui->labLength->setText(txt + tr(" sec."));
}

void RecordWords::mousePressEvent(QMouseEvent *e)
{
    if (ui->gvWave->geometry().contains(e->pos()) && e->button() == Qt::LeftButton && !mStart)
    {
        mStartPoint = QPoint(e->pos().x() - ui->gvWave->geometry().x(), 0);
        scene->clear();
        scene = drawScene(mDataArray, QRect(mStartPoint, QPoint(mStartPoint.x() + 1, 100)), scene);
        ui->gvWave->setScene(scene);
        mStart = true;
        mSoundSelected = false;
        emit selectionChanged(mSoundSelected);
        ui->gvWave->setFocus();
    }
    else if (ui->gvWave->geometry().contains(e->pos()) && e->button() == Qt::LeftButton && mStart)
    {
        mEndPoint = QPoint(e->pos().x() - ui->gvWave->geometry().x(), 100);
        if (mStartPoint.x() >= mEndPoint.x())
        {
            mStart = false;
            scene->clear();
            scene = drawScene(mDataArray, QRect(), scene);
            ui->gvWave->setScene(scene);
            return;
        }
        mRectItem->mapRectToScene(mStartPoint.x(), 0, mEndPoint.x(), 100);
        scene->clear();
        scene = drawScene(mDataArray, QRect(mStartPoint, mEndPoint), scene);
        ui->gvWave->setScene(scene);
        mStart = false;
        mSoundSelected = true;
        emit selectionChanged(mSoundSelected);
        playSoundPressed();
        ui->gvWave->setFocus();
    }
}

