#ifndef CSCENE_H
#define CSCENE_H

#include "common_types.h"

#include <QObject>
#include <QGraphicsScene>
#include <QPainter>

class CField;

class CScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CScene(CField *field, QObject *parent = nullptr);

    void setBackground(const QString &file_name);

    void newGame();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_bg_pixmap;
    CField *m_field;

};

#endif // CSCENE_H
