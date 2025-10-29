#ifndef OBJECTS_H
#define OBJECTS_H
#include "common.hpp"

class Module;
class SiteRow;
class Pin;
class Net;
class PlaceData;

class Module
{
public:
    Module()
    {
        Init();
    }
    int idx;
    string name;
    float width;
    float height;
    float area;
    float orientation;
    bool isMacro;
    bool isFixed;
    bool isFiller;
    vector<shared_ptr<Pin>> modulePins;
    vector<shared_ptr<Net>> nets;
    void Init()
    {
        idx = -1;
        center.SetZero();
        width = 0;
        height = 0;
        area = 0;
        orientation = 0;
        isMacro = false;
        isFiller = false;
        isFixed = false;
    }
    POS_2D center;

    float getArea() const { return area; }
};
class SiteRow
{
public:
    SiteRow()
    {
        bottom = 0;
        height = 0;
        step = 0;
        start.SetZero();
        end.SetZero();
        orientation = ORIENT::N; // 默认为北方向
    }

    double bottom;
    double height;
    double step;
    POS_2D start;
    POS_2D end;
    ORIENT orientation; // N (0) 或 S (1)

    vector<Interval> intervals; // 间隔列表

    double getSizeRowArea() const { return height * (end.x - start.x); }
};

class Pin
{
public:
    Pin()
    {
        init();
    }

    void init()
    {
        idx = -1;
        offset.SetZero();
    }
    int idx;
    weak_ptr<Module> module;
    weak_ptr<Net> net;
    POS_2D offset;
};

class Net
{
public:
    Net(string &_name) : name(_name) { init(); }
    string name;
    vector<shared_ptr<Pin>> netPins;

    void init() { netPins.clear(); }
};

class Bin
{
public:
    Bin()
    {
        init();
    }
    void init()
    {
        ll.SetZero();
        ur.SetZero();
        center.SetZero();
        width = 0;
        height = 0;
        terminalDensity = 0;
        darkDensity = 0;
    }
    POS_2D ll;
    POS_2D ur;
    POS_2D center;
    double width;
    double height;
    double terminalDensity;
    double darkDensity;
};
#endif