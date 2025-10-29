#include "placedata.hpp"

// 计算节点面积 = 标准单元面积 + 宏块面积
double PlaceData::calculate_node_area()
{
    double node_area_calculate = 0;
    for (auto &node : Nodes)
    {
        // add standard cell and macro area
        if (node->isFixed == false && node->isFiller == false)
        {
            node_area_calculate += node->getArea();
        }
    }
    node_area = node_area_calculate;
    return node_area;
}
// 计算平均节点面积 = 节点面积/节点数目
double PlaceData::calculate_average_node_area()
{
    if (node_area == 0)
    {
        throw std::runtime_error("Node area is zero, you need to call calculate_node_area() first");
    }

    if (Nodes.size() == 0)
    {
        throw std::runtime_error("Nodes size is zero");
    }

    average_node_area = node_area / Nodes.size();
    return average_node_area;
}
// 计算期望网格面积 = 平均节点面积 / targetDensity
double PlaceData::calculate_expected_grid_area()
{
    if (average_node_area == 0)
    {
        throw std::runtime_error("Average node area is zero, you need to call calculate_average_node_area() first");
    }

    if (target_density == 0)
    {
        throw std::runtime_error("Target density is zero");
    }

    expected_grid_area = average_node_area / target_density;
    return expected_grid_area;
}
// 计算期望网格数目 M = 布局区域的面积 / 期望网格面积
uint64_t PlaceData::calculate_expected_grid_num()
{
    if (expected_grid_area == 0)
    {
        throw std::runtime_error("Expected grid area is zero, you need to call calculate_expected_grid_area() first");
    }
    expected_grid_num = core_area / expected_grid_area;
    return expected_grid_num;
}

// 计算网格的维数（行，列）
//  行数 2^n = 列数 2^n，且使得 M 介于 2^n 与 2^(n+1) 之间。
std::tuple<uint64_t, uint64_t> PlaceData::grid_dimensions(){
    uint64_t sqrt_m = std::sqrt(expected_grid_num);
    uint64_t n = std::log2(sqrt_m);
    uint64_t m = std::pow(2, n);
    bins.resize(m);
    for(uint64_t i = 0; i < m; i++){
        bins[i].resize(m);
    }
    return std::make_tuple(m, m);
}
// 计算网格的左上（bins[i][j]->ll.x，bins[i][j]->ll.y）、右下（bins[i][j]->ur.x、
// bins[i][j]->ur.y），中心（bins[i][j]->center.x，bins[i][j]->center.y）等位置的坐标、
// 宽 bins[i][j]->width、高 bins[i][j]->height。
void PlaceData::calculate_bins(){
    int bin_width = std::sqrt(expected_grid_area);
    int bin_height = bin_width;
    for (int i = 0; i < bins.size(); i++)
    {
        for (int j = 0; j < bins[i].size(); j++)
        {
            bins[i][j].height = bin_height;
            bins[i][j].width = bin_width;
            // 左上
            bins[i][j].ll.x = j * bin_width;
            bins[i][j].ll.y = i * bin_height+bin_height;
            //right down
            bins[i][j].ur.x = bins[i][j].ll.x + bin_width;
            bins[i][j].ur.y = bins[i][j].ll.y - bin_height;
            // center
            bins[i][j].center.x = (bins[i][j].ll.x + bins[i][j].ur.x) / 2;
            bins[i][j].center.y = (bins[i][j].ll.y + bins[i][j].ur.y) / 2;
        }
    }


}


// 计算布局区域总面积
double PlaceData::calculate_site_rows_area() {
    double total_area = 0.0;
    for (const auto& row : SiteRows) {
        total_area += row.getSizeRowArea();
    }
    return total_area;
}

// 判断一个模块是否在布局行内，以及计算重叠面积
double calculate_overlap_area(const shared_ptr<Module>& module, const SiteRow& row) {
    // 模块的四个角坐标
    double module_left = module->center.x - module->width / 2;
    double module_right = module->center.x + module->width / 2;
    double module_bottom = module->center.y - module->height / 2;
    double module_top = module->center.y + module->height / 2;
    
    // 布局行的四个角坐标
    double row_left = row.start.x;
    double row_right = row.end.x;
    double row_bottom = row.bottom;
    double row_top = row.bottom + row.height;
    
    // 计算x方向的重叠
    double x_overlap = std::max(0.0, 
                      std::min(module_right, row_right) - 
                      std::max(module_left, row_left));
                      
    // 计算y方向的重叠
    double y_overlap = std::max(0.0, 
                      std::min(module_top, row_top) - 
                      std::max(module_bottom, row_bottom));
                      
    // 返回重叠面积
    return x_overlap * y_overlap;
}

// 计算terminals在布局行内的占用总面积
double PlaceData::calculate_terminals_area_in_site_rows() {
    double terminal_area = 0.0;
    
    // 遍历每个terminal
    for (const auto& terminal : Terminals) {
        double terminal_overlap_area = 0.0;
        
        // 对每个布局行，计算该terminal在其中的重叠面积
        for (const auto& row : SiteRows) {
            terminal_overlap_area += calculate_overlap_area(terminal, row);
        }
        
        // 累加到总面积
        terminal_area += terminal_overlap_area;
    }
    
    return terminal_area;
}

// 计算空白面积
double PlaceData::calculate_empty_area() {
    double site_rows_area = calculate_site_rows_area();
    double terminals_area = calculate_terminals_area_in_site_rows();
    
    empty_area = site_rows_area - terminals_area;
    return empty_area;
}

// 计算标准单元和宏块面积
void PlaceData::calculate_std_cell_and_macro_area() {
    std_cell_area = 0.0;
    macro_area = 0.0;
    
    for (const auto& node : Nodes) {
        // 跳过固定节点和填充节点
        if (node->isFixed || node->isFiller) {
            continue;
        }
        
        if (node->isMacro) {
            macro_area += node->getArea();
        } else {
            std_cell_area += node->getArea();
        }
    }
}

// 计算总填充面积
double PlaceData::calculate_total_fill_area() {
    if (empty_area == 0) {
        calculate_empty_area();
    }
    
    calculate_std_cell_and_macro_area();
    
    // 根据公式计算总填充面积
    total_fill_area = empty_area * target_density - (std_cell_area + macro_area * target_density);
    
    return total_fill_area;
}