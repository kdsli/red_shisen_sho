#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "cboard.h"

#include <QMainWindow>
#include "coptionsdialog.h"
#include "crecordsdialog.h"

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
    CBoard*m_board;
    COptionsDialog *m_options_dialog;
    CRecordsDialog *m_record_dialog;

private slots:
    void slotOptions();
    void slotUpdateInfo();
    void slotUndoRedo(bool, bool);
    void slotTrainingChange();
    void slotSetLanguage();
    void slotRecords();
    void slotShowResults(int);
    void slotAbout();
};

#endif // CMAINWINDOW_H
