#include "AdverseSelection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     
#include <numeric>

using namespace std;
using namespace H5;

#define DEFAULT_SPREAD 999.0
#define NUM_PARTS 6

struct Deleter
{
    template <typename T>
    void operator () (T *obj) const
    {
        delete obj;
    }
};

vector<string> AdverseSelection::allStocks;

herr_t file_info(hid_t loc_id, const char *name, void *opdata)
{
    H5G_stat_t statbuf;
    /*
     * Get type of the object and display its name and type.
     * The name of the object is passed to this function by 
     * the Library. Some magic :-)
     */
    H5Gget_objinfo(loc_id, name, false, &statbuf);
	if (statbuf.type == H5G_DATASET){
		AdverseSelection::allStocks.push_back(string(name));
	}
	return 0;
 };

CompType getCompType(){
	CompType mtype3( sizeof(ExegyRawData) );

	mtype3.insertMember("ask", HOFFSET(ExegyRawData, ask), PredType::NATIVE_FLOAT);
	mtype3.insertMember("ask_exchange", HOFFSET(ExegyRawData, ask_exchange), StrType(0,2));
	mtype3.insertMember("ask_size", HOFFSET(ExegyRawData, ask_size), PredType::NATIVE_LONG);
	mtype3.insertMember("bid", HOFFSET(ExegyRawData, bid), PredType::NATIVE_FLOAT);
	mtype3.insertMember("bid_exchange", HOFFSET(ExegyRawData, bid_exchange), StrType(0,2));
	mtype3.insertMember("bid_size", HOFFSET(ExegyRawData, bid_size), PredType::NATIVE_LONG);
	mtype3.insertMember("exchange", HOFFSET(ExegyRawData, exchange), StrType(0,2));
	mtype3.insertMember("exchange_time", HOFFSET(ExegyRawData, exchange_time), PredType::NATIVE_LONG);
	mtype3.insertMember("instrument_status", HOFFSET(ExegyRawData, instrument_status), PredType::NATIVE_CHAR);
	mtype3.insertMember("latency", HOFFSET(ExegyRawData, latency), PredType::NATIVE_LONG);
	mtype3.insertMember("line", HOFFSET(ExegyRawData, line), PredType::NATIVE_LONG);
	mtype3.insertMember("market_status", HOFFSET(ExegyRawData, market_status), PredType::NATIVE_LONG);
	mtype3.insertMember("prev_close", HOFFSET(ExegyRawData, prev_close), PredType::NATIVE_FLOAT);
	mtype3.insertMember("price", HOFFSET(ExegyRawData, price), PredType::NATIVE_FLOAT);
	mtype3.insertMember("quals", HOFFSET(ExegyRawData, quals), PredType::NATIVE_LONG);
	mtype3.insertMember("refresh", HOFFSET(ExegyRawData, refresh), StrType(0,2));
	mtype3.insertMember("seq_no", HOFFSET(ExegyRawData, seq_no), PredType::NATIVE_LONG);
	mtype3.insertMember("size", HOFFSET(ExegyRawData, size), PredType::NATIVE_LONG);
	mtype3.insertMember("sub_market", HOFFSET(ExegyRawData, sub_market), StrType(0,2));
	mtype3.insertMember("symbol", HOFFSET(ExegyRawData, symbol), StrType(0,9));
	hsize_t    array_dim[] = {1};
	mtype3.insertMember("thru_exempt", HOFFSET(ExegyRawData, thru_exempt), H5Tarray_create(H5T_NATIVE_CHAR, 1, array_dim));
	mtype3.insertMember("time", HOFFSET(ExegyRawData, time), StrType(0,19));
	mtype3.insertMember("type", HOFFSET(ExegyRawData, type), StrType(0,2));
	mtype3.insertMember("volume", HOFFSET(ExegyRawData, volume), PredType::NATIVE_LONG);

	return mtype3;
}

