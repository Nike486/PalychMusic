#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    listUI =
        {
            ui->labelTitle,
            ui->labelArtist,

            ui->labelPosition,
            ui->labelDuration,

            ui->sliderPosition,
            ui->sliderVolume,

            ui->buttonPlayPause,
            ui->buttonSwitchLeft,
            ui->buttonSwitchRight,

            ui->buttonFileDialog,

            ui->scrollArea
        };

    audioTrackManager = new AudioTrackManager(listUI);
}

MainWindow::~MainWindow()
{
    delete audioTrackManager;
    delete ui;
}
