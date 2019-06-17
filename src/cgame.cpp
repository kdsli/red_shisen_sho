#include "cgame.h"

#include <common_types.h>
#include <ctilesmanager.h>
#include <crecordsmanager.h>

#include <QApplication>

#include <QDebug>

CGame::CGame(QObject *parent) : QStateMachine(parent),
    m_board(nullptr),
    m_scene(nullptr),
    m_field(nullptr)
{
    m_field = new CField(this);
    m_scene = new CScene(m_field, this);
    m_board = new CBoard(m_scene, m_field, qApp->activeWindow());

    // Заполним массив типов полей
    m_field_types.insert(fz14x6, {14, 6, 4});
    m_field_types.insert(fz16x9, {16, 9, 4});
    m_field_types.insert(fz18x8, {18, 8, 4});
    m_field_types.insert(fz24x12, {24, 12, 8});
    m_field_types.insert(fz26x14, {26, 14, 8});
    m_field_types.insert(fz30x16, {30, 16, 12});

    // Состояния автомата
    addState(&m_game);
    addState(&m_new_game);
    addState(&m_repeat_game);
    addState(&m_empty);
    addState(&m_victory);
    addState(&m_not_variants);
    addState(&m_demonstration);
    addState(&m_pause);

    // Добавим возможные переходы

    // В основную игру
    m_new_game.addTransition(this, &CGame::signalDoGame, &m_game);
    m_repeat_game.addTransition(this, &CGame::signalDoGame, &m_game);

    // В новую игру можно войти из следующих состояний
    m_game.addTransition(this, &CGame::signalNewGame, &m_new_game);
    m_game.addTransition(this, &CGame::signalRepeatGame, &m_repeat_game);
    m_victory.addTransition(this, &CGame::signalNewGame, &m_new_game);
    m_victory.addTransition(this, &CGame::signalRepeatGame, &m_repeat_game);
    m_empty.addTransition(this, &CGame::signalNewGame, &m_new_game);
    m_empty.addTransition(this, &CGame::signalRepeatGame, &m_repeat_game);
    m_not_variants.addTransition(this, &CGame::signalNewGame, &m_new_game);
    m_not_variants.addTransition(this, &CGame::signalRepeatGame, &m_repeat_game);
    m_demonstration.addTransition(this, &CGame::signalNewGame, &m_new_game);
    m_demonstration.addTransition(this, &CGame::signalRepeatGame, &m_repeat_game);

    // Из игры в состояние паузы и обратно
    m_game.addTransition(this, &CGame::signalPause, &m_pause);
    m_pause.addTransition(this, &CGame::signalPause, &m_game);
    // По щелчку мыши из board
    m_pause.addTransition(m_board, &CBoard::signalClick, &m_game);

    // Из игры - в победу и далее в пустое состояние
    m_game.addTransition(m_board, &CBoard::signalVictory, &m_victory);
    m_victory.addTransition(m_board, &CBoard::signalClick, &m_empty);

    // Из игры - в поражение и далее в демонстрацию и далее в пустое поле
    m_game.addTransition(m_board, &CBoard::signalNotVariant, &m_not_variants);
    m_not_variants.addTransition(m_board, &CBoard::signalClick, &m_demonstration);
    m_demonstration.addTransition(m_board, &CBoard::signalClick, &m_empty);

    // Обработчики состояний

    connect(&m_game, &QState::entered, this, &CGame::slotGameEntered);
    connect(&m_game, &QState::exited, this, &CGame::slotGameExited);
    connect(&m_new_game, &QState::entered, this, &CGame::slotNewGameEntered);
    connect(&m_repeat_game, &QState::entered, this, &CGame::slotRepeatGameEntered);
    connect(&m_pause, &QState::entered, this, &CGame::slotPauseEntered);
    connect(&m_pause, &QState::exited, this, &CGame::slotPauseExited);
    connect(&m_victory, &QState::entered, this, &CGame::slotVictoryEntered);
    connect(&m_victory, &QState::exited, this, &CGame::slotVictoryExited);
    connect(&m_not_variants, &QState::entered, this, &CGame::slotNotVariantEntered);
    connect(&m_not_variants, &QState::exited, this, &CGame::slotNotVariantExited);
    connect(&m_empty, &QState::entered, this, &CGame::slotEmptyEntered);
    connect(&m_demonstration, &QState::entered, this, &CGame::slotDemonstrationEntered);
    connect(&m_demonstration, &QState::exited, this, &CGame::slotDemonstrationExited);

    signalNewGame();

    setInitialState(&m_new_game);
    start();
}

