#ifndef AUDIOTRACKMANAGER_H
#define AUDIOTRACKMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include <databasemanager.h>

#include <QDebug> // Потом убрать

struct ListUI
{
    QLabel      *titleLabel;
    QLabel      *artistLabel;

    QLabel      *positionLabel;
    QLabel      *durationLabel;

    QSlider     *positionSlider;
    QSlider     *volumeSlider;

    QPushButton *buttonPlayPause;
    QPushButton *buttonSwitchLeft;
    QPushButton *buttonSwitchRight;

    QPushButton *buttonFileDialog;

    QScrollArea *scrollAreaSounds;
};

class AudioTrackManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioTrackManager(ListUI listUI, QObject *parent = nullptr);

    ~AudioTrackManager();

public:
    void     onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void     onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

    void     onButtonPlayPauseClicked();
    void     onButtonSwitchRightClicked();
    void     onButtonSwitchLeftClicked();

    void     onButtonFileDialog();

    void     SetSound(qint64 numSound);
    void     PlaySound();
    void     PauseSound();
    void     StopSound();
    QString  GetStringPositionPlayer() const;
    qint64   GetCurrentPositionPlayer() const;
    QString  GetStringDurationPlayer() const;
    void     SetPositionPlayer(qint64 volue);

    qint64   GetCountSounds()   const;
    qint64   GetCurrentSounds() const;

    void     SetCurrentSounds(qint64 num);

private:
    void     SetSoundPosition(qint64 duration);
    QString  FormatTime(qint64 fullSeconds) const;

private:
    QMediaPlayer    *player;
    QAudioOutput    *audioOutput;
    DatabaseManager *dataDase;

    QMediaMetaData mediaMetaData;

    QLabel  *p_titleLabel;
    QLabel  *p_artistLabel;
    QLabel  *p_positionLabel;
    QLabel  *p_durationLabel;

    QSlider *p_positionSlider;
    QSlider *p_volumeSlider;

    QPushButton *p_buttonPlayPause;
    QPushButton *p_buttonSwitchLeft;
    QPushButton *p_buttonSwitchRight;

    QPushButton *p_buttonFileDialog;

    QScrollArea *p_scrollAreaSounds;

    qint64   currentSound = 1;

    QList<DatabaseManager::Sound> soundsList;

    QWidget     *scrollContent = nullptr;
    QVBoxLayout *scrollLayout  = nullptr;

    void onSongsAdded();

    void showSongsInWindow();
    void addSongInWindow(QVBoxLayout *layout, const QString &title,
                                              const QString &artist,
                                              const QString &album,
                                              qint64 time,
                                              qint64 id);

private:
    void sliderPositionMoved(int position);
    void sliderPositionPressed();
    void sliderPositionReleased();

    void sliderVolumeMoved(int position);
    void sliderVolumeReleased();

private:
    const QString playSymbol  = "Play";
    const QString pauseSymbol = "Pause";

signals:
};

#endif // AUDIOTRACKMANAGER_H
