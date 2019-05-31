#ifndef CBOARD_H
#define CBOARD_H

#include "cgame.h"
#include "cscene.h"

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>

// Виджет доски отображения игры, отвечает за взаимодействие с пользователем
// Управление игрой идет в объекте CGame, который владеет классом CField,
// непосредственно манипулирующего данными игры, и объектом сцены CScene

class CBoard : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CBoard(QWidget *parent = nullptr);

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

private:
    CScene *m_scene;
    CGame *m_game;

};

#endif // CBOARD_H
