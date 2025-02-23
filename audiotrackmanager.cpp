#include "audiotrackmanager.h"

AudioTrackManager::AudioTrackManager(ListUI listUI, QObject *parent) : QObject{parent},
    p_titleLabel        (listUI.titleLabel),
    p_artistLabel       (listUI.artistLabel),
    p_positionLabel     (listUI.positionLabel),
    p_durationLabel     (listUI.durationLabel),
    p_positionSlider    (listUI.positionSlider),
    p_volumeSlider      (listUI.volumeSlider),
    p_buttonPlayPause   (listUI.buttonPlayPause),
    p_buttonSwitchLeft  (listUI.buttonSwitchLeft),
    p_buttonSwitchRight (listUI.buttonSwitchRight),
    p_buttonFileDialog  (listUI.buttonFileDialog),
    p_scrollAreaSounds  (listUI.scrollAreaSounds)
{
    player      = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    dataDase    = new DatabaseManager(QString::fromStdString(R"(PATH)")); // Добавить путь!!!

    p_positionLabel->setText(GetStringPositionPlayer());
    p_durationLabel->setText(GetStringDurationPlayer());

    p_buttonPlayPause->setText(playSymbol);

    if (p_scrollAreaSounds)
    {
        p_scrollAreaSounds->setWidgetResizable(true);
    }

    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5);                  // переделать с get

    soundsList = dataDase->getSoundsList();       // Сделать отдельный функционал

    if (!soundsList.empty())
    {
        SetSound(currentSound);
        showSongsInWindow();
    }
    else
    {
        p_buttonPlayPause->setDisabled(true);      // Сделать отдельный функцию
        p_buttonSwitchLeft->setDisabled(true);
        p_buttonSwitchRight->setDisabled(true);
        p_volumeSlider->setDisabled(true);
        p_positionSlider->setDisabled(true);
    }

    connect(p_positionSlider,    &QSlider::sliderMoved,               this, &AudioTrackManager::sliderPositionMoved       );
    connect(p_positionSlider,    &QSlider::sliderPressed,             this, &AudioTrackManager::sliderPositionPressed     );
    connect(p_positionSlider,    &QSlider::sliderReleased,            this, &AudioTrackManager::sliderPositionReleased    );

    connect(p_volumeSlider,      &QSlider::sliderMoved,               this, &AudioTrackManager::sliderVolumeMoved         );
    connect(p_volumeSlider,      &QSlider::sliderReleased,            this, &AudioTrackManager::sliderVolumeReleased      );    

    connect(player,              &QMediaPlayer::positionChanged,      this, &AudioTrackManager::SetSoundPosition          );
    connect(player,              &QMediaPlayer::mediaStatusChanged,   this, &AudioTrackManager::onMediaStatusChanged      );
    connect(player,              &QMediaPlayer::playbackStateChanged, this, &AudioTrackManager::onPlaybackStateChanged    );

    connect(p_buttonPlayPause,   &QPushButton::clicked,               this, &AudioTrackManager::onButtonPlayPauseClicked  );
    connect(p_buttonSwitchLeft,  &QPushButton::clicked,               this, &AudioTrackManager::onButtonSwitchLeftClicked );
    connect(p_buttonSwitchRight, &QPushButton::clicked,               this, &AudioTrackManager::onButtonSwitchRightClicked);
    connect(p_buttonFileDialog,  &QPushButton::clicked,               this, &AudioTrackManager::onButtonFileDialog        );

    connect(dataDase,            &DatabaseManager::songsAdded,        this, &AudioTrackManager::onSongsAdded              );
    connect(dataDase,            &DatabaseManager::songsAdded,        this, &AudioTrackManager::showSongsInWindow         );
}

AudioTrackManager::~AudioTrackManager() = default;

void AudioTrackManager::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
    case QMediaPlayer::MediaStatus::LoadedMedia:
        p_durationLabel->setText(GetStringDurationPlayer());
        p_positionSlider->setMaximum(player->duration() / 1000);

        mediaMetaData = player->metaData();

        p_artistLabel->setText(mediaMetaData.stringValue(QMediaMetaData::ContributingArtist)); // брать данные из базы данных
        p_titleLabel->setText(mediaMetaData.stringValue(QMediaMetaData::Title));               // брать данные из базы данных

        break;

    case QMediaPlayer::MediaStatus::BufferingMedia:
        break;

    case QMediaPlayer::MediaStatus::EndOfMedia:

        SetSound(GetCurrentSounds() + 1);
        PlaySound();

        break;

    case QMediaPlayer::MediaStatus::InvalidMedia:
        //Тут сделать обработчик ошибок
        break;

    default:
        break;
    }
}

