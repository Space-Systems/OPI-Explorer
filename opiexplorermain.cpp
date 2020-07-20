#include "opiexplorermain.h"
#include "ui_opiexplorermain.h"
#include "dataparser.h"
#include "auxfunctions.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QClipboard>
#include <QSettings>
#include <QProgressDialog>
#include <QTextStream>
#include <QDebug>
#include <QtCharts/QLineSeries>

using namespace QtCharts;

#define _USE_MATH_DEFINES
#include <math.h>

OpiExplorerMain::OpiExplorerMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OpiExplorerMain)
{
    ui->setupUi(this);
    host.logToFile("opi-explorer.log");
    loadPlugins("plugins");
    //on_actionSet_Plugin_Directory_triggered();
    currentPopulation = new OPI::Population(host,0);
    currentPopulationFile = "";
    QFont infoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    infoFont.setPointSize(10);
    ui->lblPropInfo->setFont(infoFont);
    ui->teByteArray->setFont(infoFont);
    ui->teOpiLog->setFont(infoFont);
    ui->dtStartTime->setDateTime(QDateTime::currentDateTime());
    ui->dtEndTime->setDateTime(QDateTime::currentDateTime().addDays(1));
    ui->tabsMain->setCurrentIndex(0);
    ui->tabObjects->setCurrentIndex(0);
    ui->listObjects->setFocus();
#ifdef ENABLE_OPENGL
    gl = new GLCanvas(ui->glCanvas);
    gl->setMinimumSize(491, 141);
#endif

    inputBoxes = {ui->leStateX, ui->leStateY, ui->leStateZ, ui->leStateXdot, ui->leStateYdot, ui->leStateZdot,
                  ui->lePropsID, ui->lePropsMass, ui->lePropsDia, ui->lePropsA2M, ui->lePropsDrag, ui->lePropsReflectivity,
                  ui->leOrbitSMA, ui->leOrbitEcc, ui->leOrbitInc, ui->leOrbitRAAN, ui->leOrbitAOP, ui->leOrbitMA,
                  ui->leEpochBOL, ui->leEpochEOL, ui->leEpochCurrent, ui->leEpochOriginal, ui->leObjectName, ui->leAccX, ui->leAccY, ui->leAccZ
                 };
    covBoxes = {ui->leCov_0,  ui->leCov_1,  ui->leCov_2,  ui->leCov_3,  ui->leCov_4,  ui->leCov_5,  ui->leCov_6,  ui->leCov_7,
                ui->leCov_8,  ui->leCov_9,  ui->leCov_10, ui->leCov_11, ui->leCov_12, ui->leCov_13, ui->leCov_14, ui->leCov_15,
                ui->leCov_16, ui->leCov_17, ui->leCov_18, ui->leCov_19, ui->leCov_20, ui->leCov_21, ui->leCov_22, ui->leCov_23,
                ui->leCov_24, ui->leCov_25, ui->leCov_26, ui->leCov_27, ui->leCov_28, ui->leCov_29, ui->leCov_30, ui->leCov_31,
                ui->leCov_32, ui->leCov_33, ui->leCov_34, ui->leCov_35, ui->leCov_36, ui->leCov_37, ui->leCov_38, ui->leCov_39,
                ui->leCov_40, ui->leCov_41, ui->leCov_42, ui->leCov_43, ui->leCov_44, ui->leCov_45, ui->leCov_46, ui->leCov_47,
                ui->leCov_48, ui->leCov_49, ui->leCov_50, ui->leCov_51, ui->leCov_52, ui->leCov_53, ui->leCov_54, ui->leCov_55,
                ui->leCov_56, ui->leCov_57, ui->leCov_58, ui->leCov_59, ui->leCov_60, ui->leCov_61, ui->leCov_62, ui->leCov_63
               };
    covBoxesLowerTriangular = { ui->leCov_0, ui->leCov_8,  ui->leCov_9, ui->leCov_16, ui->leCov_17, ui->leCov_18,
                                ui->leCov_24, ui->leCov_25, ui->leCov_26, ui->leCov_27, ui->leCov_32, ui->leCov_33, ui->leCov_34, ui->leCov_35, ui->leCov_36,
                                ui->leCov_40, ui->leCov_41, ui->leCov_42, ui->leCov_43, ui->leCov_44, ui->leCov_45,
                                ui->leCov_48, ui->leCov_49, ui->leCov_50, ui->leCov_51, ui->leCov_52, ui->leCov_53, ui->leCov_54,
                                ui->leCov_56, ui->leCov_57, ui->leCov_58, ui->leCov_59, ui->leCov_60, ui->leCov_61, ui->leCov_62, ui->leCov_63 };
    resetInputBoxes();
    QSettings settings("opi-explorer.ini", QSettings::NativeFormat);
    ui->dtStartTime->setDateTime(QDateTime::fromString(settings.value("lastStartDate").toString()));
    ui->dtEndTime->setDateTime(QDateTime::fromString(settings.value("lastEndDate").toString()));


    QVBoxLayout* plotGrid = new QVBoxLayout();

    int numPlots = 5;
    for (int i=0; i<numPlots; i++)
    {
        plotViews.push_back(new QChartView);
        plotViews[i]->setRubberBand(QChartView::RectangleRubberBand);
        plotViews[i]->setMinimumHeight(240);
        plotGrid->addWidget(plotViews[i]);
    }

    ui->scrollAreaWidgetContents->setLayout(plotGrid);
    ui->scrollAreaWidgetContents->setMinimumHeight(240*numPlots);
}

OpiExplorerMain::~OpiExplorerMain()
{
    QSettings settings("opi-explorer.ini", QSettings::NativeFormat);
    settings.setValue("lastStartDate",ui->dtStartTime->dateTime().toString());
    settings.setValue("lastEndDate",ui->dtEndTime->dateTime().toString());
#ifdef ENABLE_OPENGL
    delete gl;
#endif
    delete ui;
    delete currentPopulation;
}

void OpiExplorerMain::resetInputBoxes()
{
    for (int i=0; i<inputBoxes.size(); i++)
    {
        inputBoxes[i]->setText("");
        inputBoxes[i]->setReadOnly(true);
    }
    for (int i=0; i<covBoxes.size(); i++)
    {
        covBoxes[i]->setText("");
        covBoxes[i]->setReadOnly(true);
    }
    ui->leLastPropagator->setText("");
    ui->leEpochEarliest->setText("");
    ui->leEpochLatest->setText("");
    ui->grpByteArray->setTitle("Bytes");
    ui->grpIssues->setTitle("Issues");
    ui->teByteArray->clear();
    ui->teReport->clear();
    updateWindowTitle();
}


void OpiExplorerMain::loadPlugins(QString pluginFolder)
{
    host.loadPlugins(pluginFolder.toStdString().c_str(), OPI::Host::PLATFORM_OPENCL, 0, 0);
    ui->listPlugins->clear();
    for (int i=0; i<host.getPropagatorCount(); i++)
    {        
        OPI::Propagator* p = host.getPropagator(i);
        QString pName = QString(p->getName());
        QListWidgetItem* propagatorItem = new QListWidgetItem;
        propagatorItem->setText(pName);
        propagatorItem->setIcon(QPixmap(":/icons/opi_logo_plugin_square.svg"));
        ui->listPlugins->addItem(propagatorItem);
        QAction* pluginSelect = ui->menuSelect_Propagator->addAction(pName);
        connect(pluginSelect,SIGNAL(triggered()),this,SLOT(actionSelectPropagator()));
    }
}

