#ifndef RECORDWORDS_H
#define RECORDWORDS_H

#include <QWidget>

namespace Ui {
class RecordWords;
}

class RecordWords : public QWidget
{
    Q_OBJECT

public:
    explicit RecordWords(QWidget *parent = nullptr);
    ~RecordWords();

private:
    Ui::RecordWords *ui;
};

#endif // RECORDWORDS_H
