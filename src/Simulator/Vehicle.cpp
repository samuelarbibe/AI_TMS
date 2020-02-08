//
//  MovableObject.cpp
//  SimulatorSFML
//
//  Created by Samuel Arbibe on 22/11/2019.
//  Copyright © 2019 Samuel Arbibe. All rights reserved.
//

#include "Vehicle.hpp"

const Vector2f m_forwardVec{0.f, -1.f};

int Vehicle::toBeDeleted{0};
int Vehicle::VehicleCount = 0;
list<Vehicle*> Vehicle::ActiveVehicles;

VehicleType Vehicle::Car{CAR, "Car", "../../resources/Cars/car_image_", 5,  Vector2f(0.6f, 0.6f)};
VehicleType Vehicle::Truck{TRUCK, "Truck",  "../../resources/Cars/car_image_",5, Vector2f(0.12f, 0.12f)};
VehicleType Vehicle::Motorcycle{MOTORCYCLE,  "Motorcycle", "../resources/Cars/motorcycle_image_",5, Vector2f(0.12f, 0.12f)};

Vehicle::Vehicle(VehicleTypeOptions vehicleType, int vehicleNumber, queue<Lane*> * instructionSet, Map * map)
{
    // set initial values for the movable object
    m_vehicleType    = GetVehicleTypeByOption(vehicleType);
    m_vehicleNumber  = vehicleNumber;
    m_speed          = 0.f;
    m_acceleration   = m_maxAcceleration;
    m_state          = DRIVE;
    m_currentMap     = map;
    m_instructionSet = instructionSet;
    m_sourceLane     = m_instructionSet->front();
    m_instructionSet->pop();
    m_targetLane     = m_instructionSet->front();



    // get a pointer to the current intersection
    // current intersection is the intersection that the lane leads to
    m_currentIntersection = map->GetIntersection(m_sourceLane->GetIntersectionNumber());
    // the previous intersection, or the intersection of the source lane
    m_previousIntersection = nullptr;

    m_maxSpeed       = Settings::MaxSpeeds[m_vehicleType->Type];
    m_maxAcceleration= Settings::MaxAcceleration[m_vehicleType->Type];
    m_minAcceleration= Settings::MinAcceleration[m_vehicleType->Type];

    m_angularV       = 0;
    m_rotation       = m_sourceLane->GetDirection();
    m_position       = m_sourceLane->GetStartPosition();
    m_vehicleInFront = nullptr;

    // if vehicle texture hasn't been loaded yet, load it
    if(Vehicle::LoadVehicleTextures(m_vehicleType)) {

        // set up sprite
        int textureNumber;
        if(Settings::MultiColor)
        {
            textureNumber = (m_vehicleNumber % m_vehicleType->ImageCount);
        }
        else
        {
            textureNumber = 1;
        }

        m_texture = &(m_vehicleType->Textures->at(textureNumber));
        m_sprite.setTexture(*m_texture);
    }
    m_sprite.setScale(m_vehicleType->Scale);
    m_sprite.setOrigin(m_sprite.getTextureRect().width/2, m_sprite.getTextureRect().height/3);

    m_dataBox = new DataBox(m_position);
    m_dataBox->AddData("Speed", m_speed);
    m_dataBox->AddData("ID", m_vehicleNumber);
}

void Vehicle::DeleteAllVehicles()
{
    for (Vehicle * v : Vehicle::ActiveVehicles)
    {
        v->m_state = DELETE;
    }
}

void Vehicle::ClearVehicles()
{
    auto it = ActiveVehicles.begin();

    // while there are cars to delete;
    while(toBeDeleted != 0 && it != ActiveVehicles.end())
    {
        // if is to be deleted
        if((*it)->GetState() == DELETE)
        {
            Vehicle * temp = (*it);
            it = ActiveVehicles.erase(it);

            delete temp;

            toBeDeleted--;
            if(Settings::DrawActive)cout << "active vehicles : " << ActiveVehicles.size() << endl;
        }
        else
        {
            it++;
        }
    }
}

/// add a vehicle with an instruction set
Vehicle * Vehicle::AddVehicle(queue<Lane*> * instructionSet, Map * map, VehicleTypeOptions vehicleType, int vehicleNumber)
{
    auto * temp = new Vehicle(vehicleType, vehicleNumber, instructionSet, map);
    ActiveVehicles.push_back(temp);

    temp->m_vehicleInFront = (temp->m_sourceLane->GetLastCar()) ? GetVehicle(temp->m_sourceLane->GetLastCar()) : nullptr;
    /*
    if(temp->m_vehicleInFront != nullptr)
        cout << temp->m_vehicleNumber << " -> " << temp->m_vehicleInFront->m_vehicleNumber << endl;
    */

    //set this car as the last car that entered the lane
    temp->m_sourceLane->SetLastCar(vehicleNumber);
    temp->m_sourceLane->AddVehicleCount();
    VehicleCount++;

    if(Settings::DrawAdded)cout << "car " << vehicleNumber << " added to lane " << temp->m_sourceLane->GetLaneNumber() << endl;

    return temp;
}

