#include "recordwords.h"
#include "qevent.h"
#include "ui_recordwords.h"

#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QAudioRecorder>
#include <QSound>
#include <QAudioBuffer>
#include <QSound>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

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
//    qDebug() << "audioInputs: " << input_list;

    QStringList codecs_list = recorder->supportedAudioCodecs();
    if (!codecs_list.contains("audio/pcm"))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("No audio codecs available. Exits dialog..."));
        msgBox.exec();
        close();
    }
//    qDebug() << "supportedAudioCodecs: " << codecs_list;
    scene = new QGraphicsScene(0, 0, 1000, 100);
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
//    qDebug() << "bit rate " << audioSettings.bitRate() << " * sample rate: " << audioSettings.sampleRate();

    recorder->setOutputLocation(QUrl::fromLocalFile(mRecordFileName));
    recorder->record();
}

void RecordWords::stopRecordingPressed()
{
    if (recorder->state() != QMediaRecorder::RecordingState)
        return;
    recorder->stop();
/*
    QFile file(mRecordFileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    headerArray.clear();
    byteArray.clear();

    headerArray.append(file.read(44));
    qDebug() << "headerArray: " << headerArray.size() << " * " << headerArray;

    while(!file.atEnd())
    {
        byteArray.append(file.read(1));
    }
    qDebug() << "byteArr: " << byteArray.size() << " * bytes audio: " << file.bytesAvailable();
    file.close();

    QPen pen(Qt::blue, 1.0);
    int chunk = byteArray.size() / 1000;
    int sumUp = 0;
    int amp = 0;
    int pos = 0;
    int avg = 0;
    int hori = 0;
    for (int i = 0; i < 1000; i++)
    {
        for (hori = 0; hori < chunk; hori++)
        {
            pos = i * chunk + hori;
            amp = int(byteArray.at(pos));
            if (amp < 0)
                sumUp += -amp;
            else
                sumUp += amp;
        }
        avg = sumUp/chunk;
        scene->addLine(i, scene->height(), i, scene->height() - avg, pen);
        sumUp = 0;
    }
    ui->gvWave->setScene(scene);

    /*
    // THIS WORKS!
    QFile nyFil(mLastDir + "/" + "b_" + ui->leWordPrefix->text() + ".wav");
    qDebug() << "nyfil: " << nyFil.fileName();
    if (!nyFil.open(QIODevice::ReadWrite))
        return;
    nyFil.write(headerArray);
    nyFil.write(byteArray);
    nyFil.close();
    */
}

void RecordWords::playSoundPressed()
{
    QSound::play(mRecordFileName);
}

void RecordWords::textChanged(QString s)
{
    QDir dir = ui->labWordPath->text();
    if (s.length() > 0 && dir.exists())
        setButtonsEnabled(true);
}

void RecordWords::setButtonsEnabled(bool b)
{
    ui->btnRecord->setEnabled(b);
    ui->btnStopRecording->setEnabled(b);
    ui->btnPlay->setEnabled(b);
}