void OpiExplorerMain::actionSelectPropagator()
{
    for (int i=0; i<ui->listPlugins->count(); i++)
    {
        if (ui->listPlugins->item(i)->text() == (qobject_cast<QAction*>(sender()))->text())
        {
            ui->listPlugins->setCurrentRow(i);
            break;
        }
    }
}

void OpiExplorerMain::on_actionLoad_Population_triggered()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList filters;
    filters << "OPI Population Files (*.opi)";
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Open OPI Population");
    dialog.setDirectory(".");
    if (dialog.exec())
    {
        currentPopulationFile = dialog.selectedFiles()[0];
        delete currentPopulation;
        currentPopulation = new OPI::Population(host,0);
        currentPopulation->read(currentPopulationFile.toStdString().c_str());
        updateObjects();
        ui->tabsMain->setTabText(0, "Population");
        ui->actionReset_to_Saved_State->setEnabled(true);
        setStartEndDate();
    }
    updateWindowTitle();
}

void OpiExplorerMain::on_actionLoad_with_Plugin_triggered()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setWindowTitle("Open Propagator Input File (" + ui->listPlugins->currentItem()->text() + ")");
    dialog.setDirectory(".");
    if (dialog.exec())
    {
        delete currentPopulation;
        currentPopulationFile = dialog.selectedFiles()[0];
        currentPopulation = new OPI::Population(host,0);
        OPI::Propagator* loader = host.getPropagator(ui->listPlugins->currentRow());
        if (loader) {
            loader->loadPopulation(*currentPopulation, currentPopulationFile.toStdString().c_str());
            updateObjects();
            ui->tabsMain->setTabText(0, "Population");
            ui->actionReset_to_Saved_State->setEnabled(false);
            setStartEndDate();
        }
    }
    updateWindowTitle();
}


void OpiExplorerMain::on_actionLoad_and_Append_triggered()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList filters;
    filters << "OPI Population Files (*.opi)";
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Append OPI Population");
    dialog.setDirectory(".");
    if (dialog.exec())
    {
        QString file = dialog.selectedFiles()[0];
        OPI::Population additionalObjects = OPI::Population(host,0);
        additionalObjects.read(file.toStdString().c_str());
        if (additionalObjects.getByteArraySize() != currentPopulation->getByteArraySize())
        {
            QMessageBox::StandardButton ignoreByteArray;
              ignoreByteArray = QMessageBox::warning(this, "Byte Array Mismatch", "Warning: The byte array sizes of the two populations are different. This usually means that they have been created with different propagators that stored different per-object data into the population.\n\nIf you continue, OPI will:\n-Add the new objects to the currently loaded population\n-Adjust the byte array size to match the one loaded\n-Set the bytes of the new objects to zero.\n\nDepending on how the propagator uses this additional data, propagation of this population may fail. Data that may have been stored with the new objects will be lost.\n\nDo you wish to continue?", QMessageBox::Yes|QMessageBox::No);
              if (ignoreByteArray == QMessageBox::Yes) {
                    *currentPopulation += additionalObjects;
                    updateObjects();
              }
        }
        else {
            *currentPopulation += additionalObjects;
            updateObjects();
        }
        setStartEndDate();
    }
    updateWindowTitle();
}

void OpiExplorerMain::setStartEndDate()
{
    AuxFunctions aux;
    double current = currentPopulation->getEarliestEpoch();
    if (current > 0)
    {
        QDateTime t = aux.qDateTimeFromJulianDay(current);
        ui->dtStartTime->setDateTime(t);
        ui->dtEndTime->setDateTime(t.addDays(7));
    }
}

void OpiExplorerMain::updateObjects()
{
    int lastIndex = ui->listObjects->currentRow();
    ui->listObjects->clear();
    for (int i=0; i<currentPopulation->getSize(); i++)
    {
        ui->listObjects->addItem(QString::number(currentPopulation->getObjectProperties()[i].id));
    }
    if (ui->listObjects->count() > 0)
    {
        ui->grpByteArray->setTitle("Bytes ("+QString::number(currentPopulation->getByteArraySize())+")");
        ui->tabsMain->setTabText(0, "Population*");
    }
    OPI::IndexList invalids(host);
    ui->teReport->setText(QString::fromStdString(currentPopulation->validate(invalids)));
    if (invalids.getSize() > 0) ui->grpIssues->setTitle("Issues ("+QString::number(invalids.getSize())+")");
    else ui->grpIssues->setTitle("Issues");
    for (int i=0; i<invalids.getSize(); i++)
    {
        ui->listObjects->item(invalids.getData(OPI::DEVICE_HOST)[i])->setForeground(Qt::red);
    }
    if (ui->listObjects->count() > lastIndex) ui->listObjects->setCurrentRow(lastIndex);
    const double earliestEpoch = currentPopulation->getEarliestEpoch();
    const double latestEpoch = currentPopulation->getLatestEpoch();
    ui->leEpochEarliest->setText(QString::number(earliestEpoch,'g',precision));
    ui->leEpochLatest->setText(QString::number(currentPopulation->getLatestEpoch(),'g',precision));
    AuxFunctions aux;
    ui->leEpochEarliest->setToolTip(aux.timeStringFromJulianDay(earliestEpoch));
    ui->leEpochLatest->setToolTip(aux.timeStringFromJulianDay(latestEpoch));
    ui->teDescription->setText(QString(currentPopulation->getDescription()));
    int rfIndex = ui->cmbRefFrame->findText(QString(OPI::referenceFrameToString(currentPopulation->getReferenceFrame())));
    ui->cmbRefFrame->setCurrentIndex(rfIndex);
}

void OpiExplorerMain::updateWindowTitle()
{
    QString windowTitle = "OPI Explorer";
    windowTitle += " (OPI " + QString::number(OPI_API_VERSION_MAJOR) + "." + QString::number(OPI_API_VERSION_MINOR) + ")";
    if (currentPopulationFile != "") windowTitle += " - " + currentPopulationFile;
    QWidget::setWindowTitle(windowTitle);
}

void OpiExplorerMain::pasteState(int editBoxIndex)
{
    QLineEdit* selectedBox = inputBoxes[editBoxIndex];
    QString text = selectedBox->text();
    if (text.endsWith(" ") || text.endsWith(","))
    {
        text.chop(1);
        selectedBox->setText(text);
        if (editBoxIndex < inputBoxes.size()-1)
        {
            inputBoxes[editBoxIndex+1]->setFocus();
            inputBoxes[editBoxIndex+1]->selectAll();
        }
    }
    else {
        text = text.simplified();
        QStringList split = text.split(",",QString::SkipEmptyParts);
        if (split.size() <= 1) split = text.split(" ",QString::SkipEmptyParts);
        if (split.size() > 1)
        {
            if (editBoxIndex < 3)
            {
                for (int i=0; i<std::min(6,split.size()); i++) { inputBoxes[i]->setText(split[i]); inputBoxes[i]->editingFinished(); }
            }
            else if (editBoxIndex < 6)
            {
                if (split.size() == 3) for (int i=0; i<split.size(); i++) { inputBoxes[3+i]->setText(split[i]); inputBoxes[3+i]->editingFinished(); }
                else for (int i=0; i<std::min(6,split.size()); i++) { inputBoxes[i]->setText(split[i]); inputBoxes[i]->editingFinished(); }
            }
            else if (editBoxIndex >= 12 && editBoxIndex <= 17)
            {
                if (split.size() == 6) for (int i=0; i<split.size(); i++) { inputBoxes[12+i]->setText(split[i]); inputBoxes[12+i]->editingFinished(); }
            }
            else if (editBoxIndex >= 18 && editBoxIndex <= 21)
            {
                if (split.size() == 4) for (int i=0; i<split.size(); i++) { inputBoxes[18+i]->setText(split[i]); inputBoxes[18+i]->editingFinished(); }
            }
        }
    }
}

