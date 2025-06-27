#include "mainwindow.h"
#include "ui_mainwindow.h"

#define K 2   //温度曲线弯折系数,k>0,k越大温度曲线越弯曲

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);  // 设置无边框
    setFixedSize(width(), height());         // 设置固定窗口大小
    //设置主窗口背景
    QLabel *backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/res/background.png"));
    backgroundLabel->setScaledContents(true);
    backgroundLabel->lower(); // 将标签置于底层
    backgroundLabel->resize(size());
    //创建一个退出按钮
    QPushButton *exitButton = ui->exitButton;
    exitButton->setStyleSheet("background: transparent;");
    exitButton->setIcon(QIcon(":/res/close.png"));  // 图片路径
    exitButton->setIconSize(exitButton->size());
    connect(ui->exitButton,&QPushButton::clicked,this,[=]() { qApp->exit(0); });
    connect(ui->searchButton,&QPushButton::clicked,this,&MainWindow::on_searchButton_clicked,Qt::UniqueConnection);

    //网络请求
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

    //给lblLine添加一个过滤器
    ui->lblLine->installEventFilter(this);
    weaType();
    getWeatherInfo("杭州");
}

MainWindow::~MainWindow()
{
    delete ui;
}

QMap<QString,QString> WeatherTool::mCityMaps={};

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //绘制最高温度曲线
    if(watched == ui->lblLine && event->type() == QEvent::Paint){
        paintlblLine();
    }
    return QWidget::eventFilter(watched,event);
}

//横坐标第一个点40，向后每个点间隔90
//纵坐标高温第一个点=171*1/4  后续点=171*1/4+(温度差)*K其中K为缩放系数
//纵坐标低温第一个点=171*3/4  后续点=171*3/4+(温度差)*K其中K为缩放系数
void MainWindow::paintlblLine(){
    //假设温度
    int PointX[6] = {40,40+90,40+180,40+270,40+360,40+450};
    int HighTemp[6] = {0};
    int LowTemp[6] = {0};
    int HighPointY[6] = {ui->lblLine->height()*1/4,0,0,0,0,0};
    int LowPointY[6] = {ui->lblLine->height()*3/4,0,0,0,0,0};
    for(int i = 0;i<6;i++){
        HighTemp[i] = mDay[i].high;
        LowTemp[i] = mDay[i].low;
    }
    for(int i = 1;i < 6;i++){
        HighPointY[i] = HighPointY[0]+(HighTemp[i]-HighTemp[0])*K;
        LowPointY[i] = LowPointY[0]+(LowTemp[i]-LowTemp[0])*K;
    }

    //对气温曲线进行简单的画图测试
    QPainter painter(ui->lblLine);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QPen pen = painter.pen();
    pen.setStyle(Qt::DashLine);
    pen.setWidth(3);
    //绘制高温点和折线
    pen.setColor(QColor(255,127,39));
    painter.setPen(pen);
    painter.setBrush(QColor(255,127,39));    //设置画刷内部填充的颜色
    for(int i = 0;i<6;i++){
        painter.drawEllipse(PointX[i], HighPointY[i], 3, 3);
        painter.drawText(PointX[i]-10, HighPointY[i]-10, QString::number(HighTemp[i]) + "°");
        if(i > 0)
            painter.drawLine(PointX[i],HighPointY[i],PointX[i-1],HighPointY[i-1]);
    }
    //绘制低温点和折线
    pen.setColor(QColor(0,168,243));
    painter.setPen(pen);
    painter.setBrush(QColor(0,168,243));    //设置画刷内部填充的颜色
    for(int i = 0;i<6;i++){
        painter.drawEllipse(PointX[i], LowPointY[i], 3, 3);
        painter.drawText(PointX[i]-10, LowPointY[i]-10, QString::number(LowTemp[i]) + "°");
        if(i > 0)
            painter.drawLine(PointX[i],LowPointY[i],PointX[i-1],LowPointY[i-1]);
    }
}

