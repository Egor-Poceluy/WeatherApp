#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
    , manager(new QNetworkAccessManager(this))
    , favMenu(new QMenu(this))
    , timer(new QTimer(this))
    , timeLabel(new QLabel(this))
    , dateLabel(new QLabel(this))
    , cityLine(new QLineEdit(this))
    , cityLineButton(new QPushButton(this))
    , favButton(new QPushButton(this))
    , favMenuButton(new QPushButton(this))
    , homePageButton(new QPushButton(this))
    , settingsButton(new QPushButton(this))
    , helloLabel(new QLabel(this))
    , weatherLabel(new QLabel(this))
    , temperatureLabel(new QLabel(this))
    , feelLikeTemperatureLabel(new QLabel(this))
    , windSpeedLabel(new QLabel(this))
    , cityLabel(new QLabel(this))
{
    ui->setupUi(this);

    setFixedSize(350, 500);
    setWindowTitle("Погода");

    SetupUI();
    InitTimer();

    connect(favButton, &QPushButton::clicked, this, &MainWindow::Favorite);
    connect(cityLineButton, &QPushButton::clicked, this, &MainWindow::Search);
    connect(homePageButton, &QPushButton::clicked, this, &MainWindow::Home);

    connect(favMenuButton, &QPushButton::clicked, this, [this]()
            {
                favMenu->exec(favMenuButton->mapToGlobal(favMenuButton->rect().bottomLeft()));
            });

    connect(favMenu, &QMenu::triggered, this, &MainWindow::FavMenuTriggered);

    LoadCities();
    if(!favMenu->isEmpty()) {
        QAction* clearAction = new QAction("Очистить список", this);
        favMenu->addSeparator();
        favMenu->addAction(clearAction);
    }
}


void MainWindow::GetWeather()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() != QNetworkReply::NoError) {
        statusBar()->showMessage("Ошибка: " + reply->errorString());
        cityLine->clear();
        return;
    }
    statusBar()->showMessage("Готово", 2000);

    cityLabel->setText(cityLine->text());
    cityLine->clear();

    if(IsCityFavorite())
        favButton->setIcon(QIcon(":/icons/favorite_selected.png"));
    else
        favButton->setIcon(QIcon(":/icons/favorite.png"));

    HideLabels({timeLabel, dateLabel, helloLabel, settingsButton});
    ShowLabels({weatherLabel, temperatureLabel, feelLikeTemperatureLabel, windSpeedLabel, cityLabel, homePageButton, favButton});

    QJsonObject jsonObj = QJsonDocument::fromJson(reply->readAll()).object();

    double feelLikeTemperature = jsonObj["main"].toObject()["feels_like"].toDouble();
    double temperature = jsonObj["main"].toObject()["temp"].toDouble();
    QString weather = jsonObj["weather"].toArray()[0].toObject()["description"].toString();
    double windSpeed = jsonObj["wind"].toObject()["speed"].toDouble();

    SetWeather(weather, temperature, feelLikeTemperature, windSpeed);
}


void MainWindow::SetWeather(QString weather, int temperature, int feelLikeTemperature, int windSpeed)
{
    weatherLabel->setText(weather);
    temperatureLabel->setText(QString::number(temperature) + "°");
    feelLikeTemperatureLabel->setText("Ощущается как: " + QString::number(feelLikeTemperature) + "°");
    windSpeedLabel->setText("Скорость ветра: " + QString::number(windSpeed) + " м/с");
}


void MainWindow::Home()
{
    if(timeLabel->isHidden() && dateLabel->isHidden()) {
        favButton->setIcon(QIcon(":/icons/favorite.png"));

        HideLabels({weatherLabel, temperatureLabel, feelLikeTemperatureLabel, windSpeedLabel, cityLabel, homePageButton, favButton});

        weatherLabel->clear();
        temperatureLabel->clear();
        feelLikeTemperatureLabel->clear();
        windSpeedLabel->clear();
        cityLabel->clear();

        timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
        dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy"));
        ShowLabels({timeLabel, dateLabel, helloLabel, settingsButton});
        InitTimer();
    }
}


void MainWindow::Search()
{
    if(!cityLine->isHidden()) {
        if(cityLine->text().isEmpty()) {
            cityLineButton->setIcon(QIcon(":/icons/search.png"));
            cityLine->hide();
        }
        else {
            QNetworkRequest request(QUrl("https://api.openweathermap.org/data/2.5/weather?q=" + cityLine->text() + "&appid=" + APIKEY + "&units=metric&lang=ru"));
            QNetworkReply *reply = manager->get(request);
            connect(reply, &QNetworkReply::finished, this, &MainWindow::GetWeather);
        }
    }
    else {
        cityLineButton->setIcon(QIcon(":/icons/go.png"));
        cityLine->show();
    }
}


void MainWindow::Favorite()
{
    QString cityName = cityLabel->text();

    if(!cityName.isEmpty()) {
        bool cityExistsInMenu = IsCityFavorite();

        if (cityExistsInMenu) {
            favButton->setIcon(QIcon(":/icons/favorite.png"));

            for (auto* action : favMenu->actions()) {
                if (action->text() == cityName) {
                    favMenu->removeAction(action);
                    break;
                }
            }
            if(favMenu->actions().size() == 2)
                favMenu->clear();
        }
        else {
            QAction *newCity = new QAction(cityName, this);
            QAction *clearAction = new QAction("Очистить список", this);

            if(!favMenu->isEmpty()) {
                favMenu->removeAction(favMenu->actions().last());
                favMenu->removeAction(favMenu->actions().last());
            }
            favMenu->addAction(newCity);
            favMenu->addSeparator();
            favMenu->addAction(clearAction);

            favButton->setIcon(QIcon(":/icons/favorite_selected.png"));
        }
    }
}


