#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <string.h>
#include <string>
#include <map>
#include <queue>
#include <algorithm>
using namespace std;
const long long EMAX = 2e7;

class graph
{
public:
	string RDF;
	int part;
	graph();
	~graph();
	void init();
	vector<string> split(string textline, string tag);
	void loadGraph(string txt_name, string tag);
	int getPreNum();
	long long getEntityNum();
	int getParent(int son, vector<int> &fa);
	int getParentMap(int son, unordered_map<int, int> &fa);
	void coarsening();
	void unionBlock(vector<int> &choice, int goal);
	void unionEdgeGreed();
	void greed(vector<int> &choice, vector<int> &curParent, vector<int> &curEdgeCnt, vector<int> &curRank, vector<bool> &invalid);
	void computeWccs(unordered_map<int, int> &coarseningPoint, vector<int> &coarseningEdgeCnt, int preID);

private:
	set<string> predicate;
	unordered_map<string, int> entityToID;
	vector<string> IDToEntity;
	unordered_map<string, int> predicateToID;
	vector<string> IDToPredicate;
	vector<vector<tuple<int,int,int>>> edge;

	// key: entityID		value: the count of triples containing entityID
	vector<int> entityTriples;
	// edge_cnt : the key is the property and the value is the count of the property
	unordered_map<string, int> edge_cnt;

	vector<bool> invalid;
	long long triplesCnt;
	// entityCnt : entity count
	long long entityCnt;
	// predicate count
	int preType;

	long long limit;
	long long invalidEdgeCnt;

	// coarseningPoint[preID]表示preID的谓词
	vector<unordered_map<int, int>> coarseningPoint;
	vector<unordered_map<int, int>> coarseningEdgeCnt;

	map<pair<int, int>, vector<tuple<int,int,int>>> queryTriplets;
};