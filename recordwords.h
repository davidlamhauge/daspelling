#ifndef RECORDWORDS_H
#define RECORDWORDS_H

#include "qaudiobuffer.h"
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class QAudioRecorder;
class QAudioBuffer;
class QGraphicsScene;

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
    QAudioBuffer buffer;
    QByteArray byteArray;
    QByteArray headerArray;
    QString mRecordFileName = "";
    QGraphicsScene* scene = nullptr;

    void mousePressEvent(QMouseEvent* e);
};

#endif // RECORDWORDS_H
