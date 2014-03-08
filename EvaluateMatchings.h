#ifndef EVALUATEMATCHINGS_H
#define EVALUATEMATCHINGS_H

#include "TextInfo.h"
#include "Utilities.h"
#include "Output.h"
#include "Graph.h"
#include "AnalyzeText.h"

template <class Atom>
struct MatchingEvaluator
{
    const TextInfoBeta<Atom> &a, &b;
    double bestLoss;
    Graph bestMatching;
    MatchingEvaluator(const TextInfoBeta<Atom>& _a, const TextInfoBeta<Atom>& _b) :
        a(_a),
        b(_b),
        bestLoss(1e9),
        bestMatching(a.size(), b.size()),
        matching(a.size(), b.size()),
        cnter(0) {}

    void findBest() {
        findBest(-1, 0);
    }

    double movePenalty(int distance) {
        if (distance == 0) return 0.0;
        return 5.0+sqrt(distance);
    }
    double insertDeletePenalty() {
        return 2.0;
    }

    Graph matching;
    int cnter;
    double evaluate() {
        double loss = 0.0;
        int prevRight = -1;
        cerr << "Check matching nr " << cnter ++ << endl;
        debugDumpAsText(matching, "matching");
        for (int leftInd = 0; leftInd < a.size(); leftInd ++) {
            int leftNode = matching.left(leftInd);
            assert(matching.getOutDegree(leftNode) <= 1);
            if (matching.getOutDegree(leftNode) == 0) {
                loss += 2.0*insertDeletePenalty();
                cerr << "  Left " << leftInd << " unmatched\n";
            }
            else { // is matched
                int right = matching.whichRight(matching.getOutEdgeEnd(leftNode, 0));
                // check if how many of prevRight+1, ..., right-1 are matched -> it is a distance between edges
                int matchedBetween = 0;
                if (prevRight != -1) {
                    // there are two cases: prevRight < right, prevRight > right
                    int tmp1 = min(prevRight, right), tmp2 = max(prevRight, right);
                    for (int j=tmp1+1; j<tmp2; j ++) {
                        assert(matching.getOutDegree(matching.right(j)) <= 1);
                        matchedBetween += matching.getOutDegree(matching.right(j));
                    }
                }
                loss += movePenalty(matchedBetween);
                cerr << "  Left " << leftInd << ": # of matched between " << matchedBetween << endl;
                prevRight = right;
            }
        }

        cerr << "Total loss: " << loss << endl;
        // add loss coming from deleted atoms on the left hand side?
        // NOT, because it is already included in unmatched left atoms.

        return loss;
    }
    void findBest(int prevRight, int leftInd)
    {
        if (leftInd == a.size()) {
            double loss = evaluate();
            if (loss < bestLoss) {
                bestLoss = loss;
                bestMatching = matching;
            }
            return ;
        }

        string currAtom = a.atomValue(leftInd);
        int leftNode = matching.left(leftInd);
        // check if I can prolong the previous block
        if (prevRight != -1 && prevRight+1 < b.size() &&
                currAtom == b.atomValue(prevRight+1)) {
            matching.addEdges(leftNode, matching.right(prevRight+1));
            findBest(prevRight+1, leftInd+1);
            matching.isolate(leftNode); // clean after yourself!
            return ;
        }

        // check delete. Do not add any edge to matching, do not change prev right
        findBest(prevRight, leftInd+1);

        // try to match a token
        for (int right=0; right<b.size(); right ++) {
            int rightNode = matching.right(right);
            if (matching.getOutDegree(rightNode) == 0 &&
                    currAtom == b.atomValue(right)) {
                matching.addEdges(leftNode, rightNode);
                findBest(right, leftInd+1);
                matching.isolate(leftNode); // clean after yourself!
            }
        }
    }
};





#endif // EVALUATEMATCHINGS_H
