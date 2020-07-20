#ifndef OPIPOPIMAIN_H
#define OPIPOPIMAIN_H

#include <QMainWindow>
#include <QLineEdit>
#include <QtCharts/QChartView>

#include "glcanvas.h"

#define OPI_DISABLE_OPENCL //Not required for host
#include "OPI/opi_cpp.h"

namespace Ui {
class OpiExplorerMain;
}

class OpiExplorerMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit OpiExplorerMain(QWidget *parent = 0);
    ~OpiExplorerMain();

private slots:
    void on_actionLoad_Population_triggered();

    void on_leStateX_textChanged(const QString &arg1);

    void on_listObjects_currentRowChanged(int currentRow);

    void on_listPProps_currentRowChanged(int currentRow);

    void on_listPlugins_currentRowChanged(int currentRow);

    void on_actionSave_as_triggered();

    void on_actionSave_Population_triggered();

    void on_actionSet_Plugin_Directory_triggered();

    void on_leStateY_textChanged(const QString &arg1);

    void on_leStateZ_textChanged(const QString &arg1);

    void on_leStateXdot_textChanged(const QString &arg1);

    void on_leStateYdot_textChanged(const QString &arg1);

    void on_leStateZdot_textChanged(const QString &arg1);

    void on_actionQuit_triggered();

    void on_actionAdd_Object_triggered();

    void on_leStateX_editingFinished();

    void on_leStateY_editingFinished();

    void on_leStateZ_editingFinished();

    void on_leStateXdot_editingFinished();

    void on_leStateYdot_editingFinished();

    void on_leStateZdot_editingFinished();

    void on_lePropsID_editingFinished();

    void on_lePropsMass_editingFinished();

    void on_lePropsDia_editingFinished();

    void on_lePropsA2M_editingFinished();

    void on_lePropsDrag_editingFinished();

    void on_lePropsReflectivity_editingFinished();

    void on_leOrbitSMA_editingFinished();

    void on_leOrbitEcc_editingFinished();

    void on_leOrbitInc_editingFinished();


    void on_leOrbitRAAN_editingFinished();

    void on_leOrbitAOP_editingFinished();

    void on_leOrbitMA_editingFinished();

    void on_actionLoad_with_Plugin_triggered();

    void on_btnPropagate_clicked();

    void on_btnState2Orbit_clicked();

    void on_btnOrbit2State_clicked();

    void on_btnAddObject_clicked();

    void on_actionDelete_Object_triggered();

    void on_btnDeleteObject_clicked();

    void on_actionOrbits_to_State_Vectors_triggered();

    void on_actionState_Vectors_to_Orbits_triggered();

    void actionSelectPropagator();

    void on_leObjectName_editingFinished();

    void on_leEpochBOL_editingFinished();

    void on_leEpochEOL_editingFinished();

    void on_leAccX_editingFinished();

    void on_leAccY_editingFinished();

    void on_leAccZ_editingFinished();

    void on_leCurrentPropertyValue_editingFinished();

    void on_lePropsID_textChanged(const QString &arg1);

    void on_lePropsMass_textChanged(const QString &arg1);

    void on_lePropsDia_textChanged(const QString &arg1);

    void on_lePropsA2M_textChanged(const QString &arg1);

    void on_lePropsDrag_textChanged(const QString &arg1);

    void on_lePropsReflectivity_textChanged(const QString &arg1);

    void on_leOrbitSMA_textChanged(const QString &arg1);

    void on_leOrbitEcc_textChanged(const QString &arg1);

    void on_leOrbitInc_textChanged(const QString &arg1);

    void on_leOrbitRAAN_textChanged(const QString &arg1);

    void on_leOrbitAOP_textChanged(const QString &arg1);

    void on_leOrbitMA_textChanged(const QString &arg1);

    void on_leEpochBOL_textChanged(const QString &arg1);

    void on_leEpochEOL_textChanged(const QString &arg1);

    void on_leObjectName_textChanged(const QString &arg1);

    void on_leAccX_textChanged(const QString &arg1);

    void on_leAccY_textChanged(const QString &arg1);

    void on_leAccZ_textChanged(const QString &arg1);

    void on_actionLoad_and_Append_triggered();

    void on_btnCopyState_clicked();

    void on_btnCopyOrbit_clicked();

    void on_btnCopyProps_clicked();

    void on_actionNew_Population_triggered();

    void on_actionReset_to_Saved_State_triggered();

    void on_leEpochCurrent_editingFinished();

    void on_leEpochCurrent_textChanged(const QString &arg1);

    void on_btnToRadians_clicked();

    void on_btnCopyEpoch_clicked();

    void on_btnToJulianDate_clicked();

    void on_actionOpen_Data_Parser_triggered();

    void on_btnAlign_clicked();

    void on_actionAlign_to_Latest_Object_triggered();

    void on_tabsMain_currentChanged(int index);

    void on_btnLogRefresh_clicked();

    void on_teDescription_textChanged();

    void on_leCov_0_textChanged(const QString &arg1);


    void on_leCov_0_editingFinished();

    void on_leCov_8_editingFinished();

    void on_leCov_9_editingFinished();

    void on_leCov_16_editingFinished();

    void on_leCov_17_editingFinished();

    void on_leCov_18_editingFinished();

    void on_leCov_24_editingFinished();

    void on_leCov_25_editingFinished();

    void on_leCov_26_editingFinished();

    void on_leCov_27_editingFinished();

    void on_leCov_32_editingFinished();

    void on_leCov_33_editingFinished();

    void on_leCov_34_editingFinished();

    void on_leCov_35_editingFinished();

    void on_leCov_36_editingFinished();

    void on_leCov_40_editingFinished();

    void on_leCov_41_editingFinished();

    void on_leCov_42_editingFinished();

    void on_leCov_43_editingFinished();

    void on_leCov_44_editingFinished();

    void on_leCov_45_editingFinished();

    void on_leCov_48_editingFinished();

    void on_leCov_49_editingFinished();

    void on_leCov_50_editingFinished();

    void on_leCov_51_editingFinished();

    void on_leCov_52_editingFinished();

    void on_leCov_53_editingFinished();

    void on_leCov_54_editingFinished();

    void on_leCov_56_editingFinished();

    void on_leCov_57_editingFinished();

    void on_leCov_58_editingFinished();

    void on_leCov_59_editingFinished();

    void on_leCov_60_editingFinished();

    void on_leCov_61_editingFinished();

    void on_leCov_62_editingFinished();

    void on_leCov_63_editingFinished();

    void on_btnToKm_clicked();

    void on_actionExport_to_JSON_triggered();

    void on_btnToKmCov_clicked();

    void on_btnCopyCov_clicked();

    void on_cmbRefFrame_currentIndexChanged(int index);

    void on_leEpochOriginal_editingFinished();

    void on_leEpochOriginal_textChanged(const QString &arg1);

    void on_btnClearPlot_clicked();

private:
    const int precision = 15;

    Ui::OpiExplorerMain *ui;
#ifdef ENABLE_OPENGL
    GLCanvas* gl;
#endif
    OPI::Host host;
    OPI::Population* currentPopulation;

    QString currentPopulationFile;

    QVector<QLineEdit*> inputBoxes;
    QVector<QLineEdit*> covBoxes;
    QVector<QLineEdit*> covBoxesLowerTriangular;

    QtCharts::QChartView* cvSMA;
    QtCharts::QChartView* cvEcc;
    QtCharts::QChartView* cvInc;
    QtCharts::QChartView* cvRAAN;

    void loadPlugins(QString pluginFolder);
    void pasteState(int editBoxIndex);
    bool validObjectSelected();
    void updateObjects();
    QString refFrameToString(OPI::ReferenceFrame rf);
    void resetInputBoxes();
    void updateWindowTitle();
    int diagonalIndex(int i);
    void setStartEndDate();
};

#endif // OPIPOPIMAIN_H
