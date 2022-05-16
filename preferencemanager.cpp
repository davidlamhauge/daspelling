#include "preferencemanager.h"
#include "ui_preferencemanager.h"

#include <QSettings>
#include <QDebug>

PreferenceManager::PreferenceManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceManager)
{
    ui->setupUi(this);

    // set comboboxes and checkbox
    QSettings settings("TeamLamhauge", "daSpelling");
    int index = settings.value("langIndex", 0).toInt();
    ui->cbLanguage->setCurrentIndex(index);
    index = settings.value("misspellIndex", 0).toInt();
    ui->cbDisspellColor->setCurrentIndex(index);
    index = settings.value("hideShuffledWord", 0).toInt();
    ui->cbShowWord->setCurrentIndex(index);

    connect(ui->cbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceManager::cbLangaugePressed);
    connect(ui->cbDisspellColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceManager::cbColorForMisspellingPressed);
    connect(ui->cbShowWord, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceManager::cbShowWordPressed);
    connect(ui->btnClose, &QPushButton::clicked, this, &PreferenceManager::saveAndClosePressed);
}

PreferenceManager::~PreferenceManager()
{
    delete ui;
}

void PreferenceManager::cbLangaugePressed(int index)
{
    QSettings settings("TeamLamhauge", "daSpelling");

    switch (index)
    {
    case 0:
        settings.setValue("lang", ":lang/lang/daspelling_da_DK");
        settings.setValue("langIndex", 0);
        break;
    case 1:
        settings.setValue("lang", ":lang/lang/daspelling_en");
        settings.setValue("langIndex", 1);
        break;
    default:
        settings.setValue("lang", ":lang/lang/daspelling_da_DK");
        settings.setValue("langIndex", 0);
        break;
    }
}

void PreferenceManager::cbColorForMisspellingPressed(int index)
{
    QSettings settings("TeamLamhauge", "daSpelling");
    settings.setValue("misspellIndex", index);
}

void PreferenceManager::cbShowWordPressed(int index)
{
    QSettings settings("TeamLamhauge", "daSpelling");
    settings.setValue("hideShuffledWord", index);
}

void PreferenceManager::saveAndClosePressed()
{
    close();
}
