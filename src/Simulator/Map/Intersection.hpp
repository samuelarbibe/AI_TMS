#pragma once
//
//  Intersection.hpp
//  SimulatorSFML
//
//  Created by Samuel Arbibe on 23/11/2019.
//  Copyright © 2019 Samuel Arbibe. All rights reserved.
//

#ifndef Intersection_hpp
#define Intersection_hpp

#include <iostream>
#include <list>

#include <SFML/Graphics.hpp>

#include "Road.hpp"
#include "../Settings.hpp"

using namespace sf;
using namespace std;

//typedef enum {DRY = 8, MOIST = 7 ,RAIN = 5, HEAVY_RAIN = 4, SNOW = 3, ICE = 1} WeatherCondition; // the friction coefficient, defining the cars breaking distance
typedef enum {UP = 1, RIGHT, DOWN, LEFT} ConnectionSides;


class Intersection: public RectangleShape
{
    
public:
    
    Intersection(Vector2f position, int intersectionNumber);
    ~Intersection();

    void   ReloadIntersection();
    void   ReAssignRoadPositions();
    void   Update(float elapsedTime);
    void   Draw(RenderWindow *window);
    bool   DeleteLane(int laneNumber, Intersection * otherIntersection = nullptr);

	// Add entities
    Road * AddRoad(int roadNumber, int connectionSide, float length);
    Road * AddConnectingRoad(int roadNumber, int connectionSide1, int connectionSide2, Intersection * connectedIntersection);
    Lane * AddLane(int laneNumber, int roadNumber, bool isInRoadDirection);

    // get
    Road * GetRoad(int roadNumber);
    Road * GetRoadByConnectionSide(int connectionSide);
    vector<Road*> * GetRoads(){return &(roads_);}
    Lane * GetLane(int laneNumber);

    int GetIntersectionNumber(){return intersection_number_;}
    int GetRoadCount(){return roads_.size();}
    int GetLaneCount();
    Vector2f GetPositionByConnectionSide(int connectionSide);

    // set
    void  AddVehicleCount(){current_vehicle_count_++; total_vehicle_count_++;};
    void  RemoveVehicleCount(){current_vehicle_count_--;};

    Lane * CheckSelection(Vector2f position);

    static int IntersectionCount;
    
private:

    int current_vehicle_count_;
    int total_vehicle_count_;
    int intersection_number_;
    int number_of_roads_;
    int width_;
    int height_;

    Vector2f position_;
    
    vector<Road*> roads_;
};

#endif /* Intersection_hpp */
