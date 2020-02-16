#include "auxfunctions.h"

#include <vector>

AuxFunctions::AuxFunctions()
{

}

double AuxFunctions::dateStringToJulian(QString dateString)
{
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

QString AuxFunctions::timeStringFromJulianDay(double julianDay)
{
    qint64 jd = (qint64)(qRound(julianDay));
    QDate date = QDate::fromJulianDay(jd);
    double jdfraction = julianDay - (int)julianDay;
    if (jdfraction >= 0.5) jdfraction -= 0.5;
    int msecs = (jdfraction * 24.0 * 1000.0) * 3600;
    QTime time = QTime::fromMSecsSinceStartOfDay(msecs);
    QDateTime datetime(date, time);
    return datetime.toString("yyyy-MM-ddTHH:mm:ss");
}