// ------------------------------------------------------------------------------------------------
QString CGame::gameInfo()
{
    return tr("Ваше время: ") + getSecondsString(m_seconds)
            + tr("  Удалено: ") + m_field->tilesInfo();
}

// ------------------------------------------------------------------------------------------------
// Игра новыми костяшками
void CGame::slotNewTypeGame()
{
    tiles_manager->initCurrentFile();

    emit signalNewGame();
}

// ------------------------------------------------------------------------------------------------
// События таймера
void CGame::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_game_timer) {
        if (configuration().contains(&m_game)) {
            ++m_seconds;
            emit signalUpdateInfo();
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Вход в состояние новой игры
void CGame::slotNewGameEntered()
{
    // Новое игровое поле
    m_field->newGame(m_field_types[settings->currentGameType()].x,
            m_field_types[settings->currentGameType()].y,
            m_field_types[settings->currentGameType()].count);

    doNewGame();
}

// ------------------------------------------------------------------------------------------------
// Вход в состяние повтора игры
void CGame::slotRepeatGameEntered()
{
    // Вернули сохраненное поле
    m_field->restoreField();

    doNewGame();
}

// ------------------------------------------------------------------------------------------------
// Настройки для новой игры
void CGame::doNewGame()
{
    m_seconds = 0;

    // Заполним сцену новыми значениями
    m_scene->newGame();
    m_board->recalcView();
    m_board->newGame();

    emit signalUpdateInfo();

    // Переходим в состояние игры
    if (isRunning()) emit signalDoGame();
}

// ------------------------------------------------------------------------------------------------
// Вход в состояние игры
void CGame::slotGameEntered()
{
    // Запускаем таймер игры
    m_game_timer = startTimer(1000);

    // Доска должна знать, в каком состянии сейчас игра
    m_board->setGameState(gsGame);
}

// ------------------------------------------------------------------------------------------------
// Завершение игры
void CGame::slotGameExited()
{
    Q_ASSERT(m_game_timer != -1);
    killTimer(m_game_timer);
    m_game_timer = -1;
}

// ------------------------------------------------------------------------------------------------
// Вошли в режим паузы
void CGame::slotPauseEntered()
{
    m_board->setGameState(gsPause);
    m_board->showMessage(tr("Игра приостановлена") + "\n" + tr("Щелкните для продолжения"), true);
}

// ------------------------------------------------------------------------------------------------
// Вышли из паузы
void CGame::slotPauseExited()
{
    m_board->hideMessage(true);

    emit signalResetPauseAction();
}

// ------------------------------------------------------------------------------------------------
// Вошли в режим победы
void CGame::slotVictoryEntered()
{
    m_board->setGameState(gsVictory);

    QString s = tr("Вы выиграли за ") + getSecondsString(m_seconds) + "!";
    if (settings->isTraining() && !m_board->m_is_cunning)
        s += "\n" + tr("Вы не пользовались подсказками, пожалуй, мы занесем ваш результат в таблицу рекордов");
    m_board->showMessage(s, false);
}

// ------------------------------------------------------------------------------------------------
// Ушли из состояния победы
void CGame::slotVictoryExited()
{
    m_board->hideMessage(false);
    if (!settings->isTraining() || !m_board->m_is_cunning) {
        auto result = records_manager->checkRecord(m_seconds);
        emit signalShowResult(result);
    }
}

// ------------------------------------------------------------------------------------------------
// Вошли в режим Нет варианта
void CGame::slotNotVariantEntered()
{
    m_board->setGameState(gsNotVariants);

    QString s = tr("Игра зашла в тупик.") + "\n";
    if (settings->isDecision())
        s += tr("А вариант был - посмотрите демонстрацию.") + "\n";
    m_board->showMessage(s + tr("Щелкните для продолжения..."), false);

}

// ------------------------------------------------------------------------------------------------
// Вышли из режима Нет варианта
void CGame::slotNotVariantExited()
{
    m_board->hideMessage(false);
}

// ------------------------------------------------------------------------------------------------
// Вошли в пустое состояние
void CGame::slotEmptyEntered()
{
    m_board->setGameState(gsEmpty);
}

// ------------------------------------------------------------------------------------------------
// Вошли в состояние демонстрации
void CGame::slotDemonstrationEntered()
{
    m_board->startDemonstration();
}

// ------------------------------------------------------------------------------------------------
// Выйти из режима демонстрации
void CGame::slotDemonstrationExited()
{
//    m_board->closeDemonstration();
}