void AudioTrackManager::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    QPushButton *button = scrollContent->findChild<QPushButton*>(QString("playButton_%1").arg(currentSound));

    qint64 countSounds = GetCountSounds();
    for (auto i = 1; i < countSounds + 1; ++i)
    {
        QPushButton *buttonIn = scrollContent->findChild<QPushButton*>(QString("playButton_%1").arg(i));        // Лучше использовать список кнопок.
        if (buttonIn)
        {
            buttonIn->setText(playSymbol);
        }
    }

    switch (state)
    {
    case QMediaPlayer::PlaybackState::PlayingState:
        p_buttonPlayPause->setText(pauseSymbol);
        if (button)
        {
           button->setText(pauseSymbol);
        }
        break;

    case QMediaPlayer::PlaybackState::PausedState:
        p_buttonPlayPause->setText(playSymbol);
        if (button)
        {
            button->setText(playSymbol);
        }
        break;

    case QMediaPlayer::PlaybackState::StoppedState:
        break;

    default:
        break;
    }
}

void AudioTrackManager::onButtonPlayPauseClicked()
{
    player->playbackState() == QMediaPlayer::PlaybackState::PlayingState
                             ? PauseSound()
                             : PlaySound();
}

void AudioTrackManager::onButtonSwitchRightClicked()
{
    qint64 currentSounds = GetCurrentSounds() + 1;
    SetSound(currentSounds);
    PlaySound();
}

void AudioTrackManager::onButtonSwitchLeftClicked()
{
    qint64 currentSounds = GetCurrentPositionPlayer() < 3
                         ? GetCurrentSounds() - 1
                         : GetCurrentSounds();

    SetSound(currentSounds);
    PlaySound();
}

void AudioTrackManager::onButtonFileDialog()
{
    dataDase->addSelectedSongs();
}

void AudioTrackManager::onSongsAdded()
{
    soundsList = dataDase->getSoundsList();

    if (!soundsList.empty() && player->mediaStatus() == QMediaPlayer::MediaStatus::NoMedia)
    {
        SetSound(currentSound);        

        p_buttonPlayPause->setDisabled(false);      // Сделать отдельный функцию
        p_buttonSwitchLeft->setDisabled(false);
        p_buttonSwitchRight->setDisabled(false);
        p_volumeSlider->setDisabled(false);
        p_positionSlider->setDisabled(false);
    }
}

