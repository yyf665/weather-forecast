#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "weathertool.h"



#include <QContextMenuEvent>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPainter>
#include <QTimer>

#define INCREMENT 3 //坐标随温度变化的倍率
#define POINT_RADIUS 3 //点大小
#define TEXT_OFFSET_X 8 //温度位置偏移量
#define TEXT_OFFSET_Y 10

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);//设置无边框
    setFixedSize(width(),height());     //固定大小

    //构建右键菜单
    m_exitMenu=new QMenu(this);
    m_exitAct=new QAction();

    m_exitAct->setText(tr("退出"));
    m_exitAct->setIcon(QIcon(":/pic/res/close.png"));

    m_exitMenu->addAction(m_exitAct);
    connect(m_exitAct,&QAction::triggered,this,[=]{
        qApp->exit(0);
    });

    //控件数组
    m_weekList << ui->label_Week0 << ui->label_Week1 << ui->label_Week2 << ui->label_Week3 << ui->label_Week4 << ui->label_Week5;
    m_dateList << ui->label_Date0 << ui->label_Date1 << ui->label_Date2 << ui->label_Date3 << ui->label_Date4 << ui->label_Date5;
    m_typeList << ui->label_Type0 << ui->label_Type1 << ui->label_Type2 << ui->label_Type3 << ui->label_Type4 << ui->label_Type5;
    m_typeIconList << ui->label_TypeIcon0 << ui->label_TypeIcon1 << ui->label_TypeIcon2 << ui->label_TypeIcon3 << ui->label_TypeIcon4 << ui->label_TypeIcon5;
    m_aqiList << ui->label_Quality0 << ui->label_Quality1 << ui->label_Quality2 << ui->label_Quality3 << ui->label_Quality4 << ui->label_Quality5;
    m_fxList << ui->label_FX0 << ui->label_FX1 << ui->label_FX2 << ui->label_FX3 << ui->label_FX4 << ui->label_FX5;
    m_flList << ui->label_FI0 << ui->label_FI1 << ui->label_FI2 << ui->label_FI3 << ui->label_FI4 << ui->label_FI5;

    //图标映射
    m_typeMap.insert("暴雪",":/pic/res/type/BaoXue.png");
    m_typeMap.insert("暴雨",":/pic/res/type/BaoYu.png");
    m_typeMap.insert("暴雨大到暴雨",":/pic/res/type/BaoYuDaoDaBaoYu.png");
    m_typeMap.insert("大暴雪",":/pic/res/type/DaBaoXue.png");
    m_typeMap.insert("大暴雨",":/pic/res/type/DaBaoYu.png");
    m_typeMap.insert("大雪",":/pic/res/type/DaXue.png");
    m_typeMap.insert("大雨",":/pic/res/type/DaYu.png");
    m_typeMap.insert("小雪",":/pic/res/type/XiaoXue.png");
    m_typeMap.insert("小雨",":/pic/res/type/XiaoYu.png");
    m_typeMap.insert("阵雪",":/pic/res/type/ZhenXue.png");
    m_typeMap.insert("阵雨",":/pic/res/type/ZhenYu.png");
    m_typeMap.insert("中雪",":/pic/res/type/ZhongXue.png");
    m_typeMap.insert("中雨",":/pic/res/type/ZhongYu.png");
    m_typeMap.insert("中到大雪",":/pic/res/type/ZhongDaoDaXue.png");
    m_typeMap.insert("中到大雨",":/pic/res/type/ZhongDaoDaYu.png");
    m_typeMap.insert("雨夹雪",":/pic/res/type/YuJiaXue.png");
    m_typeMap.insert("雨",":/pic/res/type/Yu.png");
    m_typeMap.insert("阴",":/pic/res/type/Yin.png");
    m_typeMap.insert("扬沙",":/pic/res/type/YangSha.png");
    m_typeMap.insert("雪",":/pic/res/type/Xue.png");
    m_typeMap.insert("雾",":/pic/res/type/Wu.png");
    m_typeMap.insert("特大暴雨",":/pic/res/type/TeDaBaoYu.png");
    m_typeMap.insert("沙尘暴",":/pic/res/type/ShaChenBao.png");
    m_typeMap.insert("晴",":/pic/res/type/Qing.png");
    m_typeMap.insert("强沙尘暴",":/pic/res/type/QiangShaChenBao.png");
    m_typeMap.insert("霾",":/pic/res/type/Mai.png");
    m_typeMap.insert("雷阵雨",":/pic/res/type/LeiZhenYu.png");
    m_typeMap.insert("雷阵雨伴有冰雹",":/pic/res/type/LeiZhenYuBanYouBingBao.png");
    m_typeMap.insert("浮尘",":/pic/res/type/FuChen.png");
    m_typeMap.insert("多云",":/pic/res/type/DuoYun.png");
    m_typeMap.insert("冻雨",":/pic/res/type/DongYu.png");

    //网络请求
    m_netAccessManager=new QNetworkAccessManager(this);
    connect(m_netAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

    //test Beijing
    //getCityWeather("101010100");
    getCityWeather("苏州");

    //标签添加事件过滤器
    ui->label_HigCurve->installEventFilter(this);
    ui->label_LowCurve->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//重写父类虚函数
//父类默认忽略右键事件
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    m_exitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffSet=event->globalPos()-this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-mOffSet);
}

