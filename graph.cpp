#include "graph.h"
#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime>
clock_t sst, eed;
double coarseningtime;

graph::graph() {}

graph::~graph() {}

void graph::init()
{
	vector<tuple<int, int, int>> tmp1;
	edge.push_back(tmp1);

	entityTriples.push_back(0);

	IDToEntity.push_back("");
	IDToPredicate.push_back("");
	preType = entityCnt = triplesCnt = invalidEdgeCnt = 0;
}

int graph::getPreNum()
{
	return predicate.size();
}

long long graph::getEntityNum()
{
	return (long long)IDToEntity.size() - 1;
}

vector<string>
graph::split(string textline, string tag)
{
	vector<string> res;
	std::size_t pre_pos = 0;
	std::size_t pos = textline.find(tag);

	while (pos != std::string::npos)
	{
		if (res.size() == 2)
		{
			pos = textline.length() - 1;
			break;
		}
		string curStr = textline.substr(pre_pos, pos - pre_pos);
		curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
		curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);

		if (strcmp(curStr.c_str(), "") != 0)
			res.push_back(curStr);
		pre_pos = pos + tag.size();
		pos = textline.find(tag, pre_pos);
	}

	string curStr = textline.substr(pre_pos, pos - pre_pos);
	curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
	curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);
	if (strcmp(curStr.c_str(), "") != 0)
		res.push_back(curStr);

	return res;
}

void graph::loadGraph(string txt_name, string tag)
{
	ifstream in(txt_name);

	vector<tuple<int, int, int>> tmp1;
	string line;
	while (getline(in, line))
	{
		if (triplesCnt % 100000000 == 0)
			cout << "loading triplesCnt : " << triplesCnt << endl;
		triplesCnt++;
		line.resize(line.length() - 2);
		vector<string> s;
		s = split(line, tag);
		predicate.insert(s[1]);

		// Determine whether this entity is valid
		for (int i = 0; i < 3; i += 2)
			if ((s[i][0] == '<') && entityToID.count(s[i]) == 0)
			{
				entityToID[s[i]] = ++entityCnt;
				IDToEntity.push_back(s[i]);
				entityTriples.push_back(0);
			}

		edge_cnt[s[1]]++;
		int a = entityToID[s[0]];
		int b = entityToID[s[2]];
		entityTriples[a]++;

		// 1. Determine whether predicate & rdf are valid
		// 2. If the predicate is valid, add the corresponding subject and object
		if ((s[0][0] == '<') && (s[2][0] == '<'))
		{
			if (predicateToID.count(s[1]) == 0)
			{
				predicateToID[s[1]] = ++preType;
				IDToPredicate.push_back(s[1]);
				edge.push_back(tmp1);
			}
			int preid = predicateToID[s[1]];
			edge[preid].push_back(make_tuple(a, b, 0));
			entityTriples[b]++;
		}
	}
	in.close();

	// Can be adjusted according to specific needs
	limit = entityCnt / part / 1.2;
	printf("limit: %lld\n", limit);
	printf("triplesCnt: %lld\n", triplesCnt);
	printf("entityCnt: %lld\n", entityCnt);
	printf("predicate: %lu\n", predicate.size());
	printf("entity->preType: %d\n", preType);
	printf("sizeof edge_cnt : %ld\n", edge_cnt.size());
}

int graph::getParent(int son, vector<int> &fa)
{
	int i, j, k;
	k = son;
	while (k != fa[k])
		k = fa[k];
	i = son;
	while (i != k)
	{
		j = fa[i];
		fa[i] = k;
		i = j;
	}
	return k;
}

int graph::getParentMap(int son, unordered_map<int, int> &fa)
{
	int i, j, k;
	k = son;
	while (k != fa[k])
		k = fa[k];
	i = son;
	while (i != k)
	{
		j = fa[i];
		fa[i] = k;
		i = j;
	}
	return k;
}

