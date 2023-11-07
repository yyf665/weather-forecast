#ifndef WEATHERDATA_H
#define WEATHERDATA_H

class ToDay{
public:
    ToDay(){
        date="2023-09-02";
        city= "北京市";

        ganmao="极少数敏感人群应减少户外活动";

        shidu="81%";
        pm25=24;
        quality="良";
        wendu=25;

        fx="南风";
        fl="2级";
        type="晴";

        high=30;
        low=19;
    }

    QString date;
    QString city;

    QString ganmao;

    int wendu;
    int pm25;
    QString shidu;
    QString quality;

    QString type;

    QString fx;
    QString fl;

    int high;
    int low;
};

class Day{
public:
   Day(){
        date="2023-09-03";
        week="周日";

        type="晴";

        fx="南风";
        fl="2级";

        high=0;
        low=0;

        aqi=0;
    }

    QString date;
    QString week;

    QString type;

    QString fx;
    QString fl;

    int high;
    int low;

    int aqi;
};

#endif // WEATHERDATA_H