void MainWindow::weaType()
{
    //将控件添加到控件数组
    mWeekDayList << ui->lblWeekDay0 << ui->lblWeekDay1 << ui->lblWeekDay2 << ui->lblWeekDay3 << ui->lblWeekDay4 << ui->lblWeekDay5;
    mTypeIconList << ui->lblIcon0 << ui->lblIcon1 << ui->lblIcon2 << ui->lblIcon3 << ui->lblIcon4 << ui->lblIcon5;
    mAqiList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;
    mFxList << ui->lblWind0 << ui->lblWind1 << ui->lblWind2 << ui->lblWind3 << ui->lblWind4 << ui->lblWind5;

    //天气对应图标
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到暴雪",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到大暴雪",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName);
    if(cityCode.isEmpty()){
        QMessageBox::warning(this,"提示","请检查城市名是否输错，该页面只支持国内。",QMessageBox::Ok);
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    //qDebug()<< ("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(reply->error() != QNetworkReply::NoError || statusCode != 200){
        qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }else{
        QByteArray  byteArray = reply->readAll();
        //qDebug() << "读所有：" << byteArray.data();
        parseJson(byteArray);
    }
    reply->deleteLater();
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray,&err);  // 检测json格式
    if(err.error != QJsonParseError::NoError){    // Json格式错误
        return;
    }

    QJsonObject rootObj = doc.object();
    //    qDebug() << rootObj.value("message").toString();

    //解析日期和城市
    mToday.date = rootObj.value("date").toString();
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();
    int index = mToday.city.indexOf("市");
    QString result = mToday.city.left(index); // 取出 "市" 前面的子串
    mToday.city = result;

    //解析昨天
    QJsonObject objData = rootObj.value("data").toObject();

    QJsonObject objYesterday = objData.value("yesterday").toObject();
    mDay[0].week = objYesterday.value("week").toString();
    mDay[0].date = objYesterday.value("ymd").toString();

    mDay[0].type = objYesterday.value("type").toString();

    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    mDay[0].high = s.toInt();

    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    mDay[0].low = s.toInt();

    //风向风力
    mDay[0].fx = objYesterday.value("fx").toString();
    mDay[0].fl = objYesterday.value("fl").toString();
    //空气质量指数
    mDay[0].aqi = objYesterday.value("aqi").toInt();

    //解析预报中的5天数据
    QJsonArray forecatArr = objData.value("forecast").toArray();
    for(int i = 0;i < 5;i++){
        QJsonObject objForecast = forecatArr[i].toObject();
        mDay[i + 1].week = objForecast.value("week").toString();
        mDay[i + 1].date = objForecast.value("ymd").toString();
        //天气类型
        mDay[i + 1].type = objForecast.value("type").toString();

        QString s;
        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        mDay[i + 1].high = s.toInt();

        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        mDay[i + 1].low = s.toInt();

        //风向风力
        mDay[i + 1].fx = objForecast.value("fx").toString();
        mDay[i + 1].fl = objForecast.value("fl").toString();
        //空气质量指数
        mDay[i + 1].aqi = objForecast.value("aqi").toInt();
    }

    //解析今天的数据
    mToday.ganmao = objData.value("ganmao").toString();
    mToday.wendu = int(objData.value("wendu").toString().toDouble());
    mToday.shidu = objData.value("shidu").toString();
    mToday.pm25 = objData.value("pm25").toInt();
    mToday.quality = objData.value("quality").toString();
    //forecast 中的第一个数组元素，即今天的数据
    mToday.type = mDay[1].type;

    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;

    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;
    //更新UI
    updateUI();

}

void MainWindow::updateUI()
{
    //更新日期和城市
    ui->lblDateCity->setText(mToday.city+" "+QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")
                         + " " + mDay[1].week);

    //更新今天
    ui->TodayIcon->setPixmap(mTypeMap[mToday.type]);
    ui->TodayIconText->setText(mToday.city + "\n" +
                               "当前温度" + QString::number(mToday.wendu) + "°C" + "\n" +
                               QString::number(mToday.low) + "~" + QString::number(mToday.high) + "°C" + "\n" +
                               mToday.type);
    ui->lblGanMao->setText("感冒指数：" + mToday.ganmao);
    ui->lblIconFengliText->setText(mDay[1].fx+"\n"+mDay[1].fl);
    ui->lblIconPM25Text->setText("PM2.5\n" + QString::number(mToday.pm25));
    ui->lblIconShidutext->setText("湿度\n" + mToday.shidu);
    ui->lblIconQualityText->setText("空气质量\n" + mToday.quality);

    //更新六天的数据
    for(int i = 0;i < 6;i++){
        //更新日期和时间
        QStringList ymdList = mDay[i].date.split("-");
        if(i == 0){
            mWeekDayList[i]->setText(QString("昨天")+"\n"+ymdList[1]+"/"+ymdList[2]);
        }
        else if(i == 1){
            mWeekDayList[i]->setText(QString("今天")+"\n"+ymdList[1]+"/"+ymdList[2]);
        }
        else if(i == 2){
            mWeekDayList[i]->setText(QString("明天")+"\n"+ymdList[1]+"/"+ymdList[2]);
        }
        else{
            mWeekDayList[i]->setText(mDay[i].week+"\n"+ymdList[1]+"/"+ymdList[2]);
        }

        //更新天气类型
        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);

        //更新空气质量
        if(mDay[i].aqi >0 && mDay[i].aqi <= 50){
            mAqiList[i]->setText("优");
        }else if(mDay[i].aqi > 50 && mDay[i].aqi <= 100){
            mAqiList[i]->setText("良");
        }else if(mDay[i].aqi > 100 && mDay[i].aqi <= 150){
            mAqiList[i]->setText("轻度");
        }else if(mDay[i].aqi > 150 && mDay[i].aqi <= 200){
            mAqiList[i]->setText("中度");
        }else if(mDay[i].aqi > 150 && mDay[i].aqi <= 200){
            mAqiList[i]->setText("重度");
        }else{
            mAqiList[i]->setText("严重");
        }
        ui->lblLine->update();
        //更新风力、风向
        mFxList[i]->setText(mDay[i].fx+"\n"+mDay[i].fl);
    }

}

void MainWindow::on_searchButton_clicked()
{
    QString newcity = ui->searchEdit->text();
    getWeatherInfo(newcity);
}

