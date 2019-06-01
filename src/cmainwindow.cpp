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

#include <QDebug>

CMainWindow::CMainWindow(QWidget *parent) :  QMainWindow(parent),
    ui(new Ui::CMainWindow),
    m_board(nullptr),
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

    createBoard();
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::createBoard()
{
    m_board = new CBoard(this);
    setCentralWidget(m_board);

    m_board->createScene();

    connect(ui->acNewGame, &QAction::triggered, m_board, &CBoard::slotNewGame);
    connect(ui->acRepeat, &QAction::triggered, m_board, &CBoard::slotRepeatGame);
    connect(ui->acHint, &QAction::triggered, m_board, &CBoard::slotHelp);
    connect(ui->acPause, &QAction::triggered, m_board, &CBoard::slotPause);

    connect(ui->acUndo, &QAction::triggered, m_board, &CBoard::signalUndo);
    connect(ui->acRedo, &QAction::triggered, m_board, &CBoard::signalRedo);

    connect(m_board, &CBoard::signalUpdateInfo, this, &CMainWindow::slotUpdateInfo);
    connect(m_board, &CBoard::signalUndoRedo, this, &CMainWindow::slotUndoRedo);
    connect(m_board, &CBoard::signalShowResult, this, &CMainWindow::slotShowResults);
}

void CMainWindow::slotOptions()
{
    if (!m_options_dialog) {
        m_options_dialog = new COptionsDialog(this);
        connect(m_options_dialog, &COptionsDialog::signalRequiredNewGame,
                m_board, &CBoard::slotNewTypeGame);
        connect(m_options_dialog, &COptionsDialog::signalTrainingChange,
                this, &CMainWindow::slotTrainingChange);
        connect(m_options_dialog, &COptionsDialog::signalTilesChange,
                m_board, &CBoard::slotSetTileset);
        connect(m_options_dialog, &COptionsDialog::signalBackgroundChange,
                m_board, &CBoard::slotSetBackground);
        connect(m_options_dialog, &COptionsDialog::signalLanguageChange,
                this, &CMainWindow::slotSetLanguage);
    }

    m_options_dialog->Show();
}

void CMainWindow::slotUpdateInfo()
{
    statusBar()->showMessage(m_board->gameInfo());
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

    QRegExp re(R"==(([0-9]+\.[0-9]+\.[0-9]+-[0-9]+)\s+(.+))==");
    if (re.indexIn(program_version) == 0) {
        version = re.cap(1);
        date = re.cap(2);
    } else {
        version = "0.0.0-0";
        date = "0000-00-00 00:00:00 +0000";
    }

    QMessageBox::about(this, tr("О программе"), tr("Автор: Дмитрий") + " kdsli@kdsl.ru\n\n"
                       + tr("Версия: ") + version + "\n"
                       + tr("Дата сборки: ") + date);
}

