#include "dataparser.h"
#include "ui_dataparser.h"
#include "auxfunctions.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <QSettings>

DataParser::DataParser(OPI::Host* hostPointer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataParser)
{    
    ui->setupUi(this);

    host = hostPointer;
    population = new OPI::Population(*host,0);

    QSettings settings("opi-explorer.ini", QSettings::NativeFormat);
    settings.beginGroup("ParseProfiles");
    QStringList profiles = settings.childKeys();
    for (int i=0; i<profiles.size(); i++)
    {
        ui->cmbParseProfiles->addItem(settings.value(profiles[i]).toString());
    }
}

DataParser::~DataParser()
{
    delete ui;
    //FIXME: Why does this crash?
    //delete population;
}

void DataParser::on_btnAnalyse_clicked()
{
    QRegExp rx(rxFullProfile);
    rx.indexIn(ui->cmbParseProfiles->currentText());
    //QString profileName = rx.cap(1);
    QString separator = rx.cap(2);
    QString parseProfile = rx.cap(3);

    QStringList identifiers;
    QStringList modifiers;
    QStringList defaults;

    QStringList components = parseProfile.split(separator);
    for (int i=0; i<components.size(); i++)
    {
        QString item = components[i].trimmed();
        if (item.startsWith('%'))
        {
            QStringList temp = item.split("*");
            if (temp.size() > 1)
            {
                identifiers.append(temp[0]);
                modifiers.append(temp[1]);
                defaults.append("");
            }
            else {
                temp = item.split("=");
                if (temp.size() > 1)
                {
                    identifiers.append(temp[0]);
                    defaults.append(temp[1]);
                    modifiers.append("");
                }
                else {
                    identifiers.append(item);
                    defaults.append("");
                    modifiers.append("");
                }
            }
        }
    }

    AuxFunctions aux;
    QStringList lines = ui->teParsablePopulation->toPlainText().split("\n",QString::SkipEmptyParts);
    if (population) delete population;
    population = new OPI::Population(*host, lines.size());
    QString parsedText;

    for (int l=0; l<lines.size(); l++)
    {
        population->getOrbit()[l] = {0.0,0.0,0.0,0.0,0.0,0.0};
        population->getObjectProperties()[l] = {0.0,0.0,0.0,0.0,0.0,0};
        population->getPosition()[l] = {0.0,0.0,0.0};
        population->getVelocity()[l] = {0.0,0.0,0.0};
        population->getEpoch()[l] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
        population->getAcceleration()[l] = {0.0,0.0,0.0};

        QStringList items = lines[l].split(separator);
        for (int i=0; i<std::min(items.size(),identifiers.size()); i++)
        {
            if (defaults[i] != "") items[i] = defaults[i];
            if (identifiers[i] == "%id")
            {
                population->getObjectProperties()[l].id = items[i].toInt();
                parsedText += "id = " + QString::number(population->getObjectProperties()[l].id) + ", ";
            }
            else if (identifiers[i] == "%nm")
            {
                population->setObjectName(l, items[i].toStdString().c_str());
                parsedText += "nm = " + items[i] + ", ";
            }
            else if (identifiers[i] == "%m")
            {
                population->getObjectProperties()[l].mass = items[i].toDouble();
                parsedText += "m = " + QString::number(population->getObjectProperties()[l].mass) + ", ";
            }
            else if (identifiers[i] == "%d")
            {
                population->getObjectProperties()[l].diameter = items[i].toDouble();
                parsedText += "d = " + QString::number(population->getObjectProperties()[l].diameter) + ", ";
            }
            else if (identifiers[i] == "%am")
            {
                population->getObjectProperties()[l].area_to_mass = items[i].toDouble();
                parsedText += "am = " + QString::number(population->getObjectProperties()[l].area_to_mass) + ", ";
            }
            else if (identifiers[i] == "%cd")
            {
                population->getObjectProperties()[l].drag_coefficient = items[i].toDouble();
                parsedText += "cd = " + QString::number(population->getObjectProperties()[l].drag_coefficient) + ", ";
            }
            else if (identifiers[i] == "%cr")
            {
                population->getObjectProperties()[l].reflectivity = items[i].toDouble();
                parsedText += "cr = " + QString::number(population->getObjectProperties()[l].reflectivity) + ", ";
            }
            else if (identifiers[i] == "%rx")
            {
                population->getPosition()[l].x = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "rx = " + QString::number(population->getPosition()[l].x) + ", ";
            }
            else if (identifiers[i] == "%ry")
            {
                population->getPosition()[l].y = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "ry = " + QString::number(population->getPosition()[l].y) + ", ";
            }
            else if (identifiers[i] == "%rz")
            {
                population->getPosition()[l].z = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "rz = " + QString::number(population->getPosition()[l].z) + ", ";
            }
            else if (identifiers[i] == "%vx")
            {
                population->getVelocity()[l].x = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "vx = " + QString::number(population->getVelocity()[l].x) + ", ";
            }
            else if (identifiers[i] == "%vy")
            {
                population->getVelocity()[l].y = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "vy = " + QString::number(population->getVelocity()[l].y) + ", ";
            }
            else if (identifiers[i] == "%vz")
            {
                population->getVelocity()[l].z = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "vz = " + QString::number(population->getVelocity()[l].z) + ", ";
            }
            else if (identifiers[i] == "%a")
            {
                population->getOrbit()[l].semi_major_axis = (modifiers[i] == "m" ? items[i].toDouble()/1000.0 : items[i].toDouble());
                parsedText += "a = " + QString::number(population->getOrbit()[l].semi_major_axis) + ", ";
            }
            else if (identifiers[i] == "%e")
            {
                population->getOrbit()[l].eccentricity = items[i].toDouble();
                parsedText += "e = " + QString::number(population->getOrbit()[l].eccentricity) + ", ";
            }
            else if (identifiers[i] == "%i")
            {
                population->getOrbit()[l].inclination = (modifiers[i] == "d" ? items[i].toDouble()*M_PI/180.0 : items[i].toDouble());
                parsedText += "i = " + QString::number(population->getOrbit()[l].inclination) + ", ";
            }
            else if (identifiers[i] == "%r")
            {
                population->getOrbit()[l].raan = (modifiers[i] == "d" ? items[i].toDouble()*M_PI/180.0 : items[i].toDouble());
                parsedText += "r = " + QString::number(population->getOrbit()[l].raan) + ", ";
            }
            else if (identifiers[i] == "%aop")
            {
                population->getOrbit()[l].arg_of_perigee = (modifiers[i] == "d" ? items[i].toDouble()*M_PI/180.0 : items[i].toDouble());
                parsedText += "aop = " + QString::number(population->getOrbit()[l].arg_of_perigee) + ", ";
            }
            else if (identifiers[i] == "%ma")
            {
                population->getOrbit()[l].mean_anomaly = (modifiers[i] == "d" ? items[i].toDouble()*M_PI/180.0 : items[i].toDouble());
                parsedText += "ma = " + QString::number(population->getOrbit()[l].mean_anomaly) + ", ";
            }
            else if (identifiers[i] == "%ep")
            {
                population->getEpoch()[l].current_epoch = aux.dateStringToOPIJulian(items[i]);
                parsedText += "ep = " + QString::number(OPI::toDouble(population->getEpoch()[l].current_epoch)) + ", ";
            }
            else if (identifiers[i] == "%x")
            {
                int skip = modifiers[i].toInt();
                if (skip>0) i += skip-1;
            }
        }
        parsedText += "\n";
    }
    ui->teParsablePopulation->setText(parsedText);
    //population->write("parsetest.opi");
}

void DataParser::on_btnSaveProfile_clicked()
{
    QSettings settings("opi-explorer.ini", QSettings::NativeFormat);
    settings.beginGroup("ParseProfiles");
    QRegExp rx(rxFullProfile);
    QString profile = ui->cmbParseProfiles->currentText();
    rx.indexIn(profile);
    QString profileName = rx.cap(1);
    if (profileName != "")
    {
        settings.setValue(profileName,profile);
    }
    settings.endGroup();
}