void AdverseSelection::parseHdf5Source(bool trimForTest)
{
	//AdverseSelection::mtx.lock();
	//cout<<"Locked"<<endl;

	H5File file(hdf5Source, H5F_ACC_RDONLY);
	DataSet dataset = file.openDataSet("/ticks/" + ticker);
	
	size_t size = dataset.getInMemDataSize();

	//int len = size / sizeof(dataset.getCompType());
	int len = size / 144;
	if (trimForTest){
		len = 100; //take only 100 entries for testing purposes
	}
	ExegyRawData *s = (ExegyRawData*) malloc(size);
	dataset.read(s, getCompType());
	file.close();
	
	//AdverseSelection::mtx.unlock();
	//cout<<"Unlocked"<<endl;

	for (int i=0;i<len; ++i){
		ExegyRow *row = new ExegyRow(s[i]);
		tickData.push_back(row);
		if (row->type == 'T'){
			trades.push_back(row);
		}
	}
	computeClassification(true); //assign classifications for all tick data
	if (trades.size() >= 1){
		cout<<"Total daily volume: "<<trades.at(trades.size()-1)->volume<<endl; 
		if (!trimForTest){ //When testing, we'll test this separately
			trimTradeSize(trades.at(trades.size()-1)->volume);
		}
		trades.clear();
		for (int i=0; i<tickData.size(); ++i){
			ExegyRow* row = tickData.at(i);
			if (row->type == 'T'){
				trades.push_back(row);
				char ex = row->exchange;
				exHasTrades[ex] = true;
				map<char, vector<ExegyRow*> >::iterator it = exchange_trades_m.find(ex);
				if (it != exchange_trades_m.end()){
					it->second.push_back(row);
				}else{
					vector<ExegyRow*> vec;
					vec.push_back(row);
					exchange_trades_m[ex] = vec;
				}
			}
		}
	}
}

bool isValidQual(int qual){
	if(qual!=32 && qual!=59){
		return true;	
	}
	return false;
}

ExegyRow* AdverseSelection::createRow(string line){
	ExegyRow* row = new ExegyRow;
	string csvItem;
	istringstream myline(line);
    for(int i=0; i<25; ++i){
		getline(myline, csvItem, ',');
		switch(i){
			//case 0: row->row_num = atoi(&csvItem.at(1)); break;
			case 1: row->ask = atof(csvItem.c_str()); break;
			case 3: row->ask_size = atoi(csvItem.c_str()); break;
			case 4: row->bid = atof(csvItem.c_str()); break;
			case 6: row->bid_size = atoi(csvItem.c_str()); break;
			case 7: row->exchange = csvItem.at(1); break;
			case 8: 
				row->exchange_time = stoll(csvItem.c_str()); 
				if (row->exchange_time < 1366718400000 + 9000000 ||
					row->exchange_time > 1366718400000 + 32400000 ){ //outside market hours
					return NULL;
				}
				break;
			case 14: row->price = atof(csvItem.c_str()); break;
			case 15: row->quals = atoi(csvItem.c_str()); 
				if(!isValidQual(row->quals)){return NULL;}
				break;
			case 18: row->size = atoi(csvItem.c_str()); break;
			case 20: row->symbol = csvItem; break;
			case 23: row->type = csvItem.at(1); break;
			case 24: row->volume = atoi(csvItem.c_str()); break;
		}
	}
	return row;
}

AdverseSelection::AdverseSelection(string hdf5, string t, bool trimForTest){
	cout<<"Processing ticker: "<<t<<endl;
	threadIdentifier = 1;
	hdf5Source = H5std_string(hdf5);
	ticker = t;
	parseHdf5Source(trimForTest);
	cout<<"Done Parsing"<<endl;
}

AdverseSelection::AdverseSelection(string hdf5, string t, int tI, bool trimForTest){
	cout<<"Processing ticker: "<<t<<endl;
	threadIdentifier = tI;
	hdf5Source = H5std_string(hdf5);
	ticker = t;
	parseHdf5Source(trimForTest);
	cout<<"Done Parsing"<<endl;
}

CLASSIFICATION AdverseSelection::tickTest(int i){
	if (i<=0){
		return NOT_CLASS; // Base case, shouldn't get triggered
	}
	else if (trades.at(i-1) > trades.at(i)){ return SELL;}
	else if (trades.at(i-1) < trades.at(i)){ return BUY;}
	else { return tickTest(i-1); }
}

void AdverseSelection::computeClassification(bool useLeeReady){
	int j=0;
	float quote_mid = 0.0;

	for (int i=0; i<tickData.size(); ++i){
		char type = tickData.at(i)->type;
		if (type == 'Q'){ 
			quote_mid = (tickData.at(i)->ask + tickData.at(i)->bid)/2; 
			continue;
		}
		else{	
			float price = tickData.at(i)->price;
			//char ex = tickData.at(i)->exchange;
			if (price < quote_mid){
				tickData.at(i)->buy_sell = SELL;
			}else if (price > quote_mid){
				tickData.at(i)->buy_sell = BUY;
			}else{
				if (useLeeReady){
					tickData.at(i)->buy_sell = tickTest(j);
				}else{
					tickData.at(i)->buy_sell = NOT_CLASS;
				}
			}
			++j;
		}
	}
}

