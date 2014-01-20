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
private: 
	// member functions
	void trimTradeSize(long totalDailyVol);
	vector<long> getCumVolPerEx(char exchanges[], const vector<int> *noCalcIndiciess=NULL);
	long getLastCumVol(char exchanges[], const vector<int> *noCalcIndiciess=NULL);
	vector<long> getTotalSumPerEx(char exchanges[]);
	vector<ExegyRow*> getRowsForExchanges(char exchanges[], bool useMktOnly=false);
	void writeToFile(vector<ExegyRow*> rows, string name);
	void writeToFile(float advSelection, string name);
	void writeToFile(Ticker ticker, string name);
	bool isValidExeyRow(const ExegyRawData& row);

	// member variables
	H5std_string hdf5Source;
	string ticker;
	CLASSIFICATION tickTest(int i);
	vector<ExegyRow*> tickData;
	vector<ExegyRow*> trades;
	map<char, vector<ExegyRow*> > exchange_trades_m;
	map<char, vector<ExegyRow*> > exchange_trades_m_mkt;
	map<char, bool> exHasTrades;
	bool hasTrades(char exchanges[]);
	int threadIdentifier;
public:
	static vector<string> allStocks;
	static void populateStockLists(string hdf5);
	vector<float> calcPartWeightAvg(float percent, char exchanges[], vector<int> *noCalcIndicies, bool useMKTOnly=false);
	vector<ExegyRow*> calcAdverseSelection(float percent, char exchanges[], bool priceWeighted=true);
	float calcWeightedAdverseSelection(float percent, char exchanges[], bool useMKTOnly=false);
	void computeBuySellNOrderType(bool useLeeReady=false);
	TRADE_TYPE computeOrderTypes(double bid, double ask, double tradePrice);
	AdverseSelection(string hdf5Source, string ticker, bool testing=false);
	void parseHdf5Source(bool trimForTest=false);
	void outputAdvSelToFile(bool outputAvg=true, string outputFile = string("AllTradesInfo.csv"), bool useMKTOnly=false);
	friend herr_t file_info(hid_t loc_id, const char *name, void *opdata);
	~AdverseSelection();
};

