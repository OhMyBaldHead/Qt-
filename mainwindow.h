#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPainter>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "weatherData.h"
#include "weatherTool.h"


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

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintlblLine();
    void weaType();
    void getWeatherInfo(QString cityName);
    void parseJson(QByteArray& byteArray);
    void updateUI();

private slots:
    void on_searchButton_clicked();

private:
    void onReplied(QNetworkReply* reply);

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager* mNetAccessManager;
    //天气数据
    Today mToday;
    Day mDay[6];
    //星期和日期
    QList<QLabel*> mWeekDayList;
    //天气图标
    QList<QLabel*> mTypeIconList;
    // 质量指数
    QList<QLabel*> mAqiList;
    //风力风向
    QList<QLabel*> mFxList;
    QMap<QString,QString>mTypeMap;
};
#endif // MAINWINDOW_H