//"excludeLastXTrades" is used in calculating cumulative volume on an exchange 
// when only a fraction of the trades have a valid pwp value and we want a volume weighted pwp 
// (i.e. the last few trades will not have pwp, because there won't be enough volume following them to calculate pwp)
vector<long> AdverseSelection::getCumVolPerEx(char exchanges[], const vector<int> *noCalcIndicies){ 
	vector<long> cumVols;
	vector<ExegyRow*> trades = getRowsForExchanges(exchanges);
	long vol = 0;
	for (int i=0; i<trades.size(); ++i){
		// we don't include volume for this trade, since the the pwp calculation for this trade is not performed
		// due to insufficient trailing volume
		if (noCalcIndicies != NULL && 
			find(noCalcIndicies->begin(), noCalcIndicies->end(), i) != noCalcIndicies->end()){ 
			continue;
		}
		vol += trades.at(i)->size;
		cumVols.push_back(vol);
	}
	return cumVols;
}

long AdverseSelection::getLastCumVol(char exchanges[], const vector<int> *noCalcIndiciess){
	vector<long> cumVols = getCumVolPerEx(exchanges, noCalcIndiciess);
	return cumVols.at(cumVols.size()-1);
}

void AdverseSelection::trimTradeSize(long totalDailyVol){
	for (int i=0; i<tickData.size(); ++i){
		tickData.at(i)->size = min(tickData.at(i)->size, static_cast<long>(totalDailyVol*0.001)); //For now, cap at a maximum of 0.1% daily volume
	}
}

void AdverseSelection::populateStockLists(string hdf5){
	H5File file(hdf5, H5F_ACC_RDONLY);
	H5Giterate(file.getId(), "/ticks", NULL, file_info, NULL);
}

vector<long> AdverseSelection::getTotalSumPerEx(char exchanges[]){
	vector<long> totalSum;
	vector<ExegyRow*> trades = getRowsForExchanges(exchanges);
	long s = 0.0;
	for (int i=0; i<trades.size(); ++i){
		s += trades.at(i)->size * trades.at(i)->price;
		totalSum.push_back(s);
	}
	return totalSum;
}

bool exegyCompare(ExegyRow* first, ExegyRow* second){
	return (first->exchange_time < second->exchange_time);
}

// Get all trades that belong to exchanges
vector<ExegyRow*> AdverseSelection::getRowsForExchanges(char exchanges[]){
	vector<ExegyRow*> trades;
	for (int i=0; i<strlen(exchanges); ++i){
		vector<ExegyRow*> t = exchange_trades_m[exchanges[i]];
		trades.insert(trades.end(),t.begin(),t.end());
	}
	std::sort (trades.begin(), trades.end(), exegyCompare);
	return trades;
}

//O(nlogn)
vector<float> AdverseSelection::calcPartWeightAvg(float percent, char exchanges[], vector<int> *noCalcIndicies){
	vector<ExegyRow*> trades = getRowsForExchanges(exchanges);
	vector<float> partWeightedPrices;

	vector<long> totalSum = getTotalSumPerEx(exchanges);
	vector<long> cumVols = getCumVolPerEx(exchanges);

	float pwp = 0.0;
	for (int i=0; i<trades.size(); ++i){
		if (trades.at(i)->size == 0){
			continue;
		}
		long partVol = trades.at(i)->size/percent; // Calculate next X volumes to use	

		long nV = partVol + cumVols.at(i);
		std::vector<long>::iterator low = lower_bound (cumVols.begin(), cumVols.end(), nV); // Search takes O(log(n)) time
		int pos = low-cumVols.begin();
		
		if (pos >= trades.size() && noCalcIndicies != NULL){ // We record it, the pwp of this index will not be calculated
			noCalcIndicies->push_back(i);
		}
		
		for (int j=pos; j<trades.size(); ++j){ // The second for loop is finished in O(1) time
			long cumVol = (cumVols.at(j) - cumVols.at(i));
			if (cumVol >= partVol){ 
				if (cumVol > partVol){
					long diff = cumVol - partVol;
					pwp = (totalSum.at(j) - totalSum.at(i) - (trades.at(j)->price*diff))/(cumVols.at(j) - cumVols.at(i) - diff + 0.0);
				}else{
					pwp = (totalSum.at(j) - totalSum.at(i))/(cumVols.at(j) - cumVols.at(i) + 0.0);
				}
				partWeightedPrices.push_back(pwp);
				break;
			}
		}
	}
	return partWeightedPrices;
}

bool AdverseSelection::hasTrades(char exchanges[]){
	bool hT = false;
	for (int i=0;i<strlen(exchanges); ++i){
		hT = hT || exHasTrades[exchanges[i]];
	}
	return hT;
}

vector<ExegyRow*> AdverseSelection::calcAdverseSelection(float percent, char exchanges[], bool priceWeighted){	
	vector<ExegyRow*> egrs;
	if (hasTrades(exchanges)){
		vector<float> pwp = calcPartWeightAvg(percent, exchanges, NULL);
		egrs = getRowsForExchanges(exchanges);

		for (int i=0; i<pwp.size(); ++i){
			float ads = egrs.at(i)->buy_sell * (egrs.at(i)->price - pwp.at(i));
			if (priceWeighted){
				float avgPrice = (egrs.at(i)->price + pwp.at(i))/2;
				ads /= avgPrice;
			}
			egrs.at(i)->advSPrices.push_back(ads);
		}
	}
	return egrs;
}

