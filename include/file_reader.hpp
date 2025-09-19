#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "placedata.hpp"
#include <filesystem>
#include <memory>
#include "myplace.hpp"
#include <charconv>
#include <timer.hpp>


namespace fs = std::filesystem;

class FileReader{
    public:
        /**
         * @brief Construct a new File Reader object
         * 
         * @param filepath 需要读取的文件夹
         * @return none
         * 
         */
        FileReader(const std::string& filepath){
            std::filesystem::path path(filepath);
            std::string dir_name = path.filename().string();
            std::filesystem::path aux_path = path / (dir_name + ".aux");
            std::filesystem::path nets_path = path / (dir_name + ".nets");
            std::filesystem::path nodes_path = path / (dir_name + ".nodes");
            std::filesystem::path pl_path = path / (dir_name + ".pl");
            std::filesystem::path scl_path = path / (dir_name + ".scl");
            std::filesystem::path wts_path = path / (dir_name + ".wts");
            pdata = make_shared<PlaceData>();
            // read_aux(aux_path);
            read_pl(pl_path);
            read_nets(nets_path);
            // read_nodes(nodes_path);
            
            read_scl(scl_path);
            // read_wts(wts_path);
            myplace = make_shared<MyPlacer>(pdata.get());
        }

        void test_print(){
            // std::cout << "here are test print"<<std::endl;
            // for(unsigned i = 0;i< pdata->Nets.size();i++){
            //     std::cout<<pdata->Nets[i]->name<<std::endl;
            // }
            // std::ofstream out("/home/ezio/ClassProject3/testout");
            // for(auto module:pdata->moduleMap)
            // {
            //     out<<module.first<<" "<<module.second.get()->center<<std::endl;
            // }
        }

    private:
    void read_aux(const std::filesystem::path& aux_path);
    void read_nets(const std::filesystem::path& nets_path);
    void read_nodes(const std::filesystem::path& nodes_path);
    void read_pl(const std::filesystem::path& pl_path);
    void read_scl(const std::filesystem::path& scl_path);
    void read_wts(const std::filesystem::path& wts_path);


    shared_ptr<PlaceData> pdata;
    shared_ptr<MyPlacer> myplace;

    int READ_LINENUMS=1024;



    //helper functions
    /**
     * @brief split a line by space
     * 
     * @param line line in file
     * @return std::vector<std::string> vector of splited strings
     */
    std::vector<std::string> splist_by_space(std::string& line);

};


struct Orientation{
    static constexpr float N=0;
    static constexpr float E=90;
    static constexpr float S=180;
    static constexpr float W=270;
    //to be confirm
    static constexpr float FS=0;
};