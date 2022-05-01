#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void init();

    // get the path to the soundfiles
    void getFileList();

    // button presses...
    void play();
    void textChanged(QString s);

    void shuffle();
    void resetList();

    void nextWord();
    void previousWord();

    void startSpelling();
    void FinishSpelling();

    //
    void prepareSpelling(int active);
    QString shuffleWord(QString s);

    // Lists with relevant data
    QString mLastDir = "";
    QStringList mFileList;
    QStringList mOrgFileList;
    QString mWord;
    QString mShuffledWord;
    QString mShuffledWordCopy;

    int mActiveSound = 0; // sound at xxx in mFileList
};
#endif // MAINWINDOW_H
