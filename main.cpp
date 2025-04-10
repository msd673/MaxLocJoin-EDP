#include "graph.cpp"
#include <ctime>
clock_t st, ed;
double endtime;
int main(int argc, char *argv[])
{
	// Input format: 
	// Dataset file name 
	// Output file prefix 
	// Separator between triplets in the dataset 
	// number of partitions
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
		
		test->unionEdgeGreed();

	ed = clock();

	endtime = (double)(ed - st) / CLOCKS_PER_SEC;
	cout << "partition : " << endtime << " s" << endl;

	delete test;

	return 0;
}
// g++ -o mlj-edp main.cpp -std=c++11
// ./mlj-edp dataset-tab.nt dataset-tab 2 8
// nohup ./mlj-edp dataset.nt dataset 1 8 >dataset.log 2>&1 &
