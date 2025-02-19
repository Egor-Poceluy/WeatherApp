#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QCloseEvent>

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QTimer>
#include <QMenu>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void SetWeather(QString weather, int temperature, int feelLikeTemperature, int windSpeed);
    void GetWeather();

    void SetupUI();
    void InitTimer();
    bool IsCityFavorite();

    void HideLabels(const QList<QWidget*>& labels);
    void ShowLabels(const QList<QWidget*>& labels);

    void LoadStyles(QFile& file);
    void LoadCities();
    void SaveCities();
    void closeEvent(QCloseEvent *event) override;

public slots:
    void Home();
    void Search();
    void Favorite();
    void FavMenuTriggered(QAction* action);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;

    QMenu *favMenu;

    QTimer *timer;
    QLabel *timeLabel;
    QLabel *dateLabel;

    QLineEdit *cityLine;
    QPushButton *cityLineButton;
    QPushButton *favButton;
    QPushButton *favMenuButton;
    QPushButton *homePageButton;
    QPushButton *settingsButton;

    QLabel* helloLabel;
    QLabel *weatherLabel;
    QLabel *temperatureLabel;
    QLabel *feelLikeTemperatureLabel;
    QLabel *windSpeedLabel;
    QLabel *cityLabel;

    const QString APIKEY = "de62a4356226e6010427a9e8022d857c";
};
#endif // MAINWINDOW_H
