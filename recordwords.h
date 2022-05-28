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

    void recWordsPosition(QPoint pos);

private:
    Ui::RecordWords *ui;
};

#endif // RECORDWORDS_H
