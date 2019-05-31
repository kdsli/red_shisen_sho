#ifndef CSCENE_H
#define CSCENE_H

#include "common_types.h"

#include <QObject>
#include <QGraphicsScene>
#include <QPainter>

class CScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CScene(QObject *parent = nullptr);

    void setBackground(const QString &file_name);

    void newGame(const Field *field);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_bg_pixmap;

};

#endif // CSCENE_H
