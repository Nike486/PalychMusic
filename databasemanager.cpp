#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject{parent}
{}

DatabaseManager::DatabaseManager(QString databasePath) : dbPath(databasePath)
{
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        showError(openingError());
    }

    fetchData();
}

DatabaseManager::~DatabaseManager()
{
    db.close();
}

void DatabaseManager::showError(QString errorText)
{
    qDebug() << errorText << db.lastError().text();
    return;
}

void DatabaseManager::insertData(const QString &artist, const QString &title, int time, const QString path)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Sounds (Artist, Title, Time, Path) VALUES (:Artist, :Title, :Time, :Path)");
    query.bindValue(":Artist", artist);
    query.bindValue(":Title",  title );
    query.bindValue(":Time",   time  );
    query.bindValue(":Path",   path  );

    if (!query.exec())
    {
        showError(dataInsertionError());
    }
}

void DatabaseManager::fetchData()
{
    sounds.clear();

    QSqlQuery query("SELECT * FROM Sounds");

    while (query.next())
    {
        Sound sound;

        sound.id     = query.value(0).toInt();
        sound.Artist = query.value(1).toString();
        sound.Title  = query.value(2).toString();
        sound.Time   = query.value(3).toInt();
        sound.Path   = query.value(4).toString();

        sounds.append(sound);        
    }

    emit songsAdded();
}

void DatabaseManager::addSelectedSongs()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        "Выберите MP3 файлы",
        "",
        "MP3 файлы (*.mp3);;Все файлы (*.*)"
        );

    if (fileNames.isEmpty())
    {
        return;
    }

    for (const auto &fileName : fileNames)
    {
        QMediaPlayer *mp = new QMediaPlayer();
        mp->setSource(QUrl::fromLocalFile(fileName));

        connect(mp, &QMediaPlayer::mediaStatusChanged, [this, mp, fileName](QMediaPlayer::MediaStatus status)
                {
                    if (status == QMediaPlayer::MediaStatus::LoadedMedia)
                    {
                        QMediaMetaData md = mp->metaData();

                        QString artist = md.stringValue(QMediaMetaData::ContributingArtist);
                        QString title = md.stringValue(QMediaMetaData::Title);
                        qint64 duration = md.value(QMediaMetaData::Duration).toLongLong() / 1000;                        

                        mp->stop();
                        mp->deleteLater();


                        insertData(artist, title, duration, fileName);
                        fetchData();
                    }
                });
    }
}

QVector<DatabaseManager::Sound> DatabaseManager::getSoundsList()
{
    return sounds;
}

const QString DatabaseManager::openingError()       noexcept { return "Ошибка открытия базы данных: "; }
const QString DatabaseManager::dataInsertionError() noexcept { return "Ошибка вставки данных: ";       }