void graph::coarsening()
{
	invalid = vector<bool>(preType + 1, 0);
	coarseningPoint = vector<unordered_map<int, int>>(preType + 1, unordered_map<int, int>());
	coarseningEdgeCnt = vector<unordered_map<int, int>>(preType + 1, unordered_map<int, int>());
	for (int preID = 1; preID <= preType; preID++)
	{
		vector<int> rank = vector<int>(entityCnt + 1, 0);
		for (int p = 0; p < edge[preID].size(); p++)
		{
			int A = get<0>(edge[preID][p]), B = get<1>(edge[preID][p]);

			if (coarseningPoint[preID].count(A) == 0)
				coarseningPoint[preID].insert(make_pair(A, A));
			if (coarseningPoint[preID].count(B) == 0)
				coarseningPoint[preID].insert(make_pair(B, B));

			int parentA = getParentMap(A, coarseningPoint[preID]), parentB = getParentMap(B, coarseningPoint[preID]);
			if (rank[parentA] < rank[parentB])
				swap(parentA, parentB);
			if (parentA != parentB)
			{
				coarseningPoint[preID][parentB] = parentA;
				if (coarseningPoint[preID].count(parentA) == 0)
					coarseningPoint[preID].insert(make_pair(parentA, 0));
				if (coarseningPoint[preID].count(parentB) == 0)
					coarseningPoint[preID].insert(make_pair(parentB, 0));
				coarseningEdgeCnt[preID][parentA] += coarseningEdgeCnt[preID][parentB] + 1;
				rank[parentA] = max(rank[parentA], rank[parentB] + 1);
				get<2>(edge[preID][p]) = parentA;

				// The size of a WCC exceeds the limit
				if (coarseningEdgeCnt[preID][parentA] > limit && !invalid[preID])
				{
					invalid[preID] = 1;
					invalidEdgeCnt++;
					printf("invalid: %d %s\n", preID, IDToPredicate[preID].data());
				}
			}
			else
				coarseningEdgeCnt[preID][parentA] += 1;
		}
	}
}

void graph::unionEdgeGreed()
{
	puts("\n==========================================================================================\n");
	cout << "start:" << endl;
	
	coarsening();

	vector<int> internalPre;
	vector<int> choice(preType + 1, 0);
	vector<int> parent = vector<int>(entityCnt + 1);
	for (int i = 1; i <= entityCnt; i++)
		parent[i] = i;
	vector<int> sonCnt = vector<int>(entityCnt + 1, 1);
	vector<int> edgeCnt = vector<int>(entityCnt + 1, 0);
	vector<int> rank = vector<int>(entityCnt + 1, 1);

	puts("\n==========================================================================================\n");
	cout << "greed" << endl;

	int threshold = triplesCnt * 0.001;

	int optim = 0;
	for (int preID = 1; preID <= preType; preID++)
	{
		// If the edge cnt of preID is less than the threshold, it is selected as the inner
		if (edge_cnt[IDToPredicate[preID]] < threshold)
		{
			for (int p = 0; p < edge[preID].size(); p++)
			{

				int A = get<0>(edge[preID][p]), B = get<1>(edge[preID][p]);
				int parentA = getParent(A, parent), parentB = getParent(B, parent);

				if (rank[parentA] < rank[parentB])
					swap(parentA, parentB);
				if (parentA != parentB)
				{
					parent[parentB] = parentA;
					sonCnt[parentA] += sonCnt[parentB];
					edgeCnt[parentA] += edgeCnt[parentB];
					rank[parentA] = max(rank[parentA], rank[parentB] + 1);
				}
			}
			choice[preID] = 1;
			optim++;
			internalPre.push_back(preID);
		}
	}
	printf("opt: %d\n", optim);

	greed(choice, parent, edgeCnt, rank, invalid);

	puts("\n==========================================================================================\n");
	ofstream locFile(RDF + "Loc.txt");
	ofstream File(RDF + "UnLoc.txt");
	int unLoc = 0;
	for (int preID = 1; preID <= preType; preID++)
		if (choice[preID] == 0)
		{
			unLoc++;
			File << IDToPredicate[preID] << endl;
		}
		else
		{
			locFile << IDToPredicate[preID] << endl;
		}

	printf("UnLoc: %d\n", unLoc);
	locFile.close();
	File.close();
	puts("\n==========================================================================================\n");

	unionBlock(choice, part);
}

