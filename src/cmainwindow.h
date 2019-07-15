#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "cgame.h"

#include "coptionsdialog.h"
#include "crecordsdialog.h"

#include <QMainWindow>

namespace Ui {
class CMainWindow;
}

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow() override;

private:
    Ui::CMainWindow *ui;
    CGame *m_game;
    COptionsDialog *m_options_dialog;
    CRecordsDialog *m_record_dialog;

private slots:
    void slotOptions();
    void slotUpdateInfo();
    void slotUndoRedo(bool, bool);
    void slotTrainingChange();
    void slotSetLanguage();
    void slotRecords();
    void slotShowResults(int index);
    void slotAbout();
    void slotResetPauseAction();
};

#endif // CMAINWINDOW_H
