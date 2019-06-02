#ifndef CTILESMANAGER_H
#define CTILESMANAGER_H

#include <QPixmap>
#include <QFileSystemWatcher>
#include <QSvgRenderer>

struct TilesFile {
    QString file_name;
    QPixmap pixmap;
    QString author;
    QString authorEmail;
    QString description;
};

// Класс управления наборами файлов костяшек

class CTilesManager : public QObject
{
    Q_OBJECT
public:
    explicit CTilesManager(QObject *parent = nullptr);

    const QVector<TilesFile> &tilesList() const { return m_files; }

    // Текущий выбранный файл
    QString currentFile();
    // Индекс текущего файла в списке файлов
    int currentIndex();
    // Иницилизировать файл с костяшками
    void initCurrentFile();

    // Renderer текущего файла
    QSvgRenderer *currentRenderer() const { return m_renderer; }

    // Массив наименований костяшек
    const QStringList &tilesNames() const { return m_tiles_names; }

    // Наименование базовых костяшек
    const QString &getBaseName() const;
    const QString &getSelectedBaseName() const;

    // Исходная ширина и высота костяшек
    const QSizeF baseSize() const { return m_base_size; }
    const QSizeF tileSize() const { return m_tile_size; }

    // Размеры тени
    qreal shadowWidht() const { return m_shadow_width; }
    qreal shadowHeight() const { return m_shadow_height; }

signals:
    void signalChangeTilesets();

private:
    QString m_lib_dir;
    // Список файлов
    QVector<TilesFile> m_files;
    QFileSystemWatcher m_watcher;
    QSvgRenderer *m_renderer;
    // Список названий костяшек
    QStringList m_tiles_names;
    // Размеры костяшек
    QSizeF m_base_size;
    QSizeF m_tile_size;
    // Ширина и высота тени костяшки
    qreal m_shadow_width, m_shadow_height;

    void addTileSeries(const QString &series_name, int count);

    void initFiles();
    void loadSvg(const QString &);

    // См. комментарий с ctilesmanager.cpp
    bool isCorrectSVG();

    void getTileSize(const QString &tile_name, QSizeF &size) const;

private slots:
    void slotDirectoryChanged(const QString &);
};

extern CTilesManager *tiles_manager;

#endif // CTILESMANAGER_H
