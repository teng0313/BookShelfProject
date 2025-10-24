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