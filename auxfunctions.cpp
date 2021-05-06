#include "auxfunctions.h"

#include <vector>

AuxFunctions::AuxFunctions()
{

}

/*
double AuxFunctions::dateStringToJulian(QString dateString)
{
    double julianDate = 0.0;
    QDateTime date;
    for (int df=0; df<dateFormats.size(); df++)
    {
        for (int tf=0; tf<timeFormats.size(); tf++)
        {
            QString format = dateFormats[df]+timeFormats[tf];
            date = QDateTime::fromString(dateString,format);
            if (date.isValid()) {
                julianDate = date.date().toJulianDay() - 0.5 + date.time().msecsSinceStartOfDay()/86400000.0;
                break;
            }
        }
    }
    return julianDate;
}

QDateTime AuxFunctions::qDateTimeFromJulianDay(double julianDay)
{
    qint64 jd = (qint64)(qRound(julianDay));
    QDate date = QDate::fromJulianDay(jd);
    double jdfraction = julianDay - (int)julianDay;
    if (jdfraction >= 0.5) jdfraction -= 0.5;
    int msecs = (jdfraction * 24.0 * 1000.0) * 3600;
    QTime time = QTime::fromMSecsSinceStartOfDay(msecs);
    QDateTime datetime(date, time);
    return datetime;
}

QString AuxFunctions::timeStringFromJulianDay(double julianDay)
{    
    return qDateTimeFromJulianDay(julianDay).toString("yyyy-MM-ddTHH:mm:ss");
}
*/

OPI::JulianDay AuxFunctions::dateStringToOPIJulian(QString dateString)
{
    OPI::JulianDay julianDate = {0,0};
    QDateTime date;
    for (int df=0; df<dateFormats.size(); df++)
    {
        for (int tf=0; tf<timeFormats.size(); tf++)
        {
            QString format = dateFormats[df]+timeFormats[tf];
            date = QDateTime::fromString(dateString,format);
            if (date.isValid()) {
                julianDate = qDateTimeToOPIJulian(date);
                break;
            }
        }
    }
    return julianDate;
}

OPI::JulianDay AuxFunctions::qDateTimeToOPIJulian(QDateTime date)
{
    OPI::JulianDay result = { (int)date.date().toJulianDay(), (long long)(date.time().msecsSinceStartOfDay()) * 1000 };
    result -= 43200000000l;
    return result;
}

QDateTime AuxFunctions::qDateTimeFromJulianDay(OPI::JulianDay julianDay)
{
    julianDay += 43200000000l;
    QDate date = QDate::fromJulianDay(julianDay.day);
    QTime time = QTime::fromMSecsSinceStartOfDay(julianDay.usec / 1000);
    QDateTime datetime(date, time);
    return datetime;
}

QString AuxFunctions::timeStringFromJulianDay(OPI::JulianDay julianDay)
{
    return qDateTimeFromJulianDay(julianDay).toString("yyyy-MM-ddTHH:mm:ss.zzz");
}