void AudioTrackManager::showSongsInWindow()
{
    if (soundsList.empty())
    {
        return;
    }

    if (!scrollContent)
    {
        scrollContent = new QWidget();
        scrollLayout  = new QVBoxLayout(scrollContent);
        p_scrollAreaSounds->setWidget(scrollContent);
        p_scrollAreaSounds->setWidgetResizable(false);
    }

    QLayoutItem* item;
    while ((item = scrollLayout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    scrollContent->setFixedHeight(soundsList.size() * 60);

    for (auto soundInfo : soundsList)
    {
        addSongInWindow(scrollLayout, soundInfo.Title, soundInfo.Artist, "album", soundInfo.Time, soundInfo.id);
    }
}

void AudioTrackManager::addSongInWindow(QVBoxLayout *layout, const QString &title,
                                                             const QString &artist,
                                                             const QString &album,
                                                             qint64 time,
                                                             qint64 id)
{
    QWidget     *songWidget = new QWidget();
    QHBoxLayout *songLayout = new QHBoxLayout();
    QVBoxLayout *artistTitleLayout = new QVBoxLayout();

    QPushButton *playButton = new QPushButton(playSymbol);

    playButton->setObjectName(QString("playButton_%1").arg(id));

    connect(playButton, &QPushButton::clicked, this, [this, id]()
    {
        if (player->playbackState() == QMediaPlayer::PlaybackState::PlayingState && currentSound == id)
        {
            PauseSound();
        }
        else
        {
            if (currentSound != id)
            {
              SetSound(id);
            }

            PlaySound();
        }
    });

    QLabel *labelArtist = new QLabel(artist);
    QLabel *labelTitle  = new QLabel(title);
    QLabel *labelAlbum  = new QLabel(album);
    QLabel *labelTime   = new QLabel(FormatTime(time));

    labelTitle->setStyleSheet("font-weight: bold;");

    playButton->setFixedSize(40, 40);

    const qint64 sizeLabel = 300;

    labelArtist->setFixedWidth(sizeLabel);
    labelTitle->setFixedWidth(sizeLabel);
    labelAlbum->setFixedWidth(sizeLabel);
    labelTime->setFixedWidth(50);

    artistTitleLayout->addWidget(labelTitle);
    artistTitleLayout->addWidget(labelArtist);

    songLayout->addWidget(playButton);
    songLayout->addLayout(artistTitleLayout);
    songLayout->addWidget(labelAlbum);
    songLayout->addWidget(labelTime);

    songWidget->setLayout(songLayout);
    songWidget->setFixedHeight(60);
    songWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(songWidget);
}

void AudioTrackManager::SetSound(qint64 numSound)
{
    SetCurrentSounds(numSound);
    qint64 currentSounds = GetCurrentSounds();

    QString pathToSound = soundsList[currentSounds - 1].Path;
    player->setSource(QUrl::fromLocalFile(pathToSound));
}

void AudioTrackManager::PlaySound()
{
    player->play();   
}

void AudioTrackManager::PauseSound()
{
    player->pause();
}

void AudioTrackManager::StopSound()
{
    player->stop();
}

void AudioTrackManager::SetSoundPosition(qint64 duration)
{
    qint64 convertedToSeconds = duration / 1000;
    qint64 minutes            = convertedToSeconds / 60;
    qint64 seconds            = convertedToSeconds % 60;

    QString formattedPosition = QString("%1:%2")
                                                .arg(minutes)
                                                .arg(seconds, 2, 10, QChar('0'));

    p_positionLabel->setText(formattedPosition);
    p_positionSlider->setValue(convertedToSeconds);
}

QString AudioTrackManager::FormatTime(qint64 fullSeconds) const
{
    qint64 minutes = (fullSeconds) / 60;
    qint64 seconds = (fullSeconds) % 60;

    return QString("%1:%2")
                           .arg(minutes)
                           .arg(seconds, 2, 10, QChar('0'));
}

void AudioTrackManager::sliderPositionMoved(int position)
{
    p_positionLabel->setText(FormatTime(position));
}

void AudioTrackManager::sliderPositionPressed()
{
    disconnect(player, &QMediaPlayer::positionChanged, this, &AudioTrackManager::SetSoundPosition);
    qint64 position = p_positionSlider->value();
    p_positionLabel->setText(FormatTime(position));
}

void AudioTrackManager::sliderPositionReleased()
{
    SetPositionPlayer(p_positionSlider->value() * 1000);
    connect(player, &QMediaPlayer::positionChanged, this, &AudioTrackManager::SetSoundPosition);
}

void AudioTrackManager::sliderVolumeMoved(int position)
{
    audioOutput->setVolume(position / 100.0);
}

void AudioTrackManager::sliderVolumeReleased()
{
    audioOutput->setVolume(p_volumeSlider->value() / 100.0);
}

qint64 AudioTrackManager::GetCurrentPositionPlayer() const
{
    return player->position() / 1000;
}

QString AudioTrackManager::GetStringPositionPlayer() const
{  
    return FormatTime(GetCurrentPositionPlayer());
}

QString AudioTrackManager::GetStringDurationPlayer() const
{
    return FormatTime(player->duration() / 1000);
}

void AudioTrackManager::SetPositionPlayer(qint64 volue)
{
    player->setPosition(volue);
}

qint64 AudioTrackManager::GetCountSounds() const
{
    return soundsList.size();
}

qint64 AudioTrackManager::GetCurrentSounds() const
{
    return soundsList[currentSound - 1].id;
}

void AudioTrackManager::SetCurrentSounds(qint64 num)
{
    qint64 countSounds = GetCountSounds();

    if (num < 1)
    {
        currentSound = countSounds;
    }
    else if (num > countSounds)
    {
        currentSound = 1;
    }
    else
    {
        currentSound = num;
    }
}
