#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMediaPlayer;
class QFile;
class PreferenceManager;
class RecordWords;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    Ui::MainWindow *ui;
    void init();

    void preferencesPressed();
    void recordWordsPressed();
    void readSettings();

    // get the path to the soundfiles
    void getWordList();

    // button presses...
    void play();
    void textChanged(QString s);

    void shuffle();
    void resetList();

    void nextWord();
    void previousWord();

    void startSpelling();
    void finishSpelling();

    //
    void prepareSpelling(int active);
    void updateShuffledWord(int index);
    QString shuffleWord(QString s);

    // Lists with relevant data
    QString mLanguage = "";
    QString mLastDir = "";
    QStringList mFileList;
    QStringList mOrgFileList;
    QString mWord;
    QString mShuffledWord;
    QString mShuffledWordCopy;

    QMediaPlayer *player = nullptr;

    int mActiveSound = -1;       // sound at xxx in mFileList
    int mNumberOfSounds = 0;    // sounds in the directory
    bool mRecordKeystrokes = false; // whether to save keystrokes to txt-file
    QString mKeystrokeFileName = "";

    const QColor LIGHT_GREEN = QColor(170, 230, 170);
    const QColor mRED = QColor(255, 0, 0);
    const QColor mORANGE = QColor(250, 160, 50);
    const QColor mPURPLE = QColor(250, 150, 250);
    QColor mMisSpellColor = mRED;

    int mHideShuffledWord = 0;

    PreferenceManager* prefs = nullptr;
    RecordWords* recWords = nullptr;
};
#endif // MAINWINDOW_H
