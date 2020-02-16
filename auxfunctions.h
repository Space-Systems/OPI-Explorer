#ifndef AUXFUNCTIONS_H
#define AUXFUNCTIONS_H

#include <QString>
#include <QDateTime>


class AuxFunctions
{
public:
    AuxFunctions();
    double dateStringToJulian(QString dateString);
    QString timeStringFromJulianDay(double julianDay);
};

#endif // AUXFUNCTIONS_H
