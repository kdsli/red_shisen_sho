#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "cboard.h"

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
    CBoard*m_board;
    COptionsDialog *m_options_dialog;
    CRecordsDialog *m_record_dialog;

    void createBoard();

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
