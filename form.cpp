#include "form.h"
#include "ui_form.h"
#include <QDebug>
#include <QWebFrame>
#include <QWebElement>
#include <QMessageBox>
#include <qjson-qt5/parser.h>



Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    connect(ui->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
    //connect(ui->pbTrajectory, SIGNAL(clicked()), this, SLOT(pbTrajectoryClicked()));
    connect(ui->lePostalAddress, SIGNAL(returnPressed()), this, SLOT(goClicked()));

    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double)), this, SLOT(showCoordinates(double,double)));
    connect(&m_geocodeDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));

    connect(&m_geocodeDataManager, SIGNAL(addressReady(QString)), ui->lePostalAddress, SLOT(setText(QString)));

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->lePostalAddress->setText("");
    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
}

Form::~Form()
{
    delete ui;
}


void Form::showCoordinates(double east, double north, bool saveMarker)
{
    qDebug() << "Form, showCoordinates" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, ui->lePostalAddress->text());
}

void Form::setMarker(double east, double north, QString caption)
{
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption) return;
    }

    QString str =
            QString("var marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("map: map,") +
  //          QString("draggable: true,") +
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, caption);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
}

void Form::goClicked()
{
    QString address = ui->lePostalAddress->text();
    m_geocodeDataManager.getCoordinates(address.replace(" ", "+"));
}

void Form::errorOccured(const QString& error)
{
    QMessageBox::warning(this, tr("Geocode Error"), error);
}



void Form::on_lwMarkers_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(m_markers[currentRow]->north).arg(m_markers[currentRow]->east) +
            QString("map.setCenter(newLoc);");

    qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void Form::on_pbRemoveMarker_clicked()
{
    if (ui->lwMarkers->currentRow() < 0) return;

    QString str =
            QString("markers[%1].setMap(null); markers.splice(%1, 1);").arg(ui->lwMarkers->currentRow());
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    //deleteing caption from markers list
    delete m_markers.takeAt(ui->lwMarkers->currentRow());
    //deleteing caption from ListWidget
    delete ui->lwMarkers->takeItem(ui->lwMarkers->currentRow());

}

void Form::on_pbTrajectory_clicked()
{
    qDebug() << "pbTrajectory clicked";
   // QVariant path = ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript("trajectory.getPath()");

   // qDebug() << path;

   // QJson::Parser parser;

   // bool ok;

 //   // json is a QString containing the data to convert
 //   QVariant result = parser.parse (result.toLatin1(), &ok);
 //   if(!ok)
 //   {
 //       qDebug() << QString("Cannot convert to QJson object: %1").arg(result);
 //       return;
 //   }
 //   qDebug() << "Success";

    //QString code = result.toMap()["status"].toString();
    //qDebug() << "Code" << code;
    //if(code != "OK")
    //{
    //    emit errorOccured(QString("Code of request is: %1").arg(code));
    //    return;
    //}

    //QVariantList results = result.toMap()["results"].toList();
    //if(results.count() == 0)
    //{
    //    emit errorOccured(QString("Cannot find any locations"));
    //    return;
    //}

    //double east  = results[0].toMap()["geometry"].toMap()["location"].toMap()["lng"].toDouble();
    //double north = results[0].toMap()["geometry"].toMap()["location"].toMap()["lat"].toDouble();
}

void Form::on_zoomSpinBox_valueChanged(int arg1)
{
    QString str =
            QString("map.setZoom(%1);").arg(arg1);
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}