void graph::greed(vector<int> &choice, vector<int> &curParent, vector<int> &curEdgeCnt, vector<int> &curRank, vector<bool> &invalid)
{
	int nextBestBlockNum = 0;
	int chosen_preID = 0;
	if (true)
	{
		vector<int> nextBestParent;
		vector<int> nextBestEdgeCnt;
		vector<int> nextBestRank;
		vector<int> nextBestChoice(choice);
		for (int preID = 1; preID <= preType; preID++)
		{
			if (choice[preID] == 0 && !invalid[preID])
			{
				vector<int> nextParent(curParent);
				vector<int> nextEdgeCnt(curEdgeCnt);
				vector<int> nextRank(curRank);
				int nextBlockNum = 0;
				bool flag = 0;
				for (auto &it : coarseningPoint[preID])
				{
					int point = it.first;
					if (point == getParentMap(point, coarseningPoint[preID]))
						continue;
					int parentA = getParent(point, nextParent), parentB = getParent(getParentMap(point, coarseningPoint[preID]), nextParent);

					if (nextRank[parentA] < nextRank[parentB])
						swap(parentA, parentB);
					if (parentA != parentB)
					{
						nextParent[parentB] = parentA;
						nextEdgeCnt[parentA] += nextEdgeCnt[parentB] + 1;
						nextRank[parentA] = max(nextRank[parentA], nextRank[parentB] + 1);
						if (nextEdgeCnt[parentA] > limit)
						{
							flag = 1;
							break;
						}
					}
					else
						nextEdgeCnt[parentA] += 1;
				}
				if (flag)
					continue;
				for (int p = 1; p <= entityCnt; p++)
					if (getParent(p, nextParent) == p)
						nextBlockNum++;
				if (!nextBestBlockNum || nextBestBlockNum < nextBlockNum)
				{
					nextBestBlockNum = nextBlockNum;
					nextBestParent.assign(nextParent.begin(), nextParent.end());
					nextBestEdgeCnt.assign(nextEdgeCnt.begin(), nextEdgeCnt.end());
					nextBestRank.assign(nextRank.begin(), nextRank.end());
					nextBestChoice.assign(choice.begin(), choice.end());
					nextBestChoice[preID] = 1;
					chosen_preID = preID;
				}
			}
		}
		choice.assign(nextBestChoice.begin(), nextBestChoice.end());
		curParent.assign(nextBestParent.begin(), nextBestParent.end());
		curEdgeCnt.assign(nextBestEdgeCnt.begin(), nextBestEdgeCnt.end());
		curRank.assign(nextBestRank.begin(), nextBestRank.end());
	}
	if (nextBestBlockNum)
		greed(choice, curParent, curEdgeCnt, curRank, invalid);
}

void graph::computeWccs(unordered_map<int, int> &coarseningPoint, vector<int> &coarseningEdgeCnt, int preID)
{
	vector<int> rank = vector<int>(entityCnt + 1, 0);
	for (int p = 0; p < edge[preID].size(); p++)
	{
		int A = get<0>(edge[preID][p]), B = get<1>(edge[preID][p]);

		// Isolated points, join the WCC with label preID in the form of self-loop
		if (coarseningPoint.count(A) == 0)
			coarseningPoint.insert(make_pair(A, A));
		if (coarseningPoint.count(B) == 0)
			coarseningPoint.insert(make_pair(B, B));

		int parentA = getParentMap(A, coarseningPoint), parentB = getParentMap(B, coarseningPoint);
		if (rank[parentA] < rank[parentB])
			swap(parentA, parentB);
		if (parentA != parentB)
		{
			coarseningPoint[parentB] = parentA;
			coarseningEdgeCnt[parentA] += coarseningEdgeCnt[parentB] + 1;
			rank[parentA] = max(rank[parentA], rank[parentB] + 1);
			get<2>(edge[preID][p]) = parentA;
		}
		else
			coarseningEdgeCnt[parentA] += 1;
		get<2>(edge[preID][p]) = parentA;
	}
}

