#ifndef PLACEDATA_H
#define PLACEDATA_H
#include "objects.hpp"
#include <cmath>

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
    shared_ptr<Module> getModuleByName(const string &name)
    {
        if (moduleMap.find(name) != moduleMap.end())
        {
            return moduleMap[name];
        }
        return nullptr;
    }

    void print_info()
    {
        int num_modules = Nodes.size();
        int macro_count = 0;
        int fixed_count = 0;
        for (auto module : Nodes)
        {
            if (module->isMacro)
                MacroCount++;
            if (module->isFixed)
                fixed_count++;
        }
        std::ofstream fout("/home/ezio/ClassProject3/info.txt");
        fout << "Object #:" << num_modules << "(=" << static_cast<int>(num_modules / 1000) << "k)" << "(fixed:" << fixed_count << ")(macro:" << macro_count << ") " << std::endl;
    }

    
    //TODO variables
    double core_area {0};






    
    // task 4
    // （1）计算空白面积 = 布局行 SiteRow 区域面积-（terminal 在布局行区域内占
    // 用的总面积）
    double empty_area;
    // 计算总填充面积 = 空白面积* targetDensity - （标准单元面积 + 宏块面积*
    // targetDensity）
    double total_area;



    // targetDensity 为设定的目标密度
    double target_density;
    // （2）设定填充节点高 = 常规高度，其中常规高度是读取*.scl 文件中读取。
    double setting_height;
    // （3）计算填充节点宽 = 填充节点面积/填充节点高
    double setting_width;
    // （4）计算填充节点的个数 = 总填充面积 / 填充节点面积
    double padding_num;
    // 计算填充节点面积：将所有节点按面积从大到小排序，去掉前后 5%，将
    //  剩下的 90%的节点面积求平均，得到填充节点面积。

    double calculate_padding_area();

    void create_padding_nodes(double setting_height, double setting_width,
                              double padding_area, int padding_num);


    double node_area{0};
    double average_node_area{0};
    double expected_grid_area{0};
    uint64_t expected_grid_num{0};
    vector<vector<Bin>> bins {};

    // 计算节点面积 = 标准单元面积 + 宏块面积
    double calculate_node_area();
    // 计算平均节点面积 = 节点面积/节点数目
    double calculate_average_node_area();
    // 计算期望网格面积 = 平均节点面积 / targetDensity
    double calculate_expected_grid_area();
    // 计算期望网格数目 M = 布局区域的面积 / 期望网格面积
    uint64_t calculate_expected_grid_num();
    // 计算网格的维数（行，列）
    //  行数 2^n = 列数 2^n，且使得 M 介于 2^n 与 2^(n+1) 之间。
    std::tuple<uint64_t, uint64_t> grid_dimensions();
    // 计算网格的左上（bins[i][j]->ll.x，bins[i][j]->ll.y）、右下（bins[i][j]->ur.x、
    // bins[i][j]->ur.y），中心（bins[i][j]->center.x，bins[i][j]->center.y）等位置的坐标、
    // 宽 bins[i][j]->width、高 bins[i][j]->height。
    void calculate_bins();
    // 计算终端 terminal 的密度 terminalDensity
    // 对每一个终端，计算其与网格重叠的区域 OATerminal，将其乘以 targetDensity
    // 后累加，即bins[i][j]->terminalDensity += targetDensity *OATerminal
    void calculate_bins_density();

    // 计算暗节点的密度 darkDensity
    // 网格超出布局行 SiteRow 的区域称为暗节点，对每一个暗节点，计算网格超
    // 出布局行 SiteRow 的区域面积 OutADark，将其乘以 targetDensity 后累加，即
    // bins[i][j]->darkDensity += targetDensity *OutADark

    void initial_layout_design();

    void terminal_and_terminal();

    void non_terminal_and_non_terminal();

    void terminal_and_non_terminal();
};
#endif