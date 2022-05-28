#include "recordwords.h"
#include "ui_recordwords.h"

RecordWords::RecordWords(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordWords)
{
    ui->setupUi(this);
}

RecordWords::~RecordWords()
{
    delete ui;
}

void RecordWords::recWordsPosition(QPoint pos)
{
    move(pos);
}