void MainWindow::SetupUI()
{
    cityLineButton->setIcon(QIcon(":/icons/search.png"));
    favButton->setIcon(QIcon(":/icons/favorite.png"));
    favMenuButton->setIcon(QIcon(":/icons/menu.png"));
    homePageButton->setIcon(QIcon(":/icons/home.png"));
    settingsButton->setIcon(QIcon(":/icons/settings.png"));

    weatherLabel->setAlignment(Qt::AlignCenter);
    temperatureLabel->setAlignment(Qt::AlignCenter);
    feelLikeTemperatureLabel->setAlignment(Qt::AlignCenter);
    windSpeedLabel->setAlignment(Qt::AlignCenter);
    cityLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setAlignment(Qt::AlignCenter);
    dateLabel->setAlignment(Qt::AlignCenter);
    cityLine->setAlignment(Qt::AlignCenter);

    helloLabel = new QLabel(this);
    helloLabel->setAlignment(Qt::AlignCenter);

    QFile file;
    if (QTime::currentTime() >= QTime(7, 0) && QTime::currentTime() <= QTime(12, 0)) {
        helloLabel->setText("Доброе утро");
        file.setFileName(":/styles/morning.qss");
    } else if (QTime::currentTime() >= QTime(12, 0) && QTime::currentTime() <= QTime(19, 0)) {
        helloLabel->setText("Добрый день");
        file.setFileName(":/styles/day.qss");
    } else {
        helloLabel->setText("Доброй ночи");
        file.setFileName(":/styles/night.qss");
    }
    LoadStyles(file);

    timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy"));
    cityLine->setPlaceholderText("Введите название города");

    HideLabels({weatherLabel, temperatureLabel, feelLikeTemperatureLabel, windSpeedLabel, cityLabel, cityLine, favButton, homePageButton});

    QHBoxLayout *searchLayout = new QHBoxLayout(this);
    searchLayout->addWidget(favMenuButton);
    searchLayout->addWidget(cityLineButton);
    searchLayout->addWidget(cityLine);
    searchLayout->addWidget(favButton);
    searchLayout->addWidget(homePageButton);
    searchLayout->addWidget(settingsButton);

    QVBoxLayout *labelsLayout = new QVBoxLayout(this);
    labelsLayout->addWidget(cityLabel);
    labelsLayout->addWidget(temperatureLabel);
    labelsLayout->addWidget(weatherLabel);
    labelsLayout->addWidget(feelLikeTemperatureLabel);
    labelsLayout->addWidget(windSpeedLabel);

    QVBoxLayout *mainWindowLayout = new QVBoxLayout(this);
    mainWindowLayout->addLayout(searchLayout);
    mainWindowLayout->addWidget(helloLabel);
    mainWindowLayout->addWidget(timeLabel);
    mainWindowLayout->addWidget(dateLabel);
    mainWindowLayout->addLayout(labelsLayout);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainWindowLayout);
    setCentralWidget(centralWidget);
}


void MainWindow::LoadStyles(QFile& file)
{
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        setStyleSheet(file.readAll());
    file.close();
}


void MainWindow::InitTimer()
{
    connect(timer, &QTimer::timeout, this, [this]() {
        if(timeLabel->isHidden() && dateLabel->isHidden())
            timer->stop();
        else {
            timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
            dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy"));
        }
    });
    timer->start(1000);
}


void MainWindow::FavMenuTriggered(QAction* action)
{
    if(action->text() == "Очистить список") {
        favMenu->clear();
        favButton->setIcon(QIcon(":/icons/favorite.png"));

        QFile file("favCities.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.resize(0);
            file.close();
        }
    }
    else
        cityLine->setText(action->text());
}


void MainWindow::LoadCities()
{
    QFile file("favCities.txt");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        while (!ts.atEnd()) {
            QAction* newAction = new QAction(ts.readLine(), this);
            favMenu->addAction(newAction);
        }
    }
    file.close();
}


void MainWindow::SaveCities()
{
    QFile file("favCities.txt");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        for(auto& city : favMenu->actions()) {
            if(city->text() != "Очистить список" && !city->isSeparator())
                ts << city->text() << '\n';
        }
    }
    file.close();
}


bool MainWindow::IsCityFavorite()
{
    bool cityExistsInMenu = false;

    for (auto* action : favMenu->actions()) {
        if (action->text() == cityLabel->text()) {
            cityExistsInMenu = true;
            break;
        }
    }
    return cityExistsInMenu;
}


void MainWindow::HideLabels(const QList<QWidget*>& labels)
{
    for (auto* label : labels) label->hide();
}


void MainWindow::ShowLabels(const QList<QWidget*>& labels)
{
    for (auto* label : labels) label->show();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    SaveCities();
    event->accept();
}


MainWindow::~MainWindow() { delete ui; }
