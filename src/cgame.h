#ifndef CGAME_H
#define CGAME_H

#include "cboard.h"
#include "cfield.h"
#include "cscene.h"

#include <QObject>
#include <QStateMachine>

// Класс игры: конечный автомат. Владеет доской, сценой и полем

class CGame : public QStateMachine
{
    Q_OBJECT
public:
    explicit CGame(QObject *parent = nullptr);

    CBoard *board() { return m_board; }

    QString gameInfo();

public slots:
    void slotNewTypeGame();

signals:
    // Внешние сигналы нам, изменяющие состояние
    void signalNewGame();
    void signalRepeatGame();
    void signalPause();

    // Сигналы от нас
    void signalUpdateInfo();
    void signalUndoRedo();
    void signalShowResult(int);
    void signalResetPauseAction();

    // Внутрение сигналы
    void signalDoGame();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    CBoard *m_board;
    CScene *m_scene;
    CField *m_field;

    // Состояния

    // Состояние обычной игры
    QState m_game;
    // Состяние инциализации новой игры
    QState m_new_game;
    // Состояние повтора игры
    QState m_repeat_game;
    QState m_empty;
    QState m_victory;
    QState m_not_variants;
    QState m_demonstration;
    QState m_pause;

    // Данные типов игр
    QHash<GameType, FieldRec> m_field_types;
    // Таймер игры
    int m_game_timer;
    // Секунды игры
    int m_seconds;

    void doNewGame();

private slots:
    void slotGameEntered();
    void slotGameExited();

    void slotNewGameEntered();
    void slotRepeatGameEntered();

    void slotPauseEntered();
    void slotPauseExited();

    void slotVictoryEntered();
    void slotVictoryExited();

    void slotNotVariantEntered();
    void slotNotVariantExited();

    void slotEmptyEntered();

    void slotDemonstrationEntered();
    void slotDemonstrationExited();

};

#endif // CGAME_H
