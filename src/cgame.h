#ifndef CGAME_H
#define CGAME_H

#include "csettings.h"
#include "cscene.h"
#include "cfield.h"

#include <QObject>
#include <QHash>

// Запись за тип поля
struct FieldRec
{
    // Размеры поля
    quint16 x;
    quint16 y;
    // Количество одинаковых костяшек для каждого типа
    quint16 count;
};

// Класс управления игрой

class CGame : public QObject
{
    Q_OBJECT
public:
    explicit CGame(CScene *m_scene, QObject *parent = nullptr);

    // Состояние игры
    enum GameState { gsEmpty, gsNormal, gsPause, gsVictory, gsNotVariants, gsDemostration };

    void newGame();

private:
    GameState m_game_state;
    // Данные типов игр
    QHash<GameType, FieldRec> m_field_types;
    CField *m_field;
    CScene *m_scene;
};

#endif // CGAME_H
