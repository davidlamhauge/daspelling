#ifndef RECORDWORDS_H
#define RECORDWORDS_H

#include "qaudiobuffer.h"
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsItem>

class QAudioRecorder;
class QAudioBuffer;
class QGraphicsScene;
class QMediaPlayer;
class QBrush;

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

signals:
    void selectionChanged(bool b);

private:
    Ui::RecordWords *ui;
    QString mLastDir = "";

    void selectFolderPressed();
    void newWordListPressed();
    void closePressed();

    void recordPressed();
    void stopRecordingPressed();
    QGraphicsScene* drawScene(QByteArray array, QRect rect, QGraphicsScene* scene);
    void playSoundPressed();
    void stopAudio(int ms);

    void textChanged(QString s);

    void setButtonsEnabled(bool b);
    void selectionChangedSent(bool b);

    QAudioRecorder* recorder = nullptr;
    QMediaPlayer* mPlayer = nullptr;
    QAudioBuffer buffer;
    QByteArray byteArray;
    QByteArray headerArray;
    QString mRecordFileName = "";
    QGraphicsScene* scene = nullptr;
    QGraphicsItem* mRectItem = nullptr;

    void mousePressEvent(QMouseEvent* e);

    QPoint mStartPoint;
    bool mStart = false;
    bool mSoundSelected = false;
    QPoint mEndPoint;
    int mStopAt = 0;
};

#endif // RECORDWORDS_H