void OpiExplorerMain::on_listObjects_currentRowChanged(int currentRow)
{
    if (validObjectSelected())
    {
        for (int i=0; i<inputBoxes.size(); i++) inputBoxes[i]->setReadOnly(false);
        for (int i=0; i<covBoxes.size(); i++) covBoxes[i]->setReadOnly(false);

        ui->leStateX->setText(QString::number(currentPopulation->getPosition()[currentRow].x,'g',precision));
        ui->leStateY->setText(QString::number(currentPopulation->getPosition()[currentRow].y,'g',precision));
        ui->leStateZ->setText(QString::number(currentPopulation->getPosition()[currentRow].z,'g',precision));
        ui->leStateXdot->setText(QString::number(currentPopulation->getVelocity()[currentRow].x,'g',precision));
        ui->leStateYdot->setText(QString::number(currentPopulation->getVelocity()[currentRow].y,'g',precision));
        ui->leStateZdot->setText(QString::number(currentPopulation->getVelocity()[currentRow].z,'g',precision));

        ui->leOrbitSMA->setText(QString::number(currentPopulation->getOrbit()[currentRow].semi_major_axis,'g',precision));
        ui->leOrbitEcc->setText(QString::number(currentPopulation->getOrbit()[currentRow].eccentricity,'g',precision));
        ui->leOrbitInc->setText(QString::number(currentPopulation->getOrbit()[currentRow].inclination,'g',precision));
        ui->leOrbitRAAN->setText(QString::number(currentPopulation->getOrbit()[currentRow].raan,'g',precision));
        ui->leOrbitAOP->setText(QString::number(currentPopulation->getOrbit()[currentRow].arg_of_perigee,'g',precision));
        ui->leOrbitMA->setText(QString::number(currentPopulation->getOrbit()[currentRow].mean_anomaly,'g',precision));

        ui->leEpochBOL->setText(QString::number(currentPopulation->getEpoch()[currentRow].beginning_of_life,'g',precision));
        ui->leEpochEOL->setText(QString::number(currentPopulation->getEpoch()[currentRow].end_of_life,'g',precision));
        ui->leEpochCurrent->setText(QString::number(currentPopulation->getEpoch()[currentRow].current_epoch,'g',precision));
        ui->leEpochOriginal->setText(QString::number(currentPopulation->getEpoch()[currentRow].original_epoch,'g',precision));

        ui->leAccX->setText(QString::number(currentPopulation->getAcceleration()[currentRow].x,'g',precision));
        ui->leAccY->setText(QString::number(currentPopulation->getAcceleration()[currentRow].y,'g',precision));
        ui->leAccZ->setText(QString::number(currentPopulation->getAcceleration()[currentRow].z,'g',precision));

        ui->leObjectName->setText(QString(currentPopulation->getObjectName(currentRow)));

        ui->lePropsID->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].id));
        ui->lePropsMass->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].mass,'g',precision));
        ui->lePropsDia->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].diameter,'g',precision));
        ui->lePropsA2M->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].area_to_mass,'g',precision));
        ui->lePropsDrag->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].drag_coefficient,'g',precision));
        ui->lePropsReflectivity->setText(QString::number(currentPopulation->getObjectProperties()[currentRow].reflectivity,'g',precision));

        double c[36];
        OPI::covarianceToArray(currentPopulation->getCovariance()[currentRow],c);
        for (int i=0; i<36; i++)
        {
            covBoxesLowerTriangular[i]->setText(QString::number(c[i]));
        }

        ui->leLastPropagator->setText(QString(currentPopulation->getLastPropagatorName()));

        ui->teByteArray->clear();
        int byteArraySize = currentPopulation->getByteArraySize();
        QByteArray bytes;
        for (int i=0; i<byteArraySize; i++)
        {
            char byte = currentPopulation->getBytes()[currentRow*byteArraySize+i];
            bytes.append(QChar(byte));
        }
        ui->teByteArray->setText((bytes.toHex(' ')).toUpper());

        OPI::Orbit o = currentPopulation->getOrbit()[currentRow];
        OPI::Vector3 p = currentPopulation->getPosition()[currentRow];
#ifdef ENABLE_OPENGL
        gl->setOrbit(o,p);
        gl->update();
#endif
    }
    else {
        resetInputBoxes();
    }

    bool hasObjects = (ui->listObjects->count() > 0);
    ui->actionDelete_Object->setEnabled(hasObjects);
    ui->btnDeleteObject->setEnabled(hasObjects);
    ui->actionOrbits_to_State_Vectors->setEnabled(hasObjects);
    ui->actionState_Vectors_to_Orbits->setEnabled(hasObjects);
    ui->btnOrbit2State->setEnabled(hasObjects);
    ui->btnState2Orbit->setEnabled(hasObjects);
    ui->actionSave_Population->setEnabled(hasObjects);
    ui->actionSave_as->setEnabled(hasObjects);
}

void OpiExplorerMain::on_listPProps_currentRowChanged(int currentRow)
{
    OPI::Propagator* propagator = host.getPropagator(ui->listPlugins->currentRow());
    if (propagator)
    {
        if (currentRow < propagator->getPropertyCount())
        {
            const char* name = propagator->getPropertyName(currentRow);
            OPI::PropertyType propType = propagator->getPropertyType(name);
            QString variable = "Selected value";
            if (propType == OPI::TYPE_DOUBLE)
            {
                variable = "double " + QString(name);
                ui->leCurrentPropertyValue->setText(QString::number(propagator->getPropertyDouble(name),'g',precision));
            }
            else if (propType == OPI::TYPE_INTEGER)
            {
                variable = "int " + QString(name);
                ui->leCurrentPropertyValue->setText(QString::number(propagator->getPropertyInt(name)));
            }
            else if (propType == OPI::TYPE_STRING)
            {
                variable = "string " + QString(name);
                ui->leCurrentPropertyValue->setText(QString(propagator->getPropertyString(name)));
            }
            else if (propType == OPI::TYPE_FLOAT)
            {
                variable = "float " + QString(name);
                ui->leCurrentPropertyValue->setText(QString::number(propagator->getPropertyFloat(name),'g',precision));
            }

            ui->lblSelectedValue->setText(variable);

        }
    }
}

