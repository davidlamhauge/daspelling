#ifndef RECORDWORDS_H
#define RECORDWORDS_H

#include "qaudiobuffer.h"
#include <QDialog>

class QAudioRecorder;
class QAudioProbe;
class QAudioDecoder;
class QAudioBuffer;
class QMediaPlayer;

namespace Ui {
class RecordWords;
}

class RecordWords : public QDialog
{
    Q_OBJECT

public:
    explicit RecordWords(QWidget *parent = nullptr);
    ~RecordWords();

    void recWordsPosition(QPoint pos, QString lastDir);

private:
    Ui::RecordWords *ui;
    QString mLastDir = "";

    void selectFolderPressed();
    void newWordListPressed();
    void closePressed();

    void recordPressed();
    void stopRecordingPressed();
    void playSoundPressed();

    void textChanged(QString s);

    void setButtonsEnabled(bool b);

    QAudioRecorder* recorder = nullptr;
    QAudioProbe* probe = nullptr;
    QAudioDecoder* decoder = nullptr;
    QAudioBuffer buffer;
    QMediaPlayer* player = nullptr;
    QByteArray byteArr;
    QString mRecordFileName = "";
};

#endif // RECORDWORDS_H
