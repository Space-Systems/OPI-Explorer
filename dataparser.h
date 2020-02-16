#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <QDialog>

#include "OPI/opi_cpp.h"

namespace Ui {
class DataParser;
}

class DataParser : public QDialog
{
    Q_OBJECT

public:
    explicit DataParser(OPI::Host* hostPointer, QWidget *parent = 0);
    ~DataParser();

    inline OPI::Population* getPopulation() { return population; }

private slots:
    void on_btnAnalyse_clicked();

    void on_btnSaveProfile_clicked();

private:
    const QString rxFullProfile = "\\[(.+)\\]\\[(.)\\]\\[(.+)\\]";
    Ui::DataParser *ui;
    OPI::Host* host;
    OPI::Population* population;
};

#endif // DATAPARSER_H
