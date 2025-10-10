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

    void print_info(){ 
        int num_modules = Nodes.size();
        int macro_count = 0;
        int fixed_count = 0;
        for(auto module:Nodes){
            if(module->isMacro) MacroCount++;
            if(module->isFixed) fixed_count++;
        }
        std::ofstream fout("/home/ezio/ClassProject3/info.txt");
        fout << "Object #:"<< num_modules <<"(="<<static_cast<int>(num_modules/1000)<<"k)"<< "(fixed:"<< fixed_count <<")(macro:" <<macro_count <<") " << std::endl;
    }

    
};
#endif