#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "weatherdata.h"


#include <QLabel>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void getCityWeather(QString cityname);

    void parseJson(QByteArray& byteArray);
    void updateUI();

private slots:
    void onReplied(QNetworkReply* reply);

    void on_pushButton_serch_clicked();

    void on_lineEdit_city_returnPressed();

    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();

private:
    QMenu* m_exitMenu;
    QAction* m_exitAct;

    QPoint mOffSet; //窗口移动时，鼠标与窗口左上角的偏移
    QNetworkAccessManager* m_netAccessManager;

    ToDay m_toDay;
    Day m_Day[6];

    //week and date
    QList<QLabel*> m_weekList;
    QList<QLabel*> m_dateList;

    //type and typeicon
    QList<QLabel*> m_typeList;
    QList<QLabel*> m_typeIconList;

    //aqi
    QList<QLabel*> m_aqiList;

    //fx and fl
    QList<QLabel*> m_fxList;
    QList<QLabel*> m_flList;

    QMap<QString, QString> m_typeMap;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};


#endif // MAINWINDOW_H
