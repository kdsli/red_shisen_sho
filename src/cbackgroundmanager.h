#ifndef CBACKGROUNDMANAGER_H
#define CBACKGROUNDMANAGER_H

#include <QObject>
#include <QVector>
#include <QPixmap>
#include <QFileSystemWatcher>

struct BackgroundFile {
    QString file_name;
    QPixmap pixmap;
    QString author;
    QString authorEmail;
    QString description;
};

// Класс управления списком фоновых изображений
// Существует директория с изображениями SVG, взятыми из оригинальной игры
// Так же существует директория с пользовательскими файлами фонов в формате JPG и PNG

class CBackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit CBackgroundManager(QObject *parent = nullptr);

    const QVector<BackgroundFile> &bgList() const { return m_files; }

    // Текущий выбранный файл
    QString currentFile();
    // Тип - SVG или JPG/PNG
    bool isCurrentSvg();
    int currentIndex();

signals:
    void signalChangeBackgrounds();

private:
    QString m_lib_dir;
    QString m_user_dir;
    // Список файлов
    QVector<BackgroundFile> m_files;
    QFileSystemWatcher m_lib_watcher;
    QFileSystemWatcher m_user_watcher;

    void initFiles();
    void loadSvg(const QString &dir, const QString &);
    void loadImage(const QString &dir, const QString &);

private slots:
    void slotDirectoryChanged(const QString &);

};

extern CBackgroundManager *bg_manager;

#endif // CBACKGROUNDMANAGER_H
