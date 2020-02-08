//
// Created by Samuel Arbibe on 07/02/2020.
//

#ifndef SIMULATORSFML_DATABOX_HPP
#define SIMULATORSFML_DATABOX_HPP

#include <iostream>
#include <fstream>
#include <list>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <nlohmann/json.hpp>

using nlohmann::json;
using namespace sf;
using namespace std;


class DataBox : public RectangleShape
{
public:
    DataBox(Vector2f position);
    ~DataBox(){};

    json GetData();
    bool AddData(string valueName, float value);
    bool SetData(string valueName, float value);
    bool RemoveData(string valueName);

    void Update(Vector2f position);
    void Draw(RenderWindow * window);

private:

    // dataBox offset relative to owner
    Vector2f m_offset;
    // the list data;
         // string is the name of the data
         // float is its value
    json m_data;

    Font m_font;

    // the maximum amount of data items allowed
    int m_maxDataItems;

    int m_itemCount;
};


#endif //SIMULATORSFML_DATABOX_HPP