#include "placedata.hpp"

// 对于固定的线长公式，化简为矩阵形式的参数是确定的，所以总线长等于多个线长相加（即不同连线对应的 A, b 累加），最终得到总的线长矩阵参数，最后求导 A, B 参数不变。
// 可以理解为线长公式的参数是固定的，3种情况对总线长的贡献度不同，把每个参数相加，得到总的线长矩阵参数。再求导
// 得到的结果就是A ，b 两个参数矩阵
void caclculate_parm_matrix(
    const vector<shared_ptr<Module>>& modules,
    const vector<shared_ptr<Net>>& nets,
    vector<unordered_map<int, double>>& A,
    vector<double>& b)
{
    int N = modules.size();
    A.assign(N, {});
    b.assign(N, 0.0);
    // printf("N = %d\n", N);

    for (const auto& net : nets)
    {
        const auto& pins = net->netPins;
        int pinCount = pins.size();

        if (pinCount < 2)
            continue;

        // 简单权重模型（可以用文献中的定义替换）
        double w = 1.0 / (pinCount - 1);

        // 枚举每一对引脚 (p, q)
        for (int i = 0; i < pinCount; ++i)
        {
            for (int j = i + 1; j < pinCount; ++j)
            {


                auto p = pins[i];
                auto q = pins[j];

                if (p == nullptr || q == nullptr)
                {
                    throw std::runtime_error("Invalid pin pointer");
                }

                shared_ptr<Module> mp = p.get()->module.lock();
                shared_ptr<Module> mq = q.get()->module.lock();

                if (mp->idx < 0 || mp->idx >= N)
                {
                    throw std::runtime_error("p Invalid module index");
                }
                if (mq->idx < 0 || mq->idx >= N)
                {
                    throw std::runtime_error("q Invalid module index");
                }
                // 考虑偏移（pin相对于module中心的offset）
                double off_p = p->offset.x;
                double off_q = q->offset.x;
                double delta = off_p - off_q;

                // Case (i): p,q 都可移动
                if (!mp->isFixed && !mq->isFixed)
                {

                    A[mp->idx][mp->idx] += w;
                    A[mq->idx][mq->idx] += w;
                    A[mp->idx][mq->idx] -= w;
                    A[mq->idx][mp->idx] -= w;

                    b[mp->idx] += 2 * w * delta;
                    b[mq->idx] += -2 * w * delta;
                }

                // Case (ii): p movable, q terminal
                if (!mp->isFixed && mq->isFixed)
                {
                    double const_xq = mq->center.x;
                    A[mp->idx][mp->idx] += w;
                    double xq = delta - const_xq;
                    b[mp->idx] += 2 * w * xq;
                }
                // Case (iii): p terminal, q movable
                if (mp->isFixed && !mq->isFixed)
                {
                    double const_xp = mp->center.x;
                    A[mq->idx][mq->idx] += w;
                    double xp = delta + const_xp;
                    b[mq->idx] += 2 * w * xp;
                }
            }
        }
    }
}

std::pair<vector<unordered_map<int, double>>, vector<double>> PlaceData::get_parm_matrix()
{

    vector<unordered_map<int, double>> A;
    vector<double> b;
    caclculate_parm_matrix(Nodes, Nets, A, b);

    return std::make_pair(move(A), move(b));
}