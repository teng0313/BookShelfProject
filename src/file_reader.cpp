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
     Timer timer;
    if(scl_path.extension() != ".scl"){
       throw std::runtime_error("File extension is not .scl");
    }
    
    std::ifstream file(scl_path);
    if(!file){
        throw std::runtime_error("File not found/opened: " + scl_path.filename().string());
    }

    // 解析 .scl 文件
    std::string line;
    int numRows = 0;
    
    // 跳过文件头部信息
    while(std::getline(file, line)) {
        if(line.find("NumRows") != std::string::npos) {
            // 提取行数
            auto tokens = splist_by_space(line);
            if(tokens.size() >= 3 && tokens[0] == "NumRows" && tokens[1] == ":") {
                numRows = std::stoi(tokens[2]);
                break;
            }
        }
    }
    
    // 开始解析每一行的CoreRow信息
    pdata->siteRows.reserve(numRows);
    
    while(std::getline(file, line)) {
        if(line.find("CoreRow") != std::string::npos) {
            // 找到一个CoreRow开始
            auto siteRow = std::make_shared<SiteRow>();
            
            // 解析CoreRow信息
            while(std::getline(file, line) && line.find("End") == std::string::npos) {
                auto tokens = splist_by_space(line);
                if(tokens.empty()) continue;
                
                if(tokens[0] == "Coordinate" && tokens.size() >= 3) {
                    // 底部y坐标
                    siteRow->bottom = std::stod(tokens[2]);
                }
                else if(tokens[0] == "Height" && tokens.size() >= 3) {
                    // 行高
                    siteRow->height = std::stod(tokens[2]);
                }
                else if(tokens[0] == "Sitewidth" && tokens.size() >= 3) {
                    // 站点宽度 - 用于计算步长
                    double sitewidth = std::stod(tokens[2]);
                    siteRow->step = sitewidth;
                }
                else if(tokens[0] == "Sitespacing" && tokens.size() >= 3) {
                    // 站点间距 - 可以用于调整步长
                    double sitespacing = std::stod(tokens[2]);
                    if(sitespacing != 1.0) { // 如果不是默认值1.0，则更新步长
                        siteRow->step = sitespacing;
                    }
                }
                else if(tokens[0] == "Siteorient" && tokens.size() >= 3) {
                    // 方向 N (0) 或 S (1)
                    int orient = std::stoi(tokens[2]);
                    siteRow->orientation = (orient == 1) ? ORIENT::S : ORIENT::N;
                }
                else if(tokens[0] == "SubrowOrigin" && tokens.size() >= 5) {
                    // 开始x坐标
                    double origin_x = std::stod(tokens[2]);
                    // 站点数量
                    int numSites = std::stoi(tokens[4]);
                    
                    // 设置开始和结束坐标
                    siteRow->start = POS_2D(origin_x, siteRow->bottom);
                    siteRow->end = POS_2D(origin_x + numSites * siteRow->step, siteRow->bottom);
                    
                    // 创建间隔
                    Interval interval;
                    interval.ll = siteRow->start;
                    interval.ur = siteRow->end;
                    siteRow->intervals.push_back(interval);
                }
            }
            
            // 添加解析完成的SiteRow到pdata中
            pdata->siteRows.push_back(siteRow);
        }
    }
    
    std::cout << "read_scl done. Parsed " << pdata->siteRows.size() << " site rows." << std::endl;
}

void FileReader::read_nodes(const std::filesystem::path& nodes_path) {
    std::cout << "Opening file: " << nodes_path << std::endl;

    // 检查文件扩展名
    if (nodes_path.extension() != ".nodes") {
        throw std::runtime_error("File extension is not .nodes");
    }

    // 打开文件
    std::ifstream file(nodes_path);
    if (!file) {
        throw std::runtime_error("File not found/opened: " + nodes_path.filename().string());
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行与注释
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        // 文件头信息
        if (token == "NumNodes") {
            iss.ignore(256, ':');
            iss >> pdata->numNodes;
        }
        else if (token == "NumTerminals") {
            iss.ignore(256, ':');
            iss >> pdata->numTerminals;
        }
        else {
            // 普通节点定义
            std::string nodeName = token;
            float width = 0.0f, height = 0.0f;
            iss >> width >> height;

            bool isTerminal = false;
            std::string flag;
            if (iss >> flag && flag == "terminal") {
                isTerminal = true;
            }

            // 填充Node信息
            Node node;
            node.name = nodeName;
            node.width = width;
            node.height = height;
            node.isTerminal = isTerminal;
            pdata->v_Node.push_back(node);

            // 同时构建Module对象
            auto mod = std::make_shared<Module>();
            mod->Init();
            mod->name = nodeName;
            mod->width = width;
            mod->height = height;
            mod->isFixed = isTerminal;
            pdata->moduleMap[nodeName] = mod;
        }
    }

    std::cout << "read_nodes done" << std::endl;
    std::cout << "numNodes=" << pdata->numNodes << std::endl;
    std::cout << "numTerminals=" << pdata->numTerminals << std::endl;

    if (!pdata->v_Node.empty()) {
        const auto& n = pdata->v_Node.front();
        std::cout << "example:" << endl;
        std::cout
            << "name:" << n.name << " "
            << "width:" << n.width << " "
            << "height:" << n.height << " "
            << "isTerminal" << " " << (n.isTerminal ? "yes" : "no")
            << std::endl;
    }
}