/// load textures as required
bool Vehicle::LoadVehicleTextures(VehicleType * vehicleType)
{
    if(vehicleType->Textures == nullptr)
    {
        vehicleType->Textures = new vector<Texture>();

        string directory;
        Texture tempTexture;
        for (int i = 1; i <= vehicleType->ImageCount; ++i)
        {
            directory = vehicleType->ImageDir + to_string(i) + ".png";

            if (tempTexture.loadFromFile(directory))
            {
                tempTexture.setSmooth(true);
                vehicleType->Textures->push_back(tempTexture);
            }
            else
            {
                cerr << "loading texture no." << i << " for " << vehicleType->VehicleTypeName <<  " failed" << endl;
            }
        }

        cout << "----------------------------------------------" << endl;
        cout << vehicleType->Textures->size() << "/" << vehicleType->ImageCount << " Textures successfully added" << endl;
        cout << "----------------------------------------------" << endl;
        vehicleType->ImageCount = vehicleType->Textures->size();
    }
    return vehicleType->ImageCount > 0 ? true : false;
}

/// convert vehicleTypeOption to VehicleType struct
VehicleType * Vehicle::GetVehicleTypeByOption(VehicleTypeOptions vehicleTypeOptions)
{
    switch (vehicleTypeOptions)
    {
        case TRUCK:
            return &(Vehicle::Truck);
        case MOTORCYCLE:
            return &(Vehicle::Motorcycle);
        case CAR:
        default:
            return &(Vehicle::Car);
    }
}

/// get vehicle by vehicleNumber
Vehicle * Vehicle::GetVehicle(int vehicleNumber)
{
    for(Vehicle *v : ActiveVehicles)
    {
        if(v->m_vehicleNumber == vehicleNumber)
        {
            return v;
        }
    }

    return nullptr;
}

/// transfer a vehicle from a lane to another lane
void Vehicle::TransferVehicle(Vehicle * vehicle, Lane * toLane, Lane * fromLane)
{
    if(vehicle == nullptr)
    {
        cout << "vehicle not found" << endl;
        return;
    }

    if(fromLane != nullptr)
    {
        if(vehicle->m_sourceLane->GetLaneNumber() != fromLane->GetLaneNumber())
        {
            cout << "vehicle is not in the given lane" << endl;
            return;
        }
        fromLane->RemoveVehicleCount();
    }

    vehicle->m_sourceLane    = toLane;
    vehicle->m_rotation      = vehicle->m_sourceLane->GetDirection();
    vehicle->m_angularV      = 0;
    vehicle->m_currentIntersection = vehicle->m_currentMap->GetIntersection(vehicle->m_sourceLane->GetIntersectionNumber());
    vehicle->m_vehicleInFront = (vehicle->m_sourceLane->GetLastCar()) ? GetVehicle(vehicle->m_sourceLane->GetLastCar()) : nullptr;
    vehicle->m_sourceLane->SetLastCar(vehicle->m_vehicleNumber);
    vehicle->m_sourceLane->AddVehicleCount();

    vehicle->m_instructionSet->pop();
    // if there are instructions left, transfer them to vehicle
    if(!vehicle->m_instructionSet->empty())
    {
        vehicle->m_targetLane = vehicle->m_instructionSet->front();
    }
    else
    {
        vehicle->m_targetLane = nullptr;
    }
}

/// do drive cycle
State Vehicle::drive()
{
    // upon creation, all cars are stacked on each other.
    // while cars dont have a min distance, they wont start driving


    // check for distance with car in front
    if(m_vehicleInFront != nullptr && m_vehicleInFront->m_state != DELETE)
    {
        float distanceFromNextCar = calculateDistance(m_position, m_vehicleInFront->m_position);
        //cout << distanceFromNextCar << endl;
        float brakingDistance = -(m_speed * m_speed)/ (2 * m_minAcceleration);

        if(distanceFromNextCar < brakingDistance + Settings::MinDistanceFromNextCar || distanceFromNextCar < Settings::MinDistanceFromNextCar)
        {
            m_state = STOP;
            m_acceleration = m_minAcceleration;
            return STOP;
        }
    }

    // check if car is in between lanes (inside an intersection)
    if(m_currentIntersection->getGlobalBounds().contains(m_position))
    {
        // TODO: fix turning left
        if(m_angularV == 0)
        {
            float distanceSourceTarget = calculateDistance(m_sourceLane->GetEndPosition(), m_targetLane->GetStartPosition());

            float angle = (m_sourceLane->GetDirection() - m_targetLane->GetDirection());

            if(angle > 180) angle -= 360;

            float turningRadius = (distanceSourceTarget/2.f) / (sin(angle * M_PI/360.f));

            float turningParameter = 2.f * M_PI * turningRadius;

            float turningDistance = (angle/360.f) * turningParameter;

            m_angularV = -angle/turningDistance;
        }

        if(m_sourceLane != nullptr)
        {
            m_previousIntersection = m_currentMap->GetIntersection(m_sourceLane->GetIntersectionNumber());
            m_previousIntersection->AddVehicleCount();
            m_sourceLane->RemoveVehicleCount();
            m_sourceLane = nullptr;
        }

        m_state = TURN;

        //set rotation
        m_acceleration = (Settings::AccWhileTurning)?m_maxAcceleration/2.f : 0;
        return TURN;
    }

    // check distance from stop (if lane is blocked)
    if(m_sourceLane != nullptr && m_sourceLane != m_targetLane && m_sourceLane->GetIsBlocked() && !m_sprite.getGlobalBounds().contains(m_sourceLane->GetEndPosition()))
    {
        float distanceFromStop = calculateDistance(this->m_position, m_sourceLane->GetEndPosition());
        float brakingDistance = -(m_speed * m_speed)/ (2 * m_minAcceleration);

        if(distanceFromStop < brakingDistance + Settings::MinDistanceFromStop)
        {
            m_state = STOP;
            m_acceleration = m_minAcceleration;
            return STOP;
        }
    }

    // check if car has left intersection and is now in targetLane
    if(m_targetLane != nullptr &&  m_targetLane->getGlobalBounds().contains(m_position))
    {
        // remove count from previous lane, and set in to nullptr
        m_previousIntersection->RemoveVehicleCount();
        m_previousIntersection = nullptr;

        // we need to transfer vehicle to target lane
        TransferVehicle(this, m_targetLane, m_sourceLane);

        m_acceleration = m_maxAcceleration;
        m_state = DRIVE;
        return DRIVE;
    }

    // check if car is no longer in intersection
    if(m_targetLane == nullptr && !m_sourceLane->getGlobalBounds().contains(m_position))
    {
        m_sourceLane->RemoveVehicleCount();
        toBeDeleted++;

        m_state = DELETE;
        return DELETE;
    }

    // default = just drive
    m_acceleration = m_maxAcceleration;
    m_state = DRIVE;
    return DRIVE;
}

/// calculate a distance between two vectors
float Vehicle::calculateDistance(Vector2f a, Vector2f b)
{
    float xDist = abs(a.x - b.x);
    float yDist = abs(a.y - b.y);

    return sqrt(xDist*xDist + yDist*yDist);
}

/// update a vehicle's location
void Vehicle::Update(float elapsedTime){

    m_dataBox->Update(m_position);
    m_dataBox->SetData("Speed", Settings::ConvertVelocity(PXS, KMH, m_speed));
    drive();
    applyChanges(elapsedTime);
}

/// apply the calculated next position
void Vehicle::applyChanges(float elapsedTime)
{
    // apply acceleration
    m_speed += m_acceleration * elapsedTime * Settings::Speed;

    // apply max Speed limit
    if(m_speed > m_maxSpeed) m_acceleration = m_minAcceleration * Settings::Speed;

    // apply min Speed limit
    if(m_speed < 0) m_speed = 0;

    // set rotation relative to currentSpeed, to create a constant turning radius
    Transform t;
    m_rotation += m_angularV * elapsedTime * m_speed * Settings::Speed;

    t.rotate(m_rotation);

    // rotate the movement vector in relation to the forward vector (0,1)
    m_movementVec = t.transformPoint(m_forwardVec);

    // apply movement vector on position, relative to elapsed time to ensure
    // a constant Speed at any FPS

    m_position += m_movementVec * m_speed * elapsedTime * Settings::Speed;

    // apply rotation and position changes to the actual car sprite
    m_sprite.setPosition(m_position);
    m_sprite.setRotation(m_rotation);

}

/// render the vehicle
void Vehicle::Draw(RenderWindow *window)
{
    (*window).draw(this->m_sprite);
    if(Settings::DrawVehicleDataBoxes) m_dataBox->Draw(window);
}



