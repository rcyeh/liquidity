#pragma once
#include <string>
#include "hdf5.h"
#include "H5Cpp.h"
#include <vector>

using namespace std;


typedef struct exegyRow {
	float ask;
	char ask_exchange; //char ask_exchange[2];
	long ask_size;
	float bid;
	char bid_exchange[2];
	long bid_size;
	char exchange[2];
	long exchange_time;
	unsigned char instrument_status;
	long latency; 
	unsigned int line;
	unsigned char market_status;
	float prev_close;
	float price;
	unsigned int quals;
	char refresh; //char refresh[2];
	long seq_no;
	long size;
	char sub_market; //char sub_market[2];
	string symbol; //char symbol[9];
	unsigned char thru_exempt;
	string time; //char time[19];
	char type; //char type[2];
	long volume;
} exegyRow;

enum CLASSIFICATION{BUY=1, SELL=-1, NOT_CLASS=0};

class AdverseSelection
{
private: 
	//H5std_string hdf5Source;
	CLASSIFICATION tickTest(int i);
	vector<exegyRow*> tickData;
	vector<exegyRow*> trades;
	exegyRow* createRow(string line);
public:
	vector<float> calcPartWeightAvg(double percent);
	vector<CLASSIFICATION> computeClassification(bool useLeeReady=false);
	AdverseSelection(string source);
	void parseHdf5Source();
	~AdverseSelection();
};

