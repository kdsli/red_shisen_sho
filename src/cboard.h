#ifndef CBOARD_H
#define CBOARD_H

#include "cscene.h"
#include "cfield.h"
#include "csettings.h"

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
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

// Виджет доски отображения игры, отвечает за взаимодействие с пользователем
// и управление игрой. Владеет классом CField, непосредственно манипулирующего
// данными игры, и объектом сцены CScene

class CBoard : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CBoard(QWidget *parent = nullptr);

    // Состояние игры
    enum GameState { gsEmpty, gsNormal, gsPause, gsVictory, gsNotVariants, gsDemostration };

    QString gameInfo();

signals:
    void signalUpdateInfo();
    void signalUndoRedo(bool, bool);
    void signalShowResult(int);
    void signalUndo();
    void signalRedo();

public slots:
    void slotNewGame();
    void slotRepeatGame();
    void slotHelp();
    void slotPause();
    void slotNewTypeGame();
    void slotSetTileset();
    void slotSetBackground();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    GameState m_game_state;
    CField *m_field;
    CScene *m_scene;
    // Данные типов игр
    QHash<GameType, FieldRec> m_field_types;

    void recalcScene();

};

#endif // CBOARD_H
