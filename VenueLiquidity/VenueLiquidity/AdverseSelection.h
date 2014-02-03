#pragma once
#include <string>
#include "hdf5.h"
#include "H5Cpp.h"
#include <vector>
#include <map>
#include "ExegyRow.h"
#include "Ticker.h"

using namespace std;

class AdverseSelection
{
	friend void advSelectionTest(); //for testing
	//friend class ASParser; //for parsing
private: 
	// member functions
	void trimTradeSize(long totalDailyVol);	
	bool isValidExeyRow(const ExegyRawData& row);
	void aggregateUseTickerEx();
	
	// member variables
	H5std_string hdf5Source;
	string ticker;
	CLASSIFICATION tickTest(int i);
	vector<ExegyRow*> tickData;
	vector<ExegyRow*> trades;

	map<char, int> tradeCountMap;
	map<char, int> totalShareMap;
	map<char, double> totalPriceVolMap;
	map<string, double> advSelMap;
public:
	static vector<string> allStocks;
	static void populateStockLists(string hdf5);

	void calcAdverseSelection();
	void calculatePWPPrice(std::vector <float> participationRates);
	void aggregTickersAndOutputBasicStat(string input, string output);
	void aggregTickersAndOutputAdvSelStat(string input, string output);
	void writeBasicStats(string name);
	void writeAdvSelStats(string name);

	void computeBuySellNOrderType(bool useLeeReady=false);

	AdverseSelection(){};
	AdverseSelection(string hdf5Source, string ticker, bool testing=false);
	void parseHdf5Source(bool trimForTest=false);
	
	void calculateAndOutput(string basicStatOutputFile, string advStatOutputFile);
	friend herr_t file_info(hid_t loc_id, const char *name, void *opdata);
	~AdverseSelection();
};

