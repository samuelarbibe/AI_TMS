#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SimulatorEngine = new Engine(ui->SimulatorFrame);

    DistanceUnits currentDistanceUnit = static_cast<DistanceUnits>(ui->DistanceUnitComboBox->currentIndex());
    VelocityUnits currentUnit = static_cast<VelocityUnits>(ui->VelocityUnitComboBox->currentIndex());

    // load presets
    ui->LaneWidthSlider->setMinimum(Settings::MinLaneWidth);
    ui->LaneWidthSlider->setMaximum(Settings::MaxLaneWidth);
    ui->LaneWidthSlider->setSliderPosition(Settings::LaneWidth);
    ui->ZoomSlider->setSliderPosition(Settings::Zoom * 99);
    ui->LaneWidthValueEdit->setText(QString::number(Settings::GetLaneWidthAs(currentDistanceUnit)));
    ui->CarMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::CAR, currentUnit)));
    ui->MotorcycleMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::MOTORCYCLE, currentUnit)));
    ui->TruckMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::TRUCK, currentUnit)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadOptionData()
{
    // set intersection number range for future use
    QStringList itemList = SimulatorEngine->map->GetIntersectionIdList();
    ui->FromIntersectionComboBox->addItems(itemList);
    ui->ToIntersectionComboBox->addItems(itemList);
    ui->IntersectionComboBox->addItems(itemList);

    ui->ToRoadComboBox->addItems(SimulatorEngine->map->GetRoadIdList());
}


// When mouse is clicked, use click coordinates in Map setup
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
    {

        QPoint clickPoint;
        Vector2f point;

        clickPoint = SimulatorEngine->mapFromGlobal(QCursor::pos());

        point.x = clickPoint.x();
        point.y = clickPoint.y();

        //cout << point.x << point.y << endl;

        if(point.x > 0 && point.y > 0)
        {
            // draw a point on simulator canvas to indicate last clicked position

            point = SimulatorEngine->DrawPoint(point);

            ui->IntersectionXEdit->setText(QString::number(int(point.x)));
            ui->IntersectionYEdit->setText(QString::number(int(point.y)));

            ui->statusbar->showMessage(tr("You can now Click 'Add Intersection' to add an intersection at the clicked position "));

            Lane * selectedLane = SimulatorEngine->map->SelectedLane;
            bool isSelected = (selectedLane != nullptr);
            ui->DeleteButton->setEnabled(isSelected);
            if(isSelected)
            {
                QString selectionText = "Selected: Lane {";
                selectionText.append(QString::number(selectedLane->GetLaneNumber()));
                selectionText.append("}, Road {");
                selectionText.append(QString::number(selectedLane->GetRoadNumber()));
                selectionText.append("}, Intersection {");
                selectionText.append(QString::number(selectedLane->GetIntersectionNumber()));
                selectionText.append("}");

                ui->statusbar->showMessage(selectionText);

            }

        }

    }
}

void MainWindow::on_AddIntersectionButton_clicked()
{
    if(ui->IntersectionXEdit->text().length() > 0 && ui->IntersectionYEdit->text().length() > 0 )
    {
        int x = ui->IntersectionXEdit->text().toInt();
        int y = ui->IntersectionYEdit->text().toInt();

        // check for usable data
        if(x > 0 && x < SimulatorEngine->map->GetSize().x && y > 0 && y < SimulatorEngine->map->GetSize().y)
        {
            Vector2f position(x, y);
            if(SimulatorEngine->map->AddIntersection(0, position) != nullptr)
            {
                // set data for future use
                reloadOptionData();
                ui->statusbar->clearMessage();
                ui->statusbar->showMessage(tr("Intersection Succefully added."), 5000);
                return; // success
            }
        }
    }
    ui->statusbar->showMessage(tr("Could not add Intersection. please check that the entered values are correct. "));
}


void MainWindow::on_AddConnectingRoadButton_clicked()
{
    int intersection1 = ui->FromIntersectionComboBox->currentText().toInt();
    int intersection2 = ui->ToIntersectionComboBox->currentText().toInt();

    // check for usable data
    if(intersection1 != intersection2)
    {
        if(SimulatorEngine->map->AddConnectingRoad(0, intersection1, intersection2) != nullptr)
        {
            ui->ToRoadComboBox->addItems(SimulatorEngine->map->GetRoadIdList());

            ui->statusbar->clearMessage();
            ui->statusbar->showMessage(tr("Connecting Road Succefully added."), 5000);
            return; // success
        }
    }

    ui->statusbar->showMessage(tr("Could not add Connecting Road. please check that the entered values are correct. "));
}

void MainWindow::on_AddRoadButton_clicked()
{
    int intersectionNumber = ui->IntersectionComboBox->currentText().toInt();
    int connectionSide     = ui->ConSideComboBox->currentIndex()+1;

    if(SimulatorEngine->map->AddRoad(0, intersectionNumber, connectionSide, Settings::DefaultLaneLength))
    {
        // refresh spinboxes data
        reloadOptionData();

        ui->statusbar->clearMessage();
        ui->statusbar->showMessage(tr("Road Succefully added."), 5000);
        return; // success
    }
    ui->statusbar->showMessage(tr("Could not add Road. please check that the entered values are correct. "));
}

void MainWindow::on_AddLanePushButton_clicked()
{
    int roadNumber         = ui->ToRoadComboBox->currentText().toInt();
    bool isInRoadDirection = ui->InDirectionCheckBox->isChecked();

    if(SimulatorEngine->map->AddLane(0, roadNumber, isInRoadDirection))
    {
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage(tr("Lane Succefully added."), 5000);
        return; // success
    }
    ui->statusbar->showMessage(tr("Could not add Lane. please check that the entered values are correct. "));

}

void MainWindow::on_SnapToGridCheckBox_stateChanged(int arg1)
{
    bool isChecked = ui->SnapToGridCheckBox->isChecked();

    ui->ShowGridCheckBox->setChecked(isChecked);
    ui->ShowGridCheckBox->setEnabled(isChecked);
    SimulatorEngine->SetSnapToGrid(isChecked);
}

void MainWindow::on_ShowGridCheckBox_stateChanged(int arg1)
{
    bool isChecked = ui->ShowGridCheckBox->isChecked();

    SimulatorEngine->ShowGrid(isChecked);
}

void MainWindow::on_LaneWidthSlider_sliderMoved(int position)
{
    DistanceUnits unit = static_cast<DistanceUnits>(ui->DistanceUnitComboBox->currentIndex());
    // setting the value will cut the value to the current range
    ui->LaneWidthSlider->setValue(position);
    Settings::LaneWidth = ui->LaneWidthSlider->value();
    ui->LaneWidthValueEdit->setText(QString::number(Settings::GetLaneWidthAs(unit)));
    // check if init was called, if was then reload map
    if(SimulatorEngine->map != nullptr)SimulatorEngine->map->ReloadMap();
}

void MainWindow::on_DistanceUnitComboBox_currentIndexChanged(int index)
{
    // get slider position -> lane with as px
    int position = ui->LaneWidthSlider->value();
    ui->LaneWidthValueEdit->setText(QString::number(Settings::GetLaneWidthAs(static_cast<DistanceUnits>(index))));
}

void MainWindow::on_LaneWidthValueEdit_editingFinished()
{
    // get the entered value in the current unit
    float enteredValue = ui->LaneWidthValueEdit->text().toFloat();

    // get the current unit
    DistanceUnits currentUnit = static_cast<DistanceUnits>(int(ui->DistanceUnitComboBox->currentIndex()));

    // convert the entered unit to PX
    enteredValue = Settings::ConvertSize(currentUnit, DistanceUnits::PX, enteredValue);
    // send it to this function that simulate a slider movement
    on_LaneWidthSlider_sliderMoved(enteredValue);
}

void MainWindow::on_ZoomSlider_valueChanged(int value)
{
    float zoomValue = 1.f - value/100.f;
    SimulatorEngine->UpdateView(Vector2f(0,0), zoomValue);
}

void MainWindow::on_CarMaxSpeed_editingFinished()
{
    // get the entered value for the max speed
    float enteredValue = ui->CarMaxSpeed->text().toFloat();

    // get the current unit
    VelocityUnits currentUnit = static_cast<VelocityUnits>(int(ui->VelocityUnitComboBox->currentIndex()));

    // convert the entered value to px
    enteredValue = Settings::ConvertVelocity(currentUnit, VelocityUnits::PXS, enteredValue);

    // save the changes
    Settings::MaxSpeeds[VehicleTypeOptions::CAR] = enteredValue;
}

void MainWindow::on_MotorcycleMaxSpeed_editingFinished()
{
    // get the entered value for the max speed
    float enteredValue = ui->CarMaxSpeed->text().toFloat();

    // get the current unit
    VelocityUnits currentUnit = static_cast<VelocityUnits>(int(ui->VelocityUnitComboBox->currentIndex()));

    // convert the entered value to px
    enteredValue = Settings::ConvertVelocity(currentUnit, VelocityUnits::PXS, enteredValue);

    // save the changes
    Settings::MaxSpeeds[VehicleTypeOptions::MOTORCYCLE] = enteredValue;
}

void MainWindow::on_TruckMaxSpeed_editingFinished()
{
    // get the entered value for the max speed
    float enteredValue = ui->CarMaxSpeed->text().toFloat();

    // get the current unit
    VelocityUnits currentUnit = static_cast<VelocityUnits>(int(ui->VelocityUnitComboBox->currentIndex()));

    // convert the entered value to px
    enteredValue = Settings::ConvertVelocity(currentUnit, VelocityUnits::PXS, enteredValue);

    // save the changes
    Settings::MaxSpeeds[VehicleTypeOptions::TRUCK] = enteredValue;
}

void MainWindow::on_VelocityUnitComboBox_currentIndexChanged(int index)
{
    VelocityUnits currentUnit = static_cast<VelocityUnits>(index);
    // re-display all the velocities
    ui->CarMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::CAR, currentUnit)));
    ui->MotorcycleMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::MOTORCYCLE, currentUnit)));
    ui->TruckMaxSpeed->setText(QString::number(Settings::GetMaxSpeedAs(VehicleTypeOptions::TRUCK, currentUnit)));
}

void MainWindow::on_MultiColorCheckBox_stateChanged(int arg1)
{
    Settings::MultiColor = ui->MultiColorCheckBox->isChecked();
}

void MainWindow::on_DeleteButton_clicked()
{
    Lane * selectedLane = SimulatorEngine->map->SelectedLane;
    if(selectedLane != nullptr)
    {
        int laneNumber = selectedLane->GetLaneNumber();
        // if deletion was successful
        if(SimulatorEngine->map->DeleteLane(selectedLane->GetLaneNumber()))
        {
            QString text = "Lane ";
            text.append(QString::number(laneNumber));
            text.append(" has been deleted. ");
            ui->statusbar->showMessage(text);

            // set spinbox ranges
            reloadOptionData();
        }
    }
}

void MainWindow::on_ResetButton_clicked()
{
    // TODO: add a confirm dialog
    SimulatorEngine->ResetMap();
    ui->statusbar->showMessage(tr("Map has been reset."));
}
