#pragma once
#include <string>
#include "hdf5.h"
#include "H5Cpp.h"
#include <vector>
#include <map>
#include "ExegyRow.h"

using namespace std;

enum CLASSIFICATION{BUY=1, SELL=-1, NOT_CLASS=0};

class AdverseSelection
{
private: 
	H5std_string hdf5Source;
	CLASSIFICATION tickTest(int i);
	vector<ExegyRow*> tickData;
	vector<ExegyRow*> trades;
	map<char, vector<ExegyRow*> > exchange_trades_m;
	map<char, vector<CLASSIFICATION> > exchange_classed_m;
	ExegyRow* createRow(string line);
	void parseCsv(string fn);
	vector<long> getCumVolPerEx(char exchange);
	vector<long> getTotalSumPerEx(char exchange);
public:
	vector<float> calcPartWeightAvg(float percent, char exchange);
	vector<float> calcAdverseSelection(float percent, char exchange);
	void computeClassification(bool useLeeReady=false);
	AdverseSelection(string source);
	void parseHdf5Source();
	~AdverseSelection();
};

