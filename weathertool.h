#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include<QMap>
#include<QFile>

#include<QJsonObject>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonParseError>
#include<QJsonValue>

class WeatherTool{

private:
    static QMap<QString, QString> m_cityMap;
    static void initCityMap(){
        //QString str="E:/citycode.json";
        QString str=":/pic/citycode.json";
        QFile file(str);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
            qDebug()<<"文件打开失败";
        QByteArray json=file.readAll();
        file.close();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json,&err);
        if(err.error!=QJsonParseError::NoError)
            return;
        //qDebug() <<"1";
        if(!doc.isArray())
            return;
        QJsonArray citys=doc.array();
        //qDebug() <<"2";
        for(int i=0; i < citys.size(); i++)
        {
            QString city=citys[i].toObject().value("city_name").toString();
            QString code=citys[i].toObject().value("city_code").toString();
            if(code.size()>0){
                m_cityMap.insert(city,code);
                //qDebug()<<"insert success";
            }
        }
    }

public:
    static QString getCityCode(QString cityName){
    if(m_cityMap.isEmpty()){
        initCityMap();
    }

    QMap<QString, QString>::iterator it= m_cityMap.find(cityName);
    if(it==m_cityMap.end())
        it=m_cityMap.find(cityName+"市");
    if(it!=m_cityMap.end())
        return it.value();
    return "";
    }
};
QMap<QString, QString> WeatherTool::m_cityMap = {};






#endif // WEATHERTOOL_H
