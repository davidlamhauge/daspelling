#include "recordwords.h"
#include "ui_recordwords.h"

#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioDecoder>
#include <QMediaPlayer>
#include <QAudioBuffer>
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
    connect(ui->btnNewWordList, &QPushButton::clicked, this, &RecordWords::newWordListPressed);

    connect(ui->btnRecord, &QPushButton::clicked, this, &RecordWords::recordPressed);
    connect(ui->btnStopRecording, &QPushButton::clicked, this, &RecordWords::stopRecordingPressed);
    connect(ui->btnPlay, &QPushButton::clicked, this, &RecordWords::playSoundPressed);

    connect(ui->leWordPrefix, &QLineEdit::textChanged, this, &RecordWords::textChanged);

    setButtonsEnabled(false);
    ui->leWordPrefix->setEnabled(false);
    ui->btnSaveRecording->setEnabled(false);

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
    if (codecs_list.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("No audio codecs available. Exits dialog..."));
        msgBox.exec();
        close();
    }

    player = new QMediaPlayer(this, QMediaPlayer::LowLatency);
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
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setQuality(QMultimedia::NormalQuality);
    audioSettings.setChannelCount(1);
    audioSettings.setSampleRate(48000);
    audioSettings.setBitRate(16);

    recorder->setEncodingSettings(audioSettings);
    qDebug() << "bit rate " << audioSettings.bitRate() << " * sample rate: " << audioSettings.sampleRate();

    recorder->setOutputLocation(QUrl::fromLocalFile(mLastDir + "/" + ui->leWordPrefix->text() + ".wav"));
    recorder->record();
}

void RecordWords::stopRecordingPressed()
{
    recorder->stop();
    QFile file(mLastDir + "/" + ui->leWordPrefix->text() + ".wav");
    if (!file.open(QIODevice::ReadOnly))
        return;
    file.seek(34);

    qDebug() << "Bits/sample: " << file.read(2).toInt()  << " * bytes audio: " << file.bytesAvailable() - 44;
    file.close();
}

void RecordWords::playSoundPressed()
{
    player->setMedia(QUrl::fromLocalFile(mLastDir + "/" + ui->leWordPrefix->text() + ".wav"));
    player->play();
    qDebug() << "dur at play: " << player->duration();

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

