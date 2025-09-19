#ifndef PLACEDATA_H
#define PLACEDATA_H
#include "objects.hpp"

class PlaceData
{
public:
    int moduleCount; 
    int MacroCount;
    int netCount;
    int pinCount;
    int numRows;


    vector<shared_ptr<Module>> Nodes;
    vector<shared_ptr<Module>> Terminals;
    vector<shared_ptr<Pin>> Pins;
    vector<shared_ptr<Net>> Nets;
    vector<SiteRow> SiteRows;

    size_t max_net_degree{0};

    unordered_map<string, shared_ptr<Module>> moduleMap; 
    shared_ptr<Module> getModuleByName(const string &name){
        if(moduleMap.find(name) != moduleMap.end()){
            return moduleMap[name];
        }
        return nullptr;
    }

    
};
#endif