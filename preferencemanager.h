#ifndef PREFERENCEMANAGER_H
#define PREFERENCEMANAGER_H

#include <QDialog>

namespace Ui {
class PreferenceManager;
}

class PreferenceManager : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceManager(QWidget *parent = nullptr);
    ~PreferenceManager();

    void prefPosition(QPoint pos);

private:
    Ui::PreferenceManager *ui;

    void cbLangaugePressed(int index);
    void cbColorForMisspellingPressed(int index);
    void cbShowWordPressed(int index);
    void cBoxKeystrokesChanged(int state);
    void saveAndClosePressed();
};

#endif // PREFERENCEMANAGER_H