void MainWindow::getCityWeather(QString cityname)
{
    QString cityCode=WeatherTool::getCityCode(cityname);
    if(cityCode.isEmpty())
    {
        QMessageBox::warning(this,"天气","请检查输入是否正确",QMessageBox::Ok);
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    m_netAccessManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
    if(err.error!=QJsonParseError::NoError)
    {
        return;
    }
    QJsonObject rootObj=doc.object();
    //qDebug() << rootObj.value("message").toString();

    //date and city
    m_toDay.date=rootObj.value("date").toString();
    m_toDay.city=rootObj.value("cityInfo").toObject().value("city").toString();

    //yesterday
    QJsonObject objData=rootObj.value("data").toObject();
    QJsonObject objYesterday=objData.value("yesterday").toObject();

    m_Day[0].week=objYesterday.value("week").toString();
    m_Day[0].date=objYesterday.value("ymd").toString();
    m_Day[0].type=objYesterday.value("type").toString();

    QString str;
    str=objYesterday.value("high").toString().split(" ").at(1);
    str=str.left(str.length()-1);
    m_Day[0].high=str.toInt();

    str=objYesterday.value("low").toString().split(" ").at(1);
    str=str.left(str.length()-1);
    m_Day[0].low=str.toInt();

    m_Day[0].fx=objYesterday.value("fx").toString();
    m_Day[0].fl=objYesterday.value("fl").toString();
    m_Day[0].aqi=objYesterday.value("aqi").toDouble();

    //forcast 5 days
    QJsonArray dayArray=objData.value("forecast").toArray();
    QJsonObject objDay;

    for (int i = 0; i < 5; i++) {
        objDay=dayArray[i].toObject();
        m_Day[i+1].week=objDay.value("week").toString();
        m_Day[i+1].date=objDay.value("ymd").toString();
        m_Day[i+1].type=objDay.value("type").toString();

        str=objDay.value("high").toString().split(" ").at(1);
        str=str.left(str.length()-1);
        m_Day[i+1].high=str.toInt();

        str=objDay.value("low").toString().split(" ").at(1);
        str=str.left(str.length()-1);
        m_Day[i+1].low=str.toInt();

        m_Day[i+1].fx=objDay.value("fx").toString();
        m_Day[i+1].fl=objDay.value("fl").toString();
        m_Day[i+1].aqi=objDay.value("aqi").toDouble();
    }

    //today
    m_toDay.ganmao=objData.value("ganmao").toString();
    m_toDay.wendu=objData.value("wendu").toString().toInt();
    m_toDay.pm25=objData.value("pm25").toDouble();
    m_toDay.shidu=objData.value("shidu").toString();
    m_toDay.quality=objData.value("quality").toString();

    //today in 5 days
    m_toDay.type=m_Day[1].type;
    m_toDay.fx=m_Day[1].fx;
    m_toDay.fl=m_Day[1].fl;
    m_toDay.high=m_Day[1].high;
    m_toDay.low=m_Day[1].low;

    updateUI();

    //更新曲线
    ui->label_HigCurve->update();
    ui->label_LowCurve->update();

}

void MainWindow::updateUI()
{
    ui->label_date->setText(QDateTime::fromString(m_toDay.date,"yyyyMMdd").toString("yyyy/MM/dd")+ " "+m_Day[1].week);
    ui->label_city->setText(m_toDay.city);

    //today
    ui->label_Temp->setText(QString::number(m_toDay.wendu)+"°");
    ui->label_LowHigh->setText(QString::number(m_toDay.low)+"°~"+QString::number(m_toDay.high)+"°");
    ui->label_Type->setText(m_toDay.type);
    ui->label_ganmao->setText("感冒指数: "+m_toDay.ganmao);
    ui->label_WindFX->setText(m_toDay.fx);
    ui->label_WindFI->setText(m_toDay.fl);
    ui->label_PM25->setText(QString::number(m_toDay.pm25));
    ui->label_ShiDu->setText(m_toDay.shidu);
    ui->label_Quality->setText(m_toDay.quality);

    ui->label_typeIco->setPixmap(m_typeMap[m_toDay.type]);

    //6 days
    QStringList strlist;
    for (int i = 0; i < 6; i++) {
        m_weekList[i]->setText("周"+m_Day[i].week.right(1));
        strlist=m_Day[i].date.split("-");
        m_dateList[i]->setText(strlist[1]+"/"+strlist[2]);

        m_typeList[i]->setText(m_Day[i].type);
        m_typeIconList[i]->setPixmap(m_typeMap[m_Day[i].type]);

        //aqi
        if(m_Day[i].aqi>=0&&m_Day[i].aqi<=50)
        {
            m_aqiList[i]->setText("优");
            m_aqiList[i]->setStyleSheet("background-color: rgb(121, 184, 0);");
        }else if(m_Day[i].aqi>50&&m_Day[i].aqi<=100)
        {
            m_aqiList[i]->setText("良");
            m_aqiList[i]->setStyleSheet("background-color: rgb(255, 187, 23);");
        }else if(m_Day[i].aqi>100&&m_Day[i].aqi<=150)
        {
            m_aqiList[i]->setText("轻度");
            m_aqiList[i]->setStyleSheet("background-color: rgb(255, 87, 97);");
        }else if(m_Day[i].aqi>150&&m_Day[i].aqi<=200)
        {
            m_aqiList[i]->setText("中度");
            m_aqiList[i]->setStyleSheet("background-color: rgb(235, 17, 27);");
        }else if(m_Day[i].aqi>200&&m_Day[i].aqi<=250)
        {
            m_aqiList[i]->setText("重度");
            m_aqiList[i]->setStyleSheet("background-color: rgb(170, 0, 0);");
        }else{
            m_aqiList[i]->setText("严重");
            m_aqiList[i]->setStyleSheet("background-color: rgb(110, 0, 0);");
        }

        //fx and fl
        m_fxList[i]->setText(m_Day[i].fx);
        m_flList[i]->setText(m_Day[i].fl);
    }

    ui->label_Week0->setText("昨天");
    ui->label_Week1->setText("今天");
    ui->label_Week2->setText("明天");

}

void MainWindow::onReplied(QNetworkReply *reply)
{
    //qDebug() << "onReplied success";

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

//    qDebug() << "operation: " << reply->operation();
//    qDebug() << "status code: " << statusCode;
//    qDebug() << "url: " << reply->url();
//    qDebug() << "raw header: " << reply->rawHeaderList();

    if(reply->error()!=QNetworkReply::NoError|| statusCode!=200)
    {
        //qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }
    else
    {
        QByteArray byteArray=reply->readAll();
        //             qDebug() << "read all: " << byteArray.data();
        parseJson(byteArray);


    }
    reply->deleteLater();
}


void MainWindow::on_pushButton_serch_clicked()
{
    QString cityName=ui->lineEdit_city->text();
    getCityWeather(cityName);
}


void MainWindow::on_lineEdit_city_returnPressed()
{
    QString cityName=ui->lineEdit_city->text();
    getCityWeather(cityName);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->label_HigCurve);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);
    //获取坐标
    int pointX[6]={0};
    for(int i=0;i<6;i++)
    {
        pointX[i]=m_weekList[i]->pos().x()+m_weekList[i]->width()/2;
    }

    int pointY[6]={0};
    int tempSum=0;
    int tempAverage=0;
    for(int i=0;i<6;i++)
    {
        tempSum+=m_Day[i].high;
    }
    tempAverage=tempSum/6;
    int yCenter=ui->label_HigCurve->height()/2;
    for(int i=0;i<6;i++)
    {
        pointY[i]=yCenter-(m_Day[i].high-tempAverage)*INCREMENT;
    }

    //绘制
    QPen pen=painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255,170,0));
    painter.setPen(pen);
    painter.setBrush(QColor(255,170,0));//画刷，内部填充的颜色

    //点
    for(int i=0;i<6;i++)
    {
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        painter.drawText(QPoint(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y),QString::number(m_Day[i].high)+"°");
    }

    //线
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    painter.drawLine(pointX[0],pointY[0],pointX[1],pointY[1]);
    for(int i=1;i<5;i++)
    {
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }

}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->label_LowCurve);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);
    //获取坐标
    int pointX[6]={0};
    for(int i=0;i<6;i++)
    {
        pointX[i]=m_weekList[i]->pos().x()+m_weekList[i]->width()/2;
    }

    int pointY[6]={0};
    int tempSum=0;
    int tempAverage=0;
    for(int i=0;i<6;i++)
    {
        tempSum+=m_Day[i].low;
    }
    tempAverage=tempSum/6;
    int yCenter=ui->label_LowCurve->height()/2;
    for(int i=0;i<6;i++)
    {
        pointY[i]=yCenter-(m_Day[i].low-tempAverage)*INCREMENT;
    }

    //绘制
    QPen pen=painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0,255,255));
    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));//画刷，内部填充的颜色

    //点
    for(int i=0;i<6;i++)
    {
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        painter.drawText(QPoint(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y),QString::number(m_Day[i].low)+"°");
    }

    //线
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    painter.drawLine(pointX[0],pointY[0],pointX[1],pointY[1]);
    for(int i=1;i<5;i++)
    {
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->label_HigCurve&&event->type() == QEvent::Paint)
        paintHighCurve();

    if(watched==ui->label_LowCurve&&event->type() == QEvent::Paint)
        paintLowCurve();

    return QWidget::eventFilter(watched,event);
}
