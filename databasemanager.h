#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QQueue>
#include <QFileDialog>
#include <QEventLoop>
#include <QFile>


#include <QDebug>


class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    explicit DatabaseManager(QString databasePath);

    ~DatabaseManager();

public:
    struct Sound
    {
        int     id;
        QString Artist;
        QString Title;
        int     Time;
        QString Path;
    };

private:
    void showError(QString errorText);
    void fetchData();

public:
    void insertData(const QString &artist, const QString &title, int time, const QString path);
    QVector<Sound> getSoundsList();

    void addSelectedSongs();

private:
    const QString openingError()       noexcept;
    const QString dataInsertionError() noexcept;

private:
    QString path;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    const QString dbPath;
    QVector<Sound> sounds;

signals:
    void songsAdded();
};

#endif // DATABASEMANAGER_H