void graph::unionBlock(vector<int> &choice, int goal)
{
	cout << "unionBlock: " << endl;
	vector<int> rank = vector<int>(entityCnt + 1, 0);
	vector<int> edgeCnt = vector<int>(entityCnt + 1, 0);
	invalid = vector<bool>(preType + 1, 0);

	unordered_map<int, int> loccoarseningPoint;
	vector<int> loccoarseningEdgeCnt = vector<int>(entityCnt + 1, 0);

	vector<tuple<int, int, int>> block;
	int blockNum = 0;

	for (int preID = 1; preID <= preType; preID++)
		if (choice[preID] == 1)
		{
			// Compute the weakly connected subgraph of all local properties
			computeWccs(loccoarseningPoint, loccoarseningEdgeCnt, preID);
		}
		else
		{
			// For the unlocal property, directly calculate the weakly connected subgraph of the property
			unordered_map<int, int> unLoccoarseningPoint;
			vector<int> unLoccoarseningEdgeCnt = vector<int>(entityCnt + 1, 0);
			computeWccs(unLoccoarseningPoint, unLoccoarseningEdgeCnt, preID);
			for (const auto &it : unLoccoarseningPoint)
			{
				int point = it.first;
				if (point == getParentMap(point, unLoccoarseningPoint))
				{
					block.push_back(make_tuple(unLoccoarseningEdgeCnt[point], point, preID)), ++blockNum;
				}
			}

			for (int p = 0; p < edge[preID].size(); p++)
			{
				int parentA;
				parentA = getParentMap(get<2>(edge[preID][p]), unLoccoarseningPoint);
				auto pos = queryTriplets.find(make_pair(parentA, preID));
				if (pos == queryTriplets.end())
				{
					vector<tuple<int, int, int>> value;
					value.push_back(make_tuple(get<0>(edge[preID][p]), preID, get<1>(edge[preID][p])));
					queryTriplets.emplace(make_pair(parentA, preID), value);
				}
				else
				{
					queryTriplets[pos->first].push_back(make_tuple(get<0>(edge[preID][p]), preID, get<1>(edge[preID][p])));
				}
			}
		}

	// Each weakly connected subgraph corresponds to a block
	for (const auto &it : loccoarseningPoint)
	{
		int point = it.first;
		if (point == getParentMap(point, loccoarseningPoint))
		{
			block.push_back(make_tuple(loccoarseningEdgeCnt[point], point, -1)), ++blockNum;
		}
	}

	for (int preID = 1; preID <= preType; preID++)
	{
		if (choice[preID] == 0)
			continue;
		for (int p = 0; p < edge[preID].size(); p++)
		{
			int parentA;
			parentA = getParentMap(get<2>(edge[preID][p]), loccoarseningPoint);
			auto pos = queryTriplets.find(make_pair(parentA, -1));
			if (pos == queryTriplets.end())
			{
				vector<tuple<int, int, int>> value;
				value.push_back(make_tuple(get<0>(edge[preID][p]), preID, get<1>(edge[preID][p])));
				queryTriplets.emplace(make_pair(parentA, -1), value);
			}
			else
			{
				queryTriplets[pos->first].push_back(make_tuple(get<0>(edge[preID][p]), preID, get<1>(edge[preID][p])));
			}
		}
	}

	// Divide all blocks
	printf("blockNum: %d\n", blockNum);

	sort(block.begin(), block.end());

	priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Q;

	for (int i = 1; i <= goal; i++)
		Q.push(make_pair(0, i));

	map<pair<int, int>, int> blockTogoal;
	for (int i = block.size() - 1; i >= 0; i--)
	{
		pair<int, int> tmp = Q.top();
		Q.pop();
		tmp.first += get<0>(block[i]);
		auto pos = blockTogoal.find(make_pair(get<1>(block[i]), get<2>(block[i])));
		if (pos == blockTogoal.end())
		{
			blockTogoal.emplace(make_pair(get<1>(block[i]), get<2>(block[i])), tmp.second);
		}
		Q.push(tmp);
	}

	while (!Q.empty())
	{
		printf("%d %d\n", Q.top().first, Q.top().second);
		Q.pop();
	}

	ofstream outFile(RDF + "-partitionTriples.txt");
	ofstream subjectOutFile(RDF + "-partition-loc-subject.txt");
	for (auto it = queryTriplets.begin(); it != queryTriplets.end(); it++)
	{
		set<int> subject;
		int partitionNum = -1;
		auto pos = blockTogoal.find(it->first);
		if (pos != blockTogoal.end())
			partitionNum = pos->second;
		if (partitionNum == -1)
		{
			cout << "error:" << it->first.first << " " << it->first.second << endl;
			continue;
		}

		for (int i = 0; i < it->second.size(); i++)
		{
			outFile << IDToEntity[get<0>(it->second[i])] << "\t" << IDToPredicate[get<1>(it->second[i])] << "\t" << IDToEntity[get<2>(it->second[i])] << ".\t" << partitionNum << "\n";
			subject.insert(get<0>(it->second[i]));
		}
		for (set<int>::iterator it = subject.begin(); it != subject.end(); it++)
		{
			subjectOutFile << IDToEntity[*it] << "\t" << partitionNum << "\t" << pos->first.second << endl;
		}
	}
	cout << endl;

	outFile.close();
}