#include "recordwords.h"
#include "ui_recordwords.h"

RecordWords::RecordWords(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordWords)
{
    ui->setupUi(this);
}

RecordWords::~RecordWords()
{
    delete ui;
}
