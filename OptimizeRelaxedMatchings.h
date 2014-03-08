#ifndef OPTIMIZERELAXED_H
#define OPTIMIZERELAXED_H

#include "TextInfo.h"
#include <random>
#include <vector>
#include <iomanip>

using namespace std;

template <class Atom>
struct Optimizer
{
    typedef pair<int,int> Edge;
    static constexpr double SingleCost = 10.0;

    const TextInfoBeta<Atom> &a, &b;
    vector< vector<bool> > graphMatrix;
    vector< vector<double> > variables;
    vector< Edge > allEdges;

    inline double& value(const Edge& edge) {
        return variables[edge.first][edge.second];
    }

    Optimizer(const TextInfoBeta<Atom>& _a, const TextInfoBeta<Atom>& _b) :
        a(_a),
        b(_b)
    {
        graphMatrix.resize(a.size(), vector<bool>(b.size(), false));
        variables.resize(a.size(), vector<double>(b.size(), 0.0));
        for (int ia=0; ia<a.size(); ia++)
            for (int ib=0; ib<b.size(); ib++)
                if (a.atomValue(ia) == b.atomValue(ib)) {
                    graphMatrix[ia][ib] = true;
                    allEdges.push_back(Edge(ia,ib));
                }
    }

    // ===================================================================

    template <class Callback>
    void forEveryLeft(int j0, Callback &call)
    {
        for (int i=0; i<a.size(); i++)
            if (graphMatrix[i][j0])
                call(i,j0,variables[i][j0]);
    }

    template <class Callback>
    void forEveryRight(int i0, Callback &call)
    {
        for (int j=0; j<b.size(); j++)
            if (graphMatrix[i0][j])
                call(i0,j,variables[i0][j]);
    }

    struct Diver
    {
        double what;
        Diver(double w) : what(w) {}
        void operator()(int,int,double& d) { d /= what; }
    };

    struct Summer
    {
        double sum;
        Summer() { sum = 0.; }
        void operator()(int,int,double &d) { sum += d; }
    };

    struct SumCrossing
    {
        Optimizer<Atom>& opt;
        double sum;
        SumCrossing(Optimizer<Atom>& o) : opt(o), sum(0.0) {}
        void operator()(int i, int j, double& d) {
            sum += d * opt.crossingEdges(Edge(i,j));
        }
    };

    // ==========================================================

    double crossingEdges(const Edge& edge)
    {
        // vector<Edge> ret;
        double sum = 0.0;
        for (int ia=0; ia<a.size(); ia ++) if (ia != edge.first) {
            int start = ia<edge.first ? edge.second+1 : 0;
            int end   = ia<edge.first ? b.size() : edge.second;
            for (int ib=start; ib<end; ib++)
                if (graphMatrix[ia][ib])
                    sum += variables[ia][ib];
        }
        // ret.push_back(Edge(ia,ib));
        return sum;
    }

    void satisfyConstraints(const Edge &edge0)
    {
        const int i0 = edge0.first, j0 = edge0.second;
        double& val = value(edge0);
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;

        // sum x_ij over i
        Summer summer1;
        forEveryLeft(j0, summer1);
        Diver diver1(summer1.sum);
        if (summer1.sum > 1.0)
            forEveryLeft(j0, diver1);

        // sum x_ij over j
        Summer summer2;
        forEveryRight(i0, summer2);
        Diver diver2(summer2.sum);
        if (summer2.sum > 1.0)
            forEveryRight(i0, diver2);

        // for every (p,q), x_pq can only decrease, so will not cause other constraints to be violated
    }

    void optimizeOne(double lambda, const Edge& edge)
    {
        double der = crossingEdges(edge) - SingleCost;
        value(edge) += lambda * (der < 0 ? 1.0 : -1.0);
        satisfyConstraints(edge);
    }

    double objectiveValue()
    {
        double obj = 0.0;
        for (int i=0; i<a.size(); i++) {
            SumCrossing summerc(*this);
            forEveryRight(i, summerc);
            obj += summerc.sum / 2.0;

            Summer summer;
            forEveryRight(i, summer);
            obj -= SingleCost * summer.sum;
        }

        return obj;
    }

    void optimize(unsigned int seed, unsigned int rounds)
    {
        srand(seed);
        for (int i=0; i<a.size(); i++)
            for (int j=0; j<b.size(); j++)
                if (graphMatrix[i][j])
                    variables[i][j] = 0.0;

        double obj = objectiveValue();
        cerr << "Initial objective = " << obj << endl;
        double lambda = 0.3;

        vector<Edge> permutation = allEdges;
        random_shuffle(permutation.begin(), permutation.end());
        for (int round=0; round<rounds; round++)
        {
            for (int i=0; i<permutation.size(); i++) {
                Edge edge = permutation[i];
                optimizeOne(lambda, edge);
                if (i%10 == 0) {
                    obj = objectiveValue();
                    cerr << "  Objective = " << obj << endl;
                }
            }
            obj = objectiveValue();
            cerr << "After " << round << " rounds, objective = " << obj << endl;
        }
    }

    Graph retrieveMatching()
    {
        // we assume there will never be a situation where
        // 1. N(i) = {j}, 2. x_ij=0.5

        cerr << "Output variable matrix\n";
        Graph matching(a.size(), b.size());
        for (int i=0; i<a.size(); i++) {
            double sum = 0.0, maxVal = 0.0;
            int maxInd = -1;
            cerr << a.atomValue(i) << " ";
            for (int j=0; j<b.size(); j++) {
                if (graphMatrix[i][j]) {
                    double var = variables[i][j];
                    cerr << std::setprecision(2) << std::fixed << var << " ";
                    sum += var;
                    if (var > maxVal) { maxVal = var; maxInd = j; }
                }
                else {
                    cerr << "- ";
                }
            }
            double other = 1.0 - sum;
            assert(other >= -0.0000001);
            if (maxVal > other) {
                matching.addEdges(matching.left(i), matching.right(maxInd));
            }
            cerr << endl;
            // else: do not add edge, the symbol is inserted
        }

        return matching;
    }
};



#endif // OPTIMIZERELAXED_H