void OpiExplorerMain::on_listPlugins_currentRowChanged(int currentRow)
{
    OPI::Propagator* propagator = host.getPropagator(ui->listPlugins->currentRow());
    if (propagator)
    {
        ui->listPProps->clear();
        for (int j=0; j<propagator->getPropertyCount(); j++)
        {
            const char* name = propagator->getPropertyName(j);
            OPI::PropertyType propType = propagator->getPropertyType(name);
            if (propType == OPI::TYPE_DOUBLE)
                ui->listPProps->addItem(QString("double " + QString(name)));
            else if (propType == OPI::TYPE_INTEGER)
                ui->listPProps->addItem(QString("int " + QString(name)));
            else if (propType == OPI::TYPE_STRING)
                ui->listPProps->addItem(QString("string " + QString(name)));
            else if (propType == OPI::TYPE_FLOAT)
                ui->listPProps->addItem(QString("float " + QString(name)));
        }
        QString info;
        info  = "Name:                           " + QString(propagator->getName()) + "\n";
        info += "Author:                         " + QString(propagator->getAuthor()) + "\n";
        info += "Requires OPI version:           " + QString::number(propagator->minimumOPIVersionRequired()) + "." + QString::number(propagator->minorOPIVersionRequired()) + "\n";
        info += "Requires CUDA:                  " + (propagator->requiresCUDA() ? QString("Yes") : QString("No")) + "\n";
        info += "Requires OpenCL:                " + (propagator->requiresOpenCL() ? QString("Yes") : QString("No")) + "\n";
        info += "Supports state vectors:         " + (propagator->cartesianCoordinates() ? QString("Yes") : QString("No")) + "\n";
        info += "Supports backwards propagation: " + (propagator->backwardPropagation() ? QString("Yes") : QString("No")) + "\n";
        info += "Supports OPI Logger:            " + (propagator->supportsOPILogger() ? QString("Yes") : QString("No")) + "\n";
        info += "Reference frame:                " + refFrameToString(propagator->referenceFrame()) + "\n";
        info += "\n" + QString(propagator->getDescription()) + "\n";
        ui->lblPropInfo->setText(info);
        ui->grpPropagate->setTitle("Propagate with " + QString(propagator->getName()));
        ui->btnPropagate->setEnabled(true);
        ui->actionLoad_with_Plugin->setEnabled(true);
        ui->btnAlign->setEnabled(true);
    }
}

QString OpiExplorerMain::refFrameToString(OPI::ReferenceFrame rf)
{
    switch (rf)
    {
    case OPI::REF_ECEF: return "ECEF";
    case OPI::REF_ECI: return "ECI";
    case OPI::REF_GCRF: return "GCRF";
    case OPI::REF_ITRF: return "ITRF";
    case OPI::REF_J2000: return "J2000";
    case OPI::REF_MOD: return "MOD";
    case OPI::REF_MULTIPLE: return "MULTIPLE - Please check propagator properties or documentation.";
    case OPI::REF_TEME: return "TEME";
    case OPI::REF_TOD: return "TOD";
    case OPI::REF_NONE: return "NONE - Please consult plugin documentation.";
    case OPI::REF_UNLISTED: return "UNLISTED - Please consult plugin documentation.";
    case OPI::REF_UNSPECIFIED: return "UNSPECIFIED - Please update plugin!";
    default: return "Unknown reference frame value, please update OPI Explorer!";

    }
}

void OpiExplorerMain::on_actionSave_as_triggered()
{
    QString saveFile = QFileDialog::getSaveFileName(this, "Save OPI Population", "", "OPI Population Files (*.opi)");
    if (saveFile != "")
    {
        currentPopulationFile = saveFile;
        currentPopulation->write(currentPopulationFile.toStdString().c_str());
        ui->tabsMain->setTabText(0, "Population");
        ui->actionReset_to_Saved_State->setEnabled(true);
    }
    updateWindowTitle();
}

void OpiExplorerMain::on_actionSave_Population_triggered()
{
    if (currentPopulationFile != "" && currentPopulationFile.endsWith(".opi"))
    {
        //check for currentPopulation->getSize() > 0 ?
        currentPopulation->write(currentPopulationFile.toStdString().c_str());
        ui->tabsMain->setTabText(0, "Population");
        ui->actionReset_to_Saved_State->setEnabled(true);
    }
    else on_actionSave_as_triggered();
    updateWindowTitle();
}

void OpiExplorerMain::on_actionSet_Plugin_Directory_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select OPI Plugin Directory");
    dialog.setDirectory(".");
    if (dialog.exec())
    {
        loadPlugins(dialog.selectedFiles()[0]);
    }
}

void OpiExplorerMain::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}

void OpiExplorerMain::on_actionAdd_Object_triggered()
{
    ui->tabsMain->setTabText(0, "Population*");
    int newSize = currentPopulation->getSize()+1;
    currentPopulation->resize(newSize, currentPopulation->getByteArraySize());
    int index = currentPopulation->getSize()-1;
    currentPopulation->getObjectProperties()[index].id = newSize;
    currentPopulation->getObjectProperties()[index].mass = 1.0;
    currentPopulation->getObjectProperties()[index].diameter = 1.0;
    currentPopulation->getObjectProperties()[index].reflectivity = 1.3;
    currentPopulation->getObjectProperties()[index].drag_coefficient = 2.2;
    currentPopulation->getObjectProperties()[index].area_to_mass = 1.0;
    std::vector<double> emptyCovariance(36,0.0);
    currentPopulation->getCovariance()[index] = OPI::arrayToCovariance(emptyCovariance.data());
    ui->listObjects->clear();
    for (int i=0; i<=index; i++)
    {
        ui->listObjects->addItem(QString::number(currentPopulation->getObjectProperties()[i].id));
    }
    ui->listObjects->setCurrentRow(index);
    ui->leStateX->setFocus();
}

bool OpiExplorerMain::validObjectSelected()
{
    return (currentPopulation->getSize() > 0
            && ui->listObjects->currentRow() >= 0
            && ui->listObjects->currentRow() < currentPopulation->getSize());
}

void OpiExplorerMain::on_leStateX_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getPosition()[ui->listObjects->currentRow()].x = ui->leStateX->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leStateY_editingFinished()
{
    if (validObjectSelected())
    {        
        currentPopulation->getPosition()[ui->listObjects->currentRow()].y = ui->leStateY->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leStateZ_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getPosition()[ui->listObjects->currentRow()].z = ui->leStateZ->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leStateXdot_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getVelocity()[ui->listObjects->currentRow()].x = ui->leStateXdot->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leStateYdot_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getVelocity()[ui->listObjects->currentRow()].y = ui->leStateYdot->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leStateZdot_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getVelocity()[ui->listObjects->currentRow()].z = ui->leStateZdot->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsID_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].id = ui->lePropsID->text().toInt();
        ui->listObjects->currentItem()->setText(ui->lePropsID->text());
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsMass_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].mass = ui->lePropsMass->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsDia_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].diameter = ui->lePropsDia->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsA2M_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].area_to_mass = ui->lePropsA2M->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsDrag_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].drag_coefficient = ui->lePropsDrag->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_lePropsReflectivity_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getObjectProperties()[ui->listObjects->currentRow()].reflectivity = ui->lePropsReflectivity->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leOrbitSMA_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].semi_major_axis = ui->leOrbitSMA->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leOrbitEcc_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].eccentricity = ui->leOrbitEcc->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leOrbitInc_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].inclination = ui->leOrbitInc->text().toDouble();
        updateObjects();
    }
}


void OpiExplorerMain::on_leOrbitRAAN_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].raan = ui->leOrbitRAAN->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leOrbitAOP_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].arg_of_perigee = ui->leOrbitAOP->text().toDouble();
        updateObjects();
    }
}

void OpiExplorerMain::on_leOrbitMA_editingFinished()
{
    if (validObjectSelected())
    {
        currentPopulation->getOrbit()[ui->listObjects->currentRow()].mean_anomaly = ui->leOrbitMA->text().toDouble();
        updateObjects();
    }
}


void OpiExplorerMain::on_btnPropagate_clicked()
{
    if (currentPopulation && currentPopulation->getSize() > 0)
    {
        QDateTime start = ui->dtStartTime->dateTime();
        QDateTime end = ui->dtEndTime->dateTime();
        double dt = ui->cmbDeltaT->currentText().toDouble();
        double jdStart = start.date().toJulianDay()-0.5 + (start.time().hour()*3600.0 + start.time().minute()*60.0 + start.time().second())/86400.0;
        double jdEnd = end.date().toJulianDay()-0.5 + (end.time().hour()*3600.0 + end.time().minute()*60.0 + end.time().second())/86400.0;
        int numSteps = ((jdEnd-jdStart)*86400.0 / dt);
        OPI::PropagationMode mode = (ui->cmbPropagatorMode->currentIndex() == 1 ? OPI::MODE_INDIVIDUAL_EPOCHS : OPI::MODE_SINGLE_EPOCH);
        OPI::Propagator* selectedPropagator = host.getPropagator(ui->listPlugins->currentRow());
        QProgressDialog progress(this);
        progress.setLabelText("Propagating "+QString::number(currentPopulation->getSize())+" objects with "+QString(selectedPropagator->getName())+"...");
        progress.setRange(0, numSteps);
        progress.setModal(true);
        selectedPropagator->enable();
        bool plot = (ui->chkPlot->checkState() == Qt::Checked);
        QList<QListWidgetItem*> items = ui->listObjects->selectedItems();
        QVector<QVector<QLineSeries*>> series;
        series.resize(plotViews.size());
        for (int s=0; s<plotViews.size(); s++) series[s].resize(items.size());
        if (plot)
        {
            for (int s=0; s<items.size(); s++)
            {
                for (int t=0; t<plotViews.size(); t++) series[t][s] = new QLineSeries();
                int index = ui->listObjects->row(items[s]);
                int id = currentPopulation->getObjectProperties()[index].id;
                QString name = QString(currentPopulation->getObjectName(index)) + " (" + QString::number(id) + ")";                
                series[0][s]->setName("Semi Major Axis: "+name);
                series[1][s]->setName("Eccentricity: "+name);
                series[2][s]->setName("Inclination: "+name);
                series[3][s]->setName("Right Ascension: "+name);
                series[4][s]->setName("Perigee Height: "+name);
            }
        }
        for (int i=0; i<numSteps; i++)
        {
            double timeStep = jdStart+((i*dt)/86400.0);
            progress.setValue(i);
            qApp->processEvents();
            if (progress.wasCanceled()) break;
            selectedPropagator->propagate(*currentPopulation, timeStep, dt, mode);

            if (plot)
            {
                for (int s=0; s<items.size(); s++)
                {
                    int index = ui->listObjects->row(items[s]);
                    if (OPI::isZero(currentPopulation->getOrbit()[0])) currentPopulation->convertStateVectorsToOrbits();
                    double epoch = (mode == OPI::MODE_INDIVIDUAL_EPOCHS) ? currentPopulation->getEpoch()[index].current_epoch : timeStep + dt/86400.0;
                    OPI::Orbit orbit = currentPopulation->getOrbit()[index];
                    double perigeeHeight = orbit.semi_major_axis * (1.0 - orbit.eccentricity) - 6378.1363;
                    series[0][s]->append(epoch, orbit.semi_major_axis);
                    series[1][s]->append(epoch, orbit.eccentricity);
                    series[2][s]->append(epoch, orbit.inclination);
                    series[3][s]->append(epoch, orbit.raan);
                    series[4][s]->append(epoch, perigeeHeight);
                }
            }

        }
        selectedPropagator->disable();

        if (plot)
        {
            for (int t=0; t<plotViews.size(); t++)
            {
                for (int s=0; s<items.size(); s++) plotViews[t]->chart()->addSeries(series[t][s]);
                plotViews[t]->chart()->createDefaultAxes();
            }
        }

        updateObjects();        
    }
}

void OpiExplorerMain::on_btnState2Orbit_clicked()
{
    currentPopulation->convertStateVectorsToOrbits();
    updateObjects();
}

void OpiExplorerMain::on_btnOrbit2State_clicked()
{
    currentPopulation->convertOrbitsToStateVectors();
    updateObjects();
}

void OpiExplorerMain::on_btnAddObject_clicked()
{
    on_actionAdd_Object_triggered();
}

void OpiExplorerMain::on_actionDelete_Object_triggered()
{
        QList<QListWidgetItem*> items = ui->listObjects->selectedItems();
        foreach(QListWidgetItem* item, items)
            currentPopulation->remove(ui->listObjects->row(item));
        updateObjects();
}

void OpiExplorerMain::on_btnDeleteObject_clicked()
{
    on_actionDelete_Object_triggered();
}

void OpiExplorerMain::on_actionOrbits_to_State_Vectors_triggered()
{
    on_btnOrbit2State_clicked();
}

void OpiExplorerMain::on_actionState_Vectors_to_Orbits_triggered()
{
    on_btnState2Orbit_clicked();
}

void OpiExplorerMain::on_leObjectName_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->setObjectName(ui->listObjects->currentRow(), ui->leObjectName->text().toStdString().c_str());
    }
}

void OpiExplorerMain::on_leEpochBOL_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getEpoch()[ui->listObjects->currentRow()].beginning_of_life = ui->leEpochBOL->text().toDouble();
    }
}

void OpiExplorerMain::on_leEpochEOL_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getEpoch()[ui->listObjects->currentRow()].end_of_life = ui->leEpochEOL->text().toDouble();
    }
}


void OpiExplorerMain::on_leEpochCurrent_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getEpoch()[ui->listObjects->currentRow()].current_epoch = ui->leEpochCurrent->text().toDouble();
    }
}

void OpiExplorerMain::on_leEpochOriginal_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getEpoch()[ui->listObjects->currentRow()].original_epoch = ui->leEpochOriginal->text().toDouble();
    }
}

void OpiExplorerMain::on_leAccX_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getAcceleration()[ui->listObjects->currentRow()].x = ui->leAccX->text().toDouble();
    }
}

void OpiExplorerMain::on_leAccY_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getAcceleration()[ui->listObjects->currentRow()].y = ui->leAccY->text().toDouble();
    }
}

void OpiExplorerMain::on_leAccZ_editingFinished()
{
    if (validObjectSelected())
    {
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getAcceleration()[ui->listObjects->currentRow()].z = ui->leAccZ->text().toDouble();
    }
}

void OpiExplorerMain::on_leCurrentPropertyValue_editingFinished()
{
    if (ui->listPlugins->currentRow() >= 0 && ui->listPlugins->currentRow() < host.getPropagatorCount())
    {
        OPI::Propagator* p = host.getPropagator(ui->listPlugins->currentRow());
        if (ui->listPProps->currentRow() >= 0 && ui->listPProps->currentRow() < p->getPropertyCount())
        {
            const char* name = p->getPropertyName(ui->listPProps->currentRow());
            OPI::PropertyType pt = p->getPropertyType(name);
            QString value = ui->leCurrentPropertyValue->text();
            switch (pt)
            {
            case OPI::TYPE_DOUBLE: p->setProperty(name, value.toDouble()); break;
            case OPI::TYPE_STRING: p->setProperty(name, value.toStdString().c_str()); break;
            case OPI::TYPE_FLOAT: p->setProperty(name, value.toFloat()); break;
            case OPI::TYPE_INTEGER: p->setProperty(name, value.toInt()); break;
            }
        }
    }
}

void OpiExplorerMain::on_leStateX_textChanged(const QString &arg1)
{
    pasteState(0);
}

void OpiExplorerMain::on_leStateY_textChanged(const QString &arg1)
{
    pasteState(1);
}

void OpiExplorerMain::on_leStateZ_textChanged(const QString &arg1)
{
    pasteState(2);
}

void OpiExplorerMain::on_leStateXdot_textChanged(const QString &arg1)
{
    pasteState(3);
}

void OpiExplorerMain::on_leStateYdot_textChanged(const QString &arg1)
{
    pasteState(4);
}

void OpiExplorerMain::on_leStateZdot_textChanged(const QString &arg1)
{
    pasteState(5);
}

void OpiExplorerMain::on_lePropsID_textChanged(const QString &arg1)
{
    pasteState(6);
}

void OpiExplorerMain::on_lePropsMass_textChanged(const QString &arg1)
{
    pasteState(7);
}

void OpiExplorerMain::on_lePropsDia_textChanged(const QString &arg1)
{
    pasteState(8);
}

void OpiExplorerMain::on_lePropsA2M_textChanged(const QString &arg1)
{
    pasteState(9);
}

void OpiExplorerMain::on_lePropsDrag_textChanged(const QString &arg1)
{
    pasteState(10);
}

void OpiExplorerMain::on_lePropsReflectivity_textChanged(const QString &arg1)
{
    pasteState(11);
}

void OpiExplorerMain::on_leOrbitSMA_textChanged(const QString &arg1)
{
    pasteState(12);
}

void OpiExplorerMain::on_leOrbitEcc_textChanged(const QString &arg1)
{
    pasteState(13);
}

void OpiExplorerMain::on_leOrbitInc_textChanged(const QString &arg1)
{
    pasteState(14);
}

void OpiExplorerMain::on_leOrbitRAAN_textChanged(const QString &arg1)
{
    pasteState(15);
}

void OpiExplorerMain::on_leOrbitAOP_textChanged(const QString &arg1)
{
    pasteState(16);
}

void OpiExplorerMain::on_leOrbitMA_textChanged(const QString &arg1)
{
    pasteState(17);
}

void OpiExplorerMain::on_leEpochBOL_textChanged(const QString &arg1)
{
    pasteState(18);
}

void OpiExplorerMain::on_leEpochEOL_textChanged(const QString &arg1)
{
    pasteState(19);
}


void OpiExplorerMain::on_leEpochCurrent_textChanged(const QString &arg1)
{
    pasteState(20);
}

void OpiExplorerMain::on_leEpochOriginal_textChanged(const QString &arg1)
{
    pasteState(21);
}

void OpiExplorerMain::on_leObjectName_textChanged(const QString &arg1)
{
    pasteState(22);
}

void OpiExplorerMain::on_leAccX_textChanged(const QString &arg1)
{
    pasteState(23);
}

void OpiExplorerMain::on_leAccY_textChanged(const QString &arg1)
{
    pasteState(24);
}

void OpiExplorerMain::on_leAccZ_textChanged(const QString &arg1)
{
    pasteState(25);
}


void OpiExplorerMain::on_btnCopyState_clicked()
{
    if (validObjectSelected())
    {
        QClipboard* clip = QApplication::clipboard();
        OPI::Vector3 pos = currentPopulation->getPosition()[ui->listObjects->currentRow()];
        OPI::Vector3 vel = currentPopulation->getVelocity()[ui->listObjects->currentRow()];
        QString text = QString::number(pos.x,'g',precision) + " " + QString::number(pos.y,'g',precision) + " " + QString::number(pos.z,'g',precision) + " "
                + QString::number(vel.x,'g',precision) + " " + QString::number(vel.y,'g',precision) + " " + QString::number(vel.z,'g',precision);
        clip->setText(text);
    }
}

void OpiExplorerMain::on_btnCopyOrbit_clicked()
{
    if (validObjectSelected())
    {
        QClipboard* clip = QApplication::clipboard();
        OPI::Orbit o = currentPopulation->getOrbit()[ui->listObjects->currentRow()];
        QString text = QString::number(o.semi_major_axis,'g',precision) + " " + QString::number(o.eccentricity,'g',precision) + " " + QString::number(o.inclination,'g',precision) + " "
                + QString::number(o.raan,'g',precision) + " " + QString::number(o.arg_of_perigee,'g',precision) + " " + QString::number(o.mean_anomaly,'g',precision);
        clip->setText(text);
    }
}

void OpiExplorerMain::on_btnCopyProps_clicked()
{
    if (validObjectSelected())
    {
        QClipboard* clip = QApplication::clipboard();
        OPI::ObjectProperties p = currentPopulation->getObjectProperties()[ui->listObjects->currentRow()];
        QString text = QString::number(p.id) + " " + QString::number(p.mass,'g',precision) + " " + QString::number(p.diameter,'g',precision) + " "
                + QString::number(p.area_to_mass,'g',precision) + " " + QString::number(p.drag_coefficient,'g',precision) + " " + QString::number(p.reflectivity,'g',precision);
        clip->setText(text);
    }
}


void OpiExplorerMain::on_btnCopyEpoch_clicked()
{
    if (validObjectSelected())
    {
        QClipboard* clip = QApplication::clipboard();
        OPI::Epoch e = currentPopulation->getEpoch()[ui->listObjects->currentRow()];
        QString text = QString::number(e.beginning_of_life,'g',precision) + " " + QString::number(e.end_of_life,'g',precision) + " " + QString::number(e.current_epoch,'g',precision);
        clip->setText(text);
    }
}

void OpiExplorerMain::on_actionNew_Population_triggered()
{
    delete currentPopulation;
    currentPopulation = new OPI::Population(host, 0);
    currentPopulationFile = "";
    ui->listObjects->clear();
    resetInputBoxes();
    ui->actionReset_to_Saved_State->setEnabled(false);
}

void OpiExplorerMain::on_actionReset_to_Saved_State_triggered()
{
    if (currentPopulationFile != "" && currentPopulationFile.endsWith(".opi"))
    {
        currentPopulation = new OPI::Population(host,0);
        currentPopulation->read(currentPopulationFile.toStdString().c_str());
        updateObjects();
        ui->tabsMain->setTabText(0, "Population");
        ui->actionReset_to_Saved_State->setEnabled(false);
    }
}


void OpiExplorerMain::on_btnToRadians_clicked()
{
    for (int i=14; i<18; i++)
    {
        inputBoxes[i]->setText(QString::number(inputBoxes[i]->text().toDouble()*M_PI/180.0));
        inputBoxes[i]->editingFinished();
    }
}


void OpiExplorerMain::on_btnToJulianDate_clicked()
{
    AuxFunctions aux;
    for (int i=18; i<22; i++)
    {
        double jd = aux.dateStringToJulian(inputBoxes[i]->text());
        if (jd > 0.0)
        {
            inputBoxes[i]->setText(QString::number(jd,'g',precision));
            inputBoxes[i]->editingFinished();
        }
    }
}

void OpiExplorerMain::on_actionOpen_Data_Parser_triggered()
{
    DataParser* dataParser = new DataParser(&host,this);
    if (dataParser->exec() == QDialog::Accepted)
    {
        delete currentPopulation;
        currentPopulation = new OPI::Population(*dataParser->getPopulation());
        updateObjects();
    }


}

void OpiExplorerMain::on_btnAlign_clicked()
{
    if (currentPopulation && currentPopulation->getSize() > 0)
    {
        OPI::Propagator* selectedPropagator = host.getPropagator(ui->listPlugins->currentRow());
        if (selectedPropagator)
        {
            double dt = ui->cmbDeltaT->currentText().toDouble();
            selectedPropagator->enable();
            selectedPropagator->align(*currentPopulation, dt);
            selectedPropagator->disable();
        }
        updateObjects();
    }
}

void OpiExplorerMain::on_actionAlign_to_Latest_Object_triggered()
{
    on_btnAlign_clicked();
}

void OpiExplorerMain::on_tabsMain_currentChanged(int index)
{
    if (index == 2) on_btnLogRefresh_clicked();
}

void OpiExplorerMain::on_btnLogRefresh_clicked()
{
    QFile log("opi-explorer.log");
    if(log.exists())
    {
        ui->teOpiLog->clear();
        if (log.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&log);
            while (!stream.atEnd())
            {
                ui->teOpiLog->append(stream.readLine());
            }
            log.close();
        }
    }
}

void OpiExplorerMain::on_teDescription_textChanged()
{
    if (currentPopulation)
    {
        currentPopulation->setDescription(ui->teDescription->toPlainText().toStdString().c_str());
    }
}


void OpiExplorerMain::on_leCov_0_textChanged(const QString &arg1)
{
    QStringList values = arg1.split(QRegularExpression("[ \\t\\r\\n]+"), QString::SkipEmptyParts);
    if (values.size() > 1)
    {
        for (int i=0; i<std::min(values.size(),covBoxesLowerTriangular.size()); i++)
        {
            covBoxesLowerTriangular[i]->setText(values[i]);
            covBoxesLowerTriangular[i]->editingFinished();
        }
        for (int i=values.size(); i<covBoxesLowerTriangular.size(); i++)
        {
            covBoxesLowerTriangular[i]->setText("0.0");
            covBoxesLowerTriangular[i]->editingFinished();
        }
    }
}

int OpiExplorerMain::diagonalIndex(int i)
{
    int x = i / 8;
    int y = i - x*8;
    return y*8 + x;
}