float AdverseSelection::calcWeightedAdverseSelection(float percent, char exchanges[], bool priceWeighted){
	float advSelection = DEFAULT_SPREAD; //default value, if this ticker does not occur at the exchanges
	if (hasTrades(exchanges)){
		vector<int> *noCalcIndicies = new vector<int>();
		vector<float> pwp = calcPartWeightAvg(percent, exchanges, noCalcIndicies);
		if (pwp.size() > 0){
			advSelection = 0.0;
			vector<ExegyRow*> egrs = getRowsForExchanges(exchanges);
			long totalVol = getLastCumVol(exchanges, noCalcIndicies);

			for (int i=0; i<pwp.size(); ++i){
				float avgPrice = 1.0;
				if (priceWeighted){
					avgPrice = (egrs.at(i)->price + pwp.at(i))/2;
				}
				float advSelContrib = (egrs.at(i)->size + 0.0) / totalVol * (egrs.at(i)->buy_sell * (egrs.at(i)->price - pwp.at(i))) / avgPrice;
				advSelection += advSelContrib;
			}			
		}
		delete(noCalcIndicies);
	}
	return advSelection;
}

AdverseSelection::~AdverseSelection(){
	std::for_each (tickData.begin(), tickData.end(), Deleter());
    tickData.clear();
}

void AdverseSelection::writeToFile(Ticker ticker, string name){
	if (ticker.pwps.size() == NUM_PARTS){
		 ofstream myfile;
		 cout<<"Outputting to file: "<<name<<endl;
		 if (myfile.is_open()){ myfile.close(); }
		 myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
		 if (myfile.fail()) {
			cerr << "open failure: " << strerror(errno) << '\n';
		 }
		 myfile << ticker.getData() <<"\n";
		 myfile.close();
	}
}

void AdverseSelection::writeToFile(float advSelection, string name){
  if (advSelection != DEFAULT_SPREAD){
	  ofstream myfile;
	  cout<<"Outputting to file: "<<name<<endl;
	  if (myfile.is_open()){ myfile.close(); }
	  myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
	  if (myfile.fail()) {
			cerr << "open failure: " << strerror(errno) << '\n';
	  }
	  myfile << ticker << "," << advSelection<<"\n";
	  myfile.close();
  }
}

void AdverseSelection::writeToFile(vector<ExegyRow*> rows, string name){
  ofstream myfile;
  cout<<"Outputting to file: "<<name<<endl;
  if (myfile.is_open()){ myfile.close(); }
  myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
  if (myfile.fail()) {
        cerr << "open failure: " << strerror(errno) << '\n';
  }
  for (int i=0; i<rows.size(); ++i){
	  myfile << ticker << "," << rows.at(i)->getTradeData() <<"\n";
  } 
  myfile.close();
}

void AdverseSelection::outputAdvSelToFile(bool outputAvg, string outputFile){
	char exchanges[17] = {'A','B','C','D','E','I','J','K','M','N','P','Q','W','X','Y','Z','\0'};

	vector<float> partRates;
	partRates.push_back(0.3f);
	partRates.push_back(0.1f);
	partRates.push_back(0.03f);
	partRates.push_back(0.01f);
	partRates.push_back(0.003f);
	partRates.push_back(0.001f);
	
	for (int j=0; j<strlen(exchanges); ++j){
		char c = exchanges[j];
		char cA[2]; 
		cA[0] = c; 
		cA[1] = '\0';
		Ticker ticker(ticker, c);
		for (int i=0; i<partRates.size(); ++i){	
			std::stringstream ss;
			if (hasTrades(cA)){
				ticker.price = trades.at(trades.size()-1)->price;
				// Depending on if we want all raw data or just one weighted adverse selection per stock
				if (outputAvg){
					//ss << c <<partRates.at(i)*1000<<".csv";
					ss << "Output_" << outputFile;
					string fileName = ss.str();
					float advSelection = calcWeightedAdverseSelection(partRates.at(i), cA);
					ticker.pwps.push_back(advSelection);
					if (advSelection == DEFAULT_SPREAD){
						break;
					}
					if (i == partRates.size() - 1){
						writeToFile(ticker, fileName);
					}
				}else{
					vector<ExegyRow*> allTrades = calcAdverseSelection(partRates.at(i), cA);
					if (i == partRates.size() - 1){
						writeToFile(allTrades, outputFile);
					}
				}
			}
		}
	}
}


