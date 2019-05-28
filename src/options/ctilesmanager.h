#ifndef CTILESMANAGER_H
#define CTILESMANAGER_H

#include <QPixmap>
#include <QFileSystemWatcher>

struct TilesFile {
    QString file_name;
    QPixmap pixmap;
    QString author;
    QString authorEmail;
    QString description;
};

// Класс управления предпросмотром наборов костяшек

class CTilesManager : public QObject
{
    Q_OBJECT
public:
    explicit CTilesManager(QObject *parent = nullptr);

    const QVector<TilesFile> &tilesList() const { return m_files; }

    // Текущий выбранный файл
    QString currentFile();
    int currentIndex();

    // Признак того, что в текущем наборе правильно выводятся SEASON и FLOWER
    // См. ремарку к реализации
    bool isCorrectSVG();

signals:
    void signalChangeTilesets();

private:
    QString m_lib_dir;
    // Список файлов
    QVector<TilesFile> m_files;
    QFileSystemWatcher m_watcher;

    void initFiles();
    void loadSvg(const QString &);

private slots:
    void slotDirectoryChanged(const QString &);
};

extern CTilesManager *tiles_manager;

#endif // CTILESMANAGER_H
