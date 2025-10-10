#include "file_reader.hpp"

std::vector<std::string> FileReader::splist_by_space(std::string& line)
{
    std::vector<string> result;
    std::stringstream iss(line);
    std::string token;
    while(iss >> token)
    {
        result.push_back(token);
    }
    return result;
}



void FileReader::read_pl(const fs::path& filePath)
{ 
    Timer timer;
    if(filePath.extension() != ".pl"){
       throw std::runtime_error("File extension is not .pl");
    }
    
    std::ifstream file(filePath);
    if(!file){
        throw std::runtime_error("File not found/opened"+filePath.filename().string());
    }

    //deal with file
    
    
    std::string line;
    while(std::getline(file,line))
    {
        //自动用移动构造（而不是拷贝构造)
        auto splist_result=splist_by_space(line);
        //unfit data handle
        if(splist_result.size()<5)
            continue;
        if(splist_result[3]!=":")
            continue;
        
        //deal data
        auto& module_name=splist_result[0];
        auto& x_coordinate=splist_result[1];
        auto& y_coordinate=splist_result[2];
        auto& direction=splist_result[4];
        bool is_fixed=(splist_result.size()==6&&splist_result[5]=="/FIXED");

        auto module=pdata->getModuleByName(module_name);
        if(module==nullptr){
            module=make_shared<Module>();
            module->Init();
            pdata->moduleMap[module_name]=module;
            pdata->Nodes.push_back(module);
        }

        module.get()->center.x=stof(x_coordinate);
        module.get()->center.y=stof(y_coordinate);
        module.get()->orientation=direction=="N" ? Orientation::N : direction=="E" ? Orientation::E : direction=="S" ?  Orientation::S : direction=="W" ? Orientation::W : 0;
        module.get()->isFixed=is_fixed;

    }
    
    cout<<"read_pl done"<<endl;
}



void FileReader::read_nets(const std::filesystem::path &nets_path) {
  std::ifstream infile(nets_path);
  if (!infile) {
    throw std::runtime_error("Could not open file: " +
                             nets_path.filename().string());
  }
  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::string token, skip;
    std::string_view sv(line);
    
    if (sv.find("NumPins") != std::string_view::npos) {
      iss >> skip >> skip >> token;
      std::from_chars(token.data(), token.data() + token.size(),
                      pdata->pinCount);
      pdata->Pins.reserve(2 * pdata->pinCount);
    } else if (sv.find("NumNets") != std::string_view::npos) {
      iss >> skip >> skip >> token;
      std::from_chars(token.data(), token.data() + token.size(),
                      pdata->netCount);
      pdata->Nets.reserve(2 * pdata->netCount);
    } else if (sv.find("NetDegree") != std::string_view::npos) {
      iss >> skip >> skip >> token;
      unsigned int num_pins = std::stoi(token);
      string net_name;
      iss >> net_name;
      auto net = std::make_shared<Net>(net_name);
      pdata->max_net_degree =
          pdata->max_net_degree > num_pins ? pdata->max_net_degree : num_pins;
      for (unsigned int i = 0; i < num_pins; i++) {
        std::getline(infile, line);
        std::istringstream pin_iss(line);
        string pin_name, module_name;
        pin_iss >> module_name;
        pin_iss >> pin_name;
        pin_iss >> skip; // skip :
        float x_offset, y_offset;
        pin_iss >> token;
        std::from_chars(token.data(), token.data() + token.size(), x_offset);
        pin_iss >> token;
        std::from_chars(token.data(), token.data() + token.size(), y_offset);
        {auto mod = pdata->getModuleByName(module_name);
        if (mod == nullptr) {
          throw std::runtime_error("Module " + module_name + " not found in moduleMap");
        } else {
        }
        auto pin = std::make_shared<Pin>();
        if (pin_name == "I") {
          pin->idx = 0;
        } else if (pin_name == "O") {
          pin->idx = 1;
        } else if (pin_name == "B") {
          pin->idx = 2;
        }
        pin->module = mod;
        pin->offset = POS_2D(x_offset, y_offset);
        pin->net = net;
        mod->modulePins.push_back(pin);
        mod->nets.push_back(net);
        // read
        net->netPins.push_back(pin);
        pdata->Pins.push_back(pin);}
      }
      pdata->Nets.push_back(net);
    }
  }
}


void FileReader::read_scl(const fs::path& scl_path)
{
    if (scl_path.extension() != ".scl") {
        throw std::runtime_error("File extension is not .scl");
    }

    std::ifstream file(scl_path);
    if (!file) {
        throw std::runtime_error("File not found/opened: " + scl_path.filename().string());
    }

    std::string line;
    int numRows = 0;
    while (std::getline(file, line)) {
        // 去除注释和空行
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "NumRows") {
            iss.ignore(256, ':');
            iss >> numRows;
            pdata->numRows = numRows; // 如果PlaceData有这个字段
        } else if (token == "CoreRow") {
            // 进入CoreRow块
            std::string orientation;
            iss >> orientation; // Horizontal
            int Coordinate = 0, Height = 0, Sitewidth = 0, Sitespacing = 0;
            int Siteorient = 0, Sitesymmetry = 0, SubrowOrigin = 0, NumSites = 0;

            // 逐行读取CoreRow参数
            while (std::getline(file, line)) {
                if (line.find("End") != std::string::npos) break;
                std::istringstream row_iss(line);
                std::string key;
                row_iss >> key;
                if (key == "Coordinate") {
                    row_iss.ignore(256, ':');
                    row_iss >> Coordinate;
                } else if (key == "Height") {
                    row_iss.ignore(256, ':');
                    row_iss >> Height;
                } else if (key == "Sitewidth") {
                    row_iss.ignore(256, ':');
                    row_iss >> Sitewidth;
                } else if (key == "Sitespacing") {
                    row_iss.ignore(256, ':');
                    row_iss >> Sitespacing;
                } else if (key == "Siteorient") {
                    row_iss.ignore(256, ':');
                    row_iss >> Siteorient;
                } else if (key == "Sitesymmetry") {
                    row_iss.ignore(256, ':');
                    row_iss >> Sitesymmetry;
                } else if (key == "SubrowOrigin") {
                    row_iss.ignore(256, ':');
                    row_iss >> SubrowOrigin;
                    std::string dummy;
                    row_iss >> dummy; // NumSites
                    row_iss.ignore(256, ':');
                    row_iss >> NumSites;
                }
            }
        }
    }
    std::cout << "read_scl done" << std::endl;
}