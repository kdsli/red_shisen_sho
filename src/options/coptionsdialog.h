#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include "clabeledslider.h"

#include <QDialog>
#include <QGroupBox>

namespace Ui {
class COptionsDialog;
}

class COptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit COptionsDialog(QWidget *parent = nullptr);
    ~COptionsDialog();

    void Show();

signals:
    void signalRequiredNewGame();
    void signalTrainingChange();
    void signalTilesChange();
    void signalBackgroundChange();
    void signalLanguageChange();

private:
    Ui::COptionsDialog *ui;
    CLabeledSlider *m_game_type_slider;
    CLabeledSlider *m_time_delay_slider;
    QStringList m_locales;

    CLabeledSlider *newSlider(const QStringList &, QGroupBox *);

    void initTilesTab();
    void initBackgroundTab();

    void fillLocales();

private slots:
    void slotAccepted();
    void slotBGRowChange(int index);
    void slotTilesRowChange(int index);

};

#endif // COPTIONSDIALOG_H
