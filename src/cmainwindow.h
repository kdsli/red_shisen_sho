#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "cboard.h"

#include <QMainWindow>
#include "options/coptionsdialog.h"

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
    CBoard *m_board;
    COptionsDialog *m_options_dialog;

private slots:
    void slotOptions();
    void slotUpdateInfo();
    void slotUndoRedo(bool, bool);
    void slotTrainingChange();
    void slotSetLanguage();

};

#endif // CMAINWINDOW_H
