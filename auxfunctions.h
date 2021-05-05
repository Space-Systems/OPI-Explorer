#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include <QString>
#include <QDateTime>

#include "OPI/opi_cpp.h"

class AuxFunctions
{
public:
    AuxFunctions();
    double dateStringToJulian(QString dateString);
    //QString timeStringFromJulianDay(double julianDay);
    //QDateTime qDateTimeFromJulianDay(double julianDay);

    OPI::JulianDay dateStringToOPIJulian(QString dateString);
    QString timeStringFromJulianDay(OPI::JulianDay julianDay);
    QDateTime qDateTimeFromJulianDay(OPI::JulianDay julianDay);

    OPI::JulianDay qDateTimeToOPIJulian(QDateTime date);

private:
    const std::vector<QString> dateFormats = {
        "yyyy-MM-dd",
        "yyyy/MM/dd",
        "yyyy-M-d",
        "yyyy/M/d",
        "dd-MM-yyyy",
        "dd/MM/yyyy",
        "d-M-yyyy",
        "d/M/yyyy"
    };
    const std::vector<QString> timeFormats = {
      "THH:mm:ss.zzz",
      " HH:mm:ss.zzz",
      "THH:mm:ss.zzz000",
      " HH:mm:ss.zzz000",
      "THH:mm:ss",
      " HH:mm:ss",
      ""
    };
};

#endif // AUXFUNCTIONS_H
