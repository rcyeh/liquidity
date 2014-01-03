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
	string ticker;
	CLASSIFICATION tickTest(int i);
	vector<ExegyRow*> tickData;
	vector<ExegyRow*> trades;
	map<char, vector<ExegyRow*> > exchange_trades_m;
	//map<char, vector<CLASSIFICATION> > exchange_classed_m;
	ExegyRow* createRow(string line);
	void parseCsv(string fn);
	void trimTradeSize(long totalDailyVol);
	vector<long> getCumVolPerEx(char exchanges[]);
	vector<long> getTotalSumPerEx(char exchanges[]);
	vector<ExegyRow*> getRowsForExchanges(char exchanges[]);
	void writeToFile(vector<float> advSelection, string name);
public:
	static vector<string> allStocks;
	vector<float> calcPartWeightAvg(float percent, char exchanges[]);
	vector<float> calcAdverseSelection(float percent, char exchanges[]);
	void computeClassification(bool useLeeReady=false);
	AdverseSelection(string csvSource);
	AdverseSelection(string hdf5Source, string ticker);
	void parseHdf5Source();
	void outputAdvSelToFile();
	friend herr_t file_info(hid_t loc_id, const char *name, void *opdata);
	~AdverseSelection();
};

