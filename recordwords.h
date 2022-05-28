#ifndef RECORDWORDS_H
#define RECORDWORDS_H

#include <QDialog>

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

    void textChanged(QString s);

    void setButtonsEnabled(bool b);
};

#endif // RECORDWORDS_H
