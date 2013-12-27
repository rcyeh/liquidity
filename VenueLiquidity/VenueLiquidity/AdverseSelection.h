#pragma once
#include <string>
#include "hdf5.h"
#include "H5Cpp.h"
#include <vector>
#include <map>
#include "ExegyRow.h"

using namespace std;

class AdverseSelection
{
private: 
	H5std_string hdf5Source;
	CLASSIFICATION tickTest(int i);
	vector<ExegyRow*> tickData;
	vector<ExegyRow*> trades;
	map<char, vector<ExegyRow*> > exchange_trades_m;
	//map<char, vector<CLASSIFICATION> > exchange_classed_m;
	ExegyRow* createRow(string line);
	void parseCsv(string fn);
	vector<long> getCumVolPerEx(char exchanges[]);
	vector<long> getTotalSumPerEx(char exchanges[]);
	vector<ExegyRow*> getRowsForExchanges(char exchanges[]);
public:
	vector<float> calcPartWeightAvg(float percent, char exchanges[]);
	vector<float> calcAdverseSelection(float percent, char exchanges[]);
	void computeClassification(bool useLeeReady=false);
	AdverseSelection(string source);
	void parseHdf5Source();
	~AdverseSelection();
};