void OpiExplorerMain::on_leCov_0_editingFinished()
{
    QString text = ui->leCov_0->text();
    covBoxes[diagonalIndex(0)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k1_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_8_editingFinished()
{
    QString text = ui->leCov_8->text();
    covBoxes[diagonalIndex(8)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k2_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_9_editingFinished()
{
    QString text = ui->leCov_9->text();
    covBoxes[diagonalIndex(9)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k2_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_16_editingFinished()
{
    QString text = ui->leCov_16->text();
    covBoxes[diagonalIndex(16)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k3_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_17_editingFinished()
{
    QString text = ui->leCov_17->text();
    covBoxes[diagonalIndex(17)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k3_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_18_editingFinished()
{
    QString text = ui->leCov_18->text();
    covBoxes[diagonalIndex(18)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k3_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_24_editingFinished()
{
    QString text = ui->leCov_24->text();
    covBoxes[diagonalIndex(24)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k4_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_25_editingFinished()
{
    QString text = ui->leCov_25->text();
    covBoxes[diagonalIndex(25)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k4_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_26_editingFinished()
{
    QString text = ui->leCov_26->text();
    covBoxes[diagonalIndex(26)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k4_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_27_editingFinished()
{
    QString text = ui->leCov_27->text();
    covBoxes[diagonalIndex(27)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k4_k4 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_32_editingFinished()
{
    QString text = ui->leCov_32->text();
    covBoxes[diagonalIndex(32)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k5_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_33_editingFinished()
{
    QString text = ui->leCov_33->text();
    covBoxes[diagonalIndex(33)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k5_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_34_editingFinished()
{
    QString text = ui->leCov_34->text();
    covBoxes[diagonalIndex(34)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k5_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_35_editingFinished()
{
    QString text = ui->leCov_35->text();
    covBoxes[diagonalIndex(35)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k5_k4 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_36_editingFinished()
{
    QString text = ui->leCov_36->text();
    covBoxes[diagonalIndex(36)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k5_k5 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_40_editingFinished()
{
    QString text = ui->leCov_40->text();
    covBoxes[diagonalIndex(40)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_41_editingFinished()
{
    QString text = ui->leCov_41->text();
    covBoxes[diagonalIndex(41)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_42_editingFinished()
{
    QString text = ui->leCov_42->text();
    covBoxes[diagonalIndex(42)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_43_editingFinished()
{
    QString text = ui->leCov_43->text();
    covBoxes[diagonalIndex(43)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k4 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_44_editingFinished()
{
    QString text = ui->leCov_44->text();
    covBoxes[diagonalIndex(44)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k5 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_45_editingFinished()
{
    QString text = ui->leCov_45->text();
    covBoxes[diagonalIndex(45)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].k6_k6 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_48_editingFinished()
{
    QString text = ui->leCov_48->text();
    covBoxes[diagonalIndex(48)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_49_editingFinished()
{
    QString text = ui->leCov_49->text();
    covBoxes[diagonalIndex(49)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_50_editingFinished()
{
    QString text = ui->leCov_50->text();
    covBoxes[diagonalIndex(50)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_51_editingFinished()
{
    QString text = ui->leCov_51->text();
    covBoxes[diagonalIndex(51)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k4 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_52_editingFinished()
{
    QString text = ui->leCov_52->text();
    covBoxes[diagonalIndex(52)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k5 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_53_editingFinished()
{
    QString text = ui->leCov_53->text();
    covBoxes[diagonalIndex(53)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_k6 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_54_editingFinished()
{
    QString text = ui->leCov_54->text();
    covBoxes[diagonalIndex(54)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d1_d1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_56_editingFinished()
{
    QString text = ui->leCov_56->text();
    covBoxes[diagonalIndex(56)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_57_editingFinished()
{
    QString text = ui->leCov_57->text();
    covBoxes[diagonalIndex(57)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k2 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_58_editingFinished()
{
    QString text = ui->leCov_58->text();
    covBoxes[diagonalIndex(58)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k3 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_59_editingFinished()
{
    QString text = ui->leCov_59->text();
    covBoxes[diagonalIndex(59)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k4 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_60_editingFinished()
{
    QString text = ui->leCov_60->text();
    covBoxes[diagonalIndex(60)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k5 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_61_editingFinished()
{
    QString text = ui->leCov_61->text();
    covBoxes[diagonalIndex(61)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_k6 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_62_editingFinished()
{
    QString text = ui->leCov_62->text();
    covBoxes[diagonalIndex(62)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_d1 = text.toDouble();
    }
}

void OpiExplorerMain::on_leCov_63_editingFinished()
{
    QString text = ui->leCov_63->text();
    covBoxes[diagonalIndex(63)]->setText(text);
    if (validObjectSelected())
    {
        int popIndex = ui->listObjects->currentRow();
        ui->tabsMain->setTabText(0, "Population*");
        currentPopulation->getCovariance()[popIndex].d2_d2 = text.toDouble();
    }
}

void OpiExplorerMain::on_btnToKm_clicked()
{
    for (int i=0; i<6; i++)
    {
        inputBoxes[i]->setText(QString::number(inputBoxes[i]->text().toDouble()/1000.0,'g',precision));
        inputBoxes[i]->editingFinished();
    }
}

void OpiExplorerMain::on_actionExport_to_JSON_triggered()
{
    QString saveFile = QFileDialog::getSaveFileName(this, "Export OPI Population", "", "JSON (*.json)");
    if (saveFile != "")
    {
        currentPopulationFile = saveFile;
        currentPopulation->writeJSON(currentPopulationFile.toStdString().c_str());
        ui->actionReset_to_Saved_State->setEnabled(true);
    }
    updateWindowTitle();
}

void OpiExplorerMain::on_btnToKmCov_clicked()
{
    for (int i=0; i<covBoxes.size(); i++)
    {
        //Divide spacials by 1000000, spatial x force by 1000 and force x force not at all.
        double divisor = 1000000.0;
        if (i == 54 || i == 55 || i == 62 || i == 63) divisor = 1.0;
        else if (i > 45) divisor = 1000.0;
        covBoxes[i]->setText(QString::number(covBoxes[i]->text().toDouble()/divisor,'g',precision));
        covBoxes[i]->editingFinished();
    }
}

void OpiExplorerMain::on_btnCopyCov_clicked()
{
    if (validObjectSelected())
    {
        QClipboard* clip = QApplication::clipboard();
        OPI::Covariance c = currentPopulation->getCovariance()[ui->listObjects->currentRow()];
        QString text = QString::number(c.k1_k1,'g',precision) + " "
                + QString::number(c.k2_k1,'g',precision) + " "
                + QString::number(c.k2_k2,'g',precision) + " "
                + QString::number(c.k3_k1,'g',precision) + " "
                + QString::number(c.k3_k2,'g',precision) + " "
                + QString::number(c.k3_k3,'g',precision) + " "
                + QString::number(c.k4_k1,'g',precision) + " "
                + QString::number(c.k4_k2,'g',precision) + " "
                + QString::number(c.k4_k3,'g',precision) + " "
                + QString::number(c.k4_k4,'g',precision) + " "
                + QString::number(c.k5_k1,'g',precision) + " "
                + QString::number(c.k5_k2,'g',precision) + " "
                + QString::number(c.k5_k3,'g',precision) + " "
                + QString::number(c.k5_k4,'g',precision) + " "
                + QString::number(c.k5_k5,'g',precision) + " "
                + QString::number(c.k6_k1,'g',precision) + " "
                + QString::number(c.k6_k2,'g',precision) + " "
                + QString::number(c.k6_k3,'g',precision) + " "
                + QString::number(c.k6_k4,'g',precision) + " "
                + QString::number(c.k6_k5,'g',precision) + " "
                + QString::number(c.k6_k6,'g',precision) + " "
                + QString::number(c.d1_k1,'g',precision) + " "
                + QString::number(c.d1_k2,'g',precision) + " "
                + QString::number(c.d1_k3,'g',precision) + " "
                + QString::number(c.d1_k4,'g',precision) + " "
                + QString::number(c.d1_k5,'g',precision) + " "
                + QString::number(c.d1_k6,'g',precision) + " "
                + QString::number(c.d1_d1,'g',precision) + " "
                + QString::number(c.d2_k1,'g',precision) + " "
                + QString::number(c.d2_k2,'g',precision) + " "
                + QString::number(c.d2_k3,'g',precision) + " "
                + QString::number(c.d2_k4,'g',precision) + " "
                + QString::number(c.d2_k5,'g',precision) + " "
                + QString::number(c.d2_k6,'g',precision) + " "
                + QString::number(c.d2_d1,'g',precision) + " "
                + QString::number(c.d2_d2,'g',precision);
        clip->setText(text);
    }
}

void OpiExplorerMain::on_cmbRefFrame_currentIndexChanged(int index)
{
    if (validObjectSelected())
    {
        bool proceed = true;
        if (ui->cmbRefFrame->hasFocus())
        {
            QMessageBox warning;
            warning.setWindowTitle("Warning");
            warning.setIcon(QMessageBox::Warning);
            warning.setText("WARNING: Changing this value does not convert any coordinates! Modify only if you are certain that this is the correct reference frame for this population.\n\nAre you sure you wish to proceed?");
            warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            proceed = (warning.exec() == QMessageBox::Yes);
        }

        if (proceed)
        {
            switch (index)
            {
            case 0: currentPopulation->setReferenceFrame(OPI::REF_NONE); break;
            case 1: currentPopulation->setReferenceFrame(OPI::REF_UNSPECIFIED); break;
            case 2: currentPopulation->setReferenceFrame(OPI::REF_TEME); break;
            case 3: currentPopulation->setReferenceFrame(OPI::REF_GCRF); break;
            case 4: currentPopulation->setReferenceFrame(OPI::REF_ITRF); break;
            case 5: currentPopulation->setReferenceFrame(OPI::REF_ECI); break;
            case 6: currentPopulation->setReferenceFrame(OPI::REF_ECEF); break;
            case 7: currentPopulation->setReferenceFrame(OPI::REF_MOD); break;
            case 8: currentPopulation->setReferenceFrame(OPI::REF_TOD); break;
            case 9: currentPopulation->setReferenceFrame(OPI::REF_J2000); break;
            case 10: currentPopulation->setReferenceFrame(OPI::REF_MULTIPLE); break;
            case 11: currentPopulation->setReferenceFrame(OPI::REF_UNLISTED); break;
            default: currentPopulation->setReferenceFrame(OPI::REF_UNSPECIFIED); break;
            }
        }
        else {
            ui->cmbRefFrame->blockSignals(true);
            int rfIndex = ui->cmbRefFrame->findText(QString(OPI::referenceFrameToString(currentPopulation->getReferenceFrame())));
            ui->cmbRefFrame->setCurrentIndex(rfIndex);
            ui->cmbRefFrame->clearFocus();
            ui->cmbRefFrame->blockSignals(false);
        }
    }
}

void OpiExplorerMain::on_btnClearPlot_clicked()
{
    for (int i=0; i<plotViews.size(); i++) plotViews[i]->chart()->removeAllSeries();
}

void OpiExplorerMain::on_btnSavePlots_clicked()
{
    QString saveFile = QFileDialog::getSaveFileName(this, "Save Plots", "", "PNG (*.png)");
    if (saveFile != "")
    {
        QPixmap p = ui->scrollAreaWidgetContents->grab();
        p.save(saveFile);
    }
}
