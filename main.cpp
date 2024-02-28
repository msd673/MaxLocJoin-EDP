#include "graph.cpp"
#include <ctime>
clock_t st, ed;
double endtime;
int main(int argc, char *argv[])
{
	//输入格式： 数据集文件名 输出文件前缀名 数据集中三元组之间的分隔符以及分区个数
	string txt_name = argv[1];
	string name = argv[2];
	string sign = (string(argv[3]) == "1") ? " " : "\t";
	int part = atoi(argv[4]);

	st = clock();
		graph *test = new graph();
		test->init();
		test->RDF = name;
		test->part = part;

		test->loadGraph(txt_name, sign);

		int predNum = test->getPreNum();
		long long entityNum = test->getEntityNum();
		
		cout << "predNum: " << predNum << endl << "entityNum: " << entityNum << endl;
		cout << "main:----------------" << endl;
		
		test->unionEdgeGreed();

	ed = clock();

	endtime = (double)(ed - st) / CLOCKS_PER_SEC;
	cout << "partition : " << endtime << " s" << endl;

	delete test;

	return 0;
}
// g++ -o mlj-edp main.cpp -std=c++11
// ./mlj-edp latest-truthy-202209-remove-tab.nt latest-truthy-202209-remove-tab 2 8
// ./mlj-edp dbpedia34.nt dbpedia2014_100M 1 8
// ./mlj-edp dbpedia3.8.nt dbpedia_500M 1 8
// nohup ./mlj-edp /home/msd/wikidata/data/latest-truthy-202209-remove.nt wikidata_new 1 8 >wikidata.log 2>&1 &