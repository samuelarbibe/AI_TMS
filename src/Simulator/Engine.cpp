//
//  Engine.cpp
//  SimulatorSFML
//
//  Created by Samuel Arbibe on 22/11/2019.
//  Copyright © 2019 Samuel Arbibe. All rights reserved.
//

#include "Engine.hpp"



int Vehicle::VehicleCount = 0;
list<Vehicle*> Vehicle::ActiveVehicles;

Engine::Engine(int windowWidth, int windowHeight, const char * windowName)
{
    cout << "Creating window..." << endl;
    m_window.create(VideoMode(windowWidth, windowHeight), windowName);

    cout << "Setting max fps to " << MAX_FPS << "..." <<  endl;
    m_window.setFramerateLimit(MAX_FPS);

    m_window.setView(View(FloatRect(0, 0, windowWidth, windowHeight)));

    cout << "Activating window..." << endl;

    m_window.setActive();

    if(!m_window.isOpen())
    {
        cerr << "Window creation has failed..." << endl;
        cerr << "Exiting Application..." << endl;
        exit(1);
    }
    cout << "----------------------------------------------" << endl;
    cout << "Window successfully created" << endl;
    cout << "----------------------------------------------" << endl;

    cout << "Setting up max speeds..." << endl;
    Vehicle::SetMaxSpeed(VehicleTypeOptions::CAR, 100.f, 1.5f);
    Vehicle::SetMaxSpeed(VehicleTypeOptions::TRUCK, 80.f, 1.f);

    cout << "Setting up weather conditions..." << endl;
    //Vehicle::SetWeatherCondition(WeatherCondition::DRY);

    map = new Map(0, Vector2f(windowWidth/2, windowHeight/2), windowWidth, windowHeight);

    map->AddIntersection(0,Vector2f(500, 500));

    map->AddIntersection(0,Vector2f(500, 500));

    map->AddConnectingRoad(1, 1, 2, ConnectionSides::RIGHT, ConnectionSides::LEFT);

    map->AddRoad(2, 1, ConnectionSides::UP, 200);
    map->AddRoad(3, 1, ConnectionSides::DOWN, 200);

    map->AddRoad(4, 2, ConnectionSides::UP, 200);
    map->AddRoad(5, 2, ConnectionSides::DOWN, 200);


    map->AddLane(0, 1, false);
    map->AddLane(0, 1, false);
    map->AddLane(0, 1, true);
    map->AddLane(0, 1, true);

    map->AddLane(0, 2, false);
    map->AddLane(0, 2, true);

    map->AddLane(0, 3, false);
    map->AddLane(0, 3, true);

    map->AddLane(0, 4, false);
    map->AddLane(0, 4, true);

    map->AddLane(0, 5, false);
    map->AddLane(0, 5, true);

};

void Engine::Start(){
    
    Clock clock;
    int frameCount = 0;
    
    while (m_window.isOpen()) {
        
        // count elapsed frames for FPS calculation
        frameCount++;
        
        // the time it takes to do 1 frame iteration
        Time dt = clock.restart();
        
        float dtInSeconds = dt.asSeconds();
        
        // print out FPS
        if(DRAW_FPS && frameCount % 20 == 0) std::cout << 1/dtInSeconds << std::endl;
        
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)){
                m_window.close();
            }
        }
        
        input();
        update(dtInSeconds / (float)SCALE * (float)SPEED);
        draw();
        
    }
}

void Engine::input(){
    
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        Vehicle::AddVehicle(map->GetLane(1), map->GetLane(1), map->GetIntersection(1));
    }

    if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        //Vehicle::AddVehicle(inter->GetLane(2), inter->GetLane(7), inter);
    }
    
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        //inter->GetLane(2)->SetIsBlocked(true);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        //inter->GetLane(2)->SetIsBlocked(false);
    }
}

void Engine::update(float dtInSeconds){

    map->Update(dtInSeconds);

    for (Vehicle *v : Vehicle::ActiveVehicles)
    {
        v->Update(dtInSeconds);
    }
    
    //clear all cars to be deleted
    Vehicle::ClearVehicles();
}

void Engine::draw()
{
    // Clean out the last frame
    m_window.clear(BackgroundColor);
 
    // Draw the objects
    this->map->Draw(&m_window);

    for(Vehicle * v : Vehicle::ActiveVehicles)
    {
        v->Draw(&m_window);
    }
     
    // Show everything that has been drawn
    m_window.display();
}