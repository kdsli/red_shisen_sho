#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "crecordsmanager.h"

#include "version.inc"

#include <QMessageBox>
#include <QPushButton>
#include <QKeyEvent>

CMainWindow::CMainWindow(QWidget *parent) :  QMainWindow(parent),
    ui(new Ui::CMainWindow),
    m_game(nullptr),
    m_options_dialog(nullptr),
    m_record_dialog(nullptr)
{
    ui->setupUi(this);

    settings->registerGeometry(this);

    // Инициализация менеджеров костяшек, фонов, рекордов
    tiles_manager = new CTilesManager(this);
    bg_manager = new CBackgroundManager(this);
    records_manager = new CRecordsManager(this);

    connect(ui->acExit, &QAction::triggered, this, &CMainWindow::close);
    connect(ui->acOptions, &QAction::triggered, this, &CMainWindow::slotOptions);
    connect(ui->acRecords, &QAction::triggered, this, &CMainWindow::slotRecords);
    connect(ui->acAbout, &QAction::triggered, this, &CMainWindow::slotAbout);

    ui->acUndo->setEnabled(false);
    ui->acRedo->setEnabled(false);

    slotTrainingChange();

    m_game = new CGame(this);
    setCentralWidget(m_game->board());

    // Сигналы от нас, изменяющие состояние
    connect(ui->acNewGame, &QAction::triggered, m_game, &CGame::signalNewGame);
    connect(ui->acRepeat, &QAction::triggered, m_game, &CGame::signalRepeatGame);
    connect(ui->acPause, &QAction::triggered, m_game, &CGame::signalPause);

    // Сигналы от нас, не изменяющие состояние
    connect(ui->acHint, &QAction::triggered, m_game->board(), &CBoard::slotHint);
    connect(ui->acUndo, &QAction::triggered, m_game->board(), &CBoard::slotUndo);
    connect(ui->acRedo, &QAction::triggered, m_game->board(), &CBoard::slotRedo);

    // Сигналы нам
    connect(m_game, &CGame::signalUpdateInfo, this, &CMainWindow::slotUpdateInfo);
    connect(m_game, &CGame::signalResetPauseAction, this, &CMainWindow::slotResetPauseAction);
    connect(m_game, &CGame::signalShowResult, this, &CMainWindow::slotShowResults);
    connect(m_game->board(), &CBoard::signalUndoRedo, this, &CMainWindow::slotUndoRedo);
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::slotOptions()
{
    if (!m_options_dialog) {
        m_options_dialog = new COptionsDialog(this);
        connect(m_options_dialog, &COptionsDialog::signalRequiredNewGame,
                m_game, &CGame::slotNewTypeGame);
        connect(m_options_dialog, &COptionsDialog::signalTrainingChange,
                this, &CMainWindow::slotTrainingChange);
        connect(m_options_dialog, &COptionsDialog::signalBackgroundChange,
                m_game->board(), &CBoard::slotSetBackground);
        connect(m_options_dialog, &COptionsDialog::signalLanguageChange,
                this, &CMainWindow::slotSetLanguage);
    }

    m_options_dialog->Show();
}

void CMainWindow::slotUpdateInfo()
{
    statusBar()->showMessage(m_game->gameInfo());
}

void CMainWindow::slotUndoRedo(bool is_undo, bool is_redo)
{
    ui->acUndo->setEnabled(is_undo);
    ui->acRedo->setEnabled(is_redo);
}

void CMainWindow::slotTrainingChange()
{
    auto state = settings->isTraining();
    ui->miTraining->setEnabled(state);
    ui->acHint->setVisible(state);
    ui->acUndo->setVisible(state);
    ui->acRedo->setVisible(state);
}

void CMainWindow::slotSetLanguage()
{
    QMessageBox::information(this, tr("Внимание!"),
                             tr("Изменился язык. Изменение вступят в силу после перезапуска программы."));
}

void CMainWindow::slotRecords()
{
    slotShowResults(-1);
}

// Показать результаты
void CMainWindow::slotShowResults(int index)
{
    if (!m_record_dialog) m_record_dialog = new CRecordsDialog(this);
    m_record_dialog->Show(index);
}

void CMainWindow::slotAbout()
{
    QString version, date;

    QRegularExpression re(R"==(([0-9]+\.[0-9]+\.[0-9]+(-[0-9]+)?)\s+(.+))==");
    auto match = re.match(program_version);
    if (match.hasMatch()) {
        version = match.captured(1);
        date = match.captured(3);
    } else {
        version = "0.0.0-0";
        date = "0000-00-00 00:00:00 +0000";
    }

    QMessageBox::about(this, tr("О программе"), tr("Автор: Дмитрий") + " kdsli@kdsl.ru\n\n"
                       + tr("Версия: ") + version + "\n"
                       + tr("Дата сборки: ") + date
                       + "\n\n" + tr("Исходный код:") + " https://github.com/kdsli/red_shisen_sho" );
}

// Сбросить статус pause action (из-за щелчка во время паузы)
void CMainWindow::slotResetPauseAction()
{
    ui->acPause->setChecked(false);
}

