#include "AdverseSelection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     
#include <numeric>
#include <list>

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

//forward declaration
vector<string> AdverseSelection::allStocks;
std::vector <EndPWAPBookmark> EndPWAPBookmark::allBookmarks;
std::vector < std::pair< long, double > > EndPWAPBookmark::cumulativeVolume; 

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

float getNumFromStr(const char* str, int begin, int increment){
	stringstream ss;
	for (int i=begin; i<begin+increment; ++i){
		ss<<str[i];
	}
	return atoi(ss.str().c_str());
}

bool exegyCompare(const ExegyRow* first, const ExegyRow* second){
	int hour1 = getNumFromStr(first->time.c_str(), 0, 2);
	int minute1 = getNumFromStr(first->time.c_str(), 3, 2);
	int second1 = getNumFromStr(first->time.c_str(), 6, 2);
	int rest1 = getNumFromStr(first->time.c_str(), 9, 9);

	int hour2 = getNumFromStr(second->time.c_str(), 0, 2);
	int minute2 = getNumFromStr(second->time.c_str(), 3, 2);
	int second2 = getNumFromStr(second->time.c_str(), 6, 2);
	int rest2 = getNumFromStr(second->time.c_str(), 9, 9);

	if (hour1 == hour2 && minute1==minute2 && second1==second2){return rest1 < rest2;}
	else if (hour1 == hour2 && minute1==minute2){ return second1 < second2; }
	else if (hour1==hour2){return minute1 < minute2; }
	else{return hour1 < hour2;}
}

bool AdverseSelection::isValidExeyRow(const ExegyRawData& row){
	int hour = getNumFromStr(row.time, 0, 2) - 5;
	int minutes = getNumFromStr(row.time, 3, 2);

	if ((hour < 9) || (hour == 9 && minutes < 30) || (hour >= 16)) {
		return false;
	}
	else if((row.quals==32 || row.quals==102)){
		return false;
	}
	return true;
}

void pushOrAddPush(map<char, vector<ExegyRow*> > &m, ExegyRow* row, char key){
	map<char, vector<ExegyRow*> >::iterator it = m.find(key);
	if (it != m.end()){
		it->second.push_back(row);
	}else{
		vector<ExegyRow*> vec;
		vec.push_back(row);
		m[key] = vec;
	}
}

void AdverseSelection::parseHdf5Source(bool testing)
{
	H5File file(hdf5Source, H5F_ACC_RDONLY);
	DataSet dataset = file.openDataSet("/ticks/" + ticker);
	
	size_t size = dataset.getInMemDataSize();

	//int len = size / sizeof(dataset.getCompType());
	int len = size / 144;
	if (testing){
		len = 100; //take only 100 entries for testing purposes
	}
	ExegyRawData *s = (ExegyRawData*) malloc(size);
	dataset.read(s, getCompType());
	file.close();
	
	for (int i=0;i<len; ++i){
		if (isValidExeyRow(s[i]) || testing){
			ExegyRow *row = new ExegyRow(s[i]);
			tickData.push_back(row);
			if (row->type == 'T'){
				trades.push_back(row);
			}
		}
	}
	computeBuySellNOrderType(true); //assign classifications for all tick data
	if (trades.size() >= 1){
		cout<<"Total daily volume: "<<trades.at(trades.size()-1)->volume<<endl; 
		if (!testing){ //When testing, we'll test this separately
			trimTradeSize(trades.at(trades.size()-1)->volume);
		}
		trades.clear();
		for (int i=0; i<tickData.size(); ++i){
			ExegyRow* row = tickData.at(i);
			if (row->type == 'T'){
				trades.push_back(row);
			}
		}
	}
}

bool compare(const EndPWAPBookmark & first, const EndPWAPBookmark & second) {
  if (first.finalShareVolume < second.finalShareVolume) {
    return true;
  } else if (first.finalShareVolume > second.finalShareVolume) {
    return false;
  } else {
    return first.trade_idx < second.trade_idx;
  }
}

void AdverseSelection::calculatePWPPrice(std::vector<float> participationRates){
	long shareVolume = 0;
	double priceVolume = 0.0;
	for (std::vector< ExegyRow* >::const_iterator it = trades.begin(); it != trades.end(); ++it) {
	  shareVolume += (*it)->size;
	  priceVolume += (*it)->price * (*it)->size;
	  EndPWAPBookmark::cumulativeVolume.push_back( std::pair< long, double >(shareVolume, priceVolume) );

	  for (std::vector <float>::const_iterator it2 = participationRates.begin(); it2 != participationRates.end(); ++it2 ) {
		EndPWAPBookmark k;
		k.finalShareVolume = static_cast<long> ( shareVolume + (*it)->size / *it2 );
		k.trade_idx = it - trades.begin();
		k.ticker = ticker;
		k.participationRate = *it2;
		EndPWAPBookmark::allBookmarks.push_back(k);
	  }
	}
	std::sort(EndPWAPBookmark::allBookmarks.begin(), EndPWAPBookmark::allBookmarks.end(), compare);
}

void AdverseSelection::calcAdverseSelection(){
	std::vector < std::pair< long, double > >::const_iterator itv = EndPWAPBookmark::cumulativeVolume.begin();

	for (std::vector < EndPWAPBookmark >::iterator itb = EndPWAPBookmark::allBookmarks.begin(); 
			itb != EndPWAPBookmark::allBookmarks.end(); ++itb) {
	  while (itv != EndPWAPBookmark::cumulativeVolume.end() && itv->first < itb->finalShareVolume) {
		++itv;
	  }
	  if (itv == EndPWAPBookmark::cumulativeVolume.end()) {
		  EndPWAPBookmark::allBookmarks.erase(itb, EndPWAPBookmark::allBookmarks.end());
		  break;
	  }

	  std::pair <long, double> &cv = EndPWAPBookmark::cumulativeVolume[itb->trade_idx];
	  const ExegyRow* tr = trades[itb->trade_idx];
	  double pwap_price = itv->first > cv.first ? (itv->second - cv.second) / (itv->first - cv.first) : tr->price;
	  double adverseSelectionDollars = tr->buy_sell * tr->size * (tr->price - pwap_price); // classification should be {-1, 0, 1}
	  
	  itb->advSelDollarBars = adverseSelectionDollars;
	  itb->exchange = tr->exchange;
	}
}

AdverseSelection::AdverseSelection(string hdf5, string t, bool trimForTest){
	cout<<"Processing ticker: "<<t<<endl;
	hdf5Source = H5std_string(hdf5);
	ticker = t;
	parseHdf5Source(trimForTest);
	cout<<"Done Parsing"<<endl;
}

CLASSIFICATION AdverseSelection::tickTest(int i){
	if (i<=0){
		return NOT_CLASS; // Base case, shouldn't get triggered
	}
	else if (trades.at(i-1) > trades.at(i)){ return BUY;}
	else if (trades.at(i-1) < trades.at(i)){ return SELL;}
	else { return tickTest(i-1); }
}

void AdverseSelection::computeBuySellNOrderType(bool useLeeReady){
	int j=0;
	float quote_mid = 0.0;
	float ask = 0.0;
	float bid = 0.0;
	
	for (int i=0; i<tickData.size(); ++i){
		char type = tickData.at(i)->type;
		if (type == 'Q'){ 
			ask = tickData.at(i)->ask;
			bid = tickData.at(i)->bid;
			quote_mid = (ask + bid)/2; 
			continue;
		}
		else{	
			float price = tickData.at(i)->price;		
			if (price < quote_mid){
				tickData.at(i)->buy_sell = BUY;
			}else if (price > quote_mid){
				tickData.at(i)->buy_sell = SELL;
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

void AdverseSelection::trimTradeSize(long totalDailyVol){
	for (int i=0; i<tickData.size(); ++i){
		//For now, cap at a maximum of 0.1% daily volume, or 1000
		tickData.at(i)->size = min(tickData.at(i)->size, max(static_cast<long>(totalDailyVol*0.001), long(1000))); 
	}
}

void AdverseSelection::populateStockLists(string hdf5){
	H5File file(hdf5, H5F_ACC_RDONLY);
	H5Giterate(file.getId(), "/ticks", NULL, file_info, NULL);
}

AdverseSelection::~AdverseSelection(){
	std::for_each (tickData.begin(), tickData.end(), Deleter());
    tickData.clear();
}

void AdverseSelection::writeBasicStats(string name){
  ofstream myfile;
  cout<<"Outputting to file: "<<name<<endl;
  if (myfile.is_open()){ myfile.close(); }
  myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
  if (myfile.fail()) {
        cerr << "open failure: " << strerror(errno) << '\n';
  }
  for (std::map<char, int>::const_iterator it = tradeCountMap.begin(); it != tradeCountMap.end(); ++it){
	  char key = it->first;
	  myfile << ticker << "," << key<< "," << tradeCountMap[key] <<","<< totalShareMap[key] <<"," << totalPriceVolMap[key] <<"\n";
  } 
  myfile.close();
}

void AdverseSelection::writeAdvSelStats(string name){
  ofstream myfile;
  cout<<"Outputting to file: "<<name<<endl;
  if (myfile.is_open()){ myfile.close(); }
  myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
  if (myfile.fail()) {
        cerr << "open failure: " << strerror(errno) << '\n';
  }
  for (std::map<string, double>::const_iterator it = advSelMap.begin(); it != advSelMap.end(); ++it){
	  string key = it->first;
	  myfile << ticker << "," << key << "," << advSelMap[key] << "\n";
  } 
  myfile.close();
}

template<class T, class L> void pushOrAddPush(map<L, T> &m, L key, T vol){
	map<L, T>::iterator it = m.find(key);
	if (it != m.end()){
		it->second += vol;
	}else{
		m[key] = 0;
	}
}

void AdverseSelection::aggregTickersAndOutputBasicStat(string input, string output){
	ifstream myfile(input);
	string l;
	if (myfile.is_open()){
		while( getline(myfile, l)){
			string m;
			int i=0;
			char key;
			istringstream line(l);
			while(getline(line, m, ',')){
				switch(i){
					case 1:key = m.at(0); break;
					case 2:pushOrAddPush(tradeCountMap, key, atoi(m.c_str())); break;
					case 3:pushOrAddPush(totalShareMap, key, atoi(m.c_str())); break;
					case 4:pushOrAddPush(totalPriceVolMap, key, atof(m.c_str())); break;
				}
				++i;
			}
		}
		myfile.close();
	}
	ticker = "AGGREGATE";
	writeBasicStats(output);
}

void AdverseSelection::aggregTickersAndOutputAdvSelStat(string input, string output){
	ifstream myfile(input);
	string l;
	if (myfile.is_open()){
		while( getline(myfile, l)){
			string m;
			int i=0;
			string key;
			istringstream line(l);
			while(getline(line, m, ',')){
				switch(i){
					case 1:key = m;break;
					case 2:pushOrAddPush(advSelMap, key, atof(m.c_str())); break;
				}
				++i;
			}
		}
	}
	ticker = "AGGREGATE";
	writeAdvSelStats(output);
}

void AdverseSelection::aggregateUseTickerEx(){
	for (int i=0; i<trades.size(); ++i){
		pushOrAddPush(tradeCountMap, trades[i]->exchange, 1);
		pushOrAddPush(totalShareMap, trades[i]->exchange, static_cast<int>(trades[i]->size));
		pushOrAddPush(totalPriceVolMap, trades[i]->exchange, trades[i]->price * (trades[i]->size + 0.0)); //implicit
	}
	for (int i=0; i<EndPWAPBookmark::allBookmarks.size(); ++i){
		stringstream ss;
		ss << EndPWAPBookmark::allBookmarks[i].exchange << EndPWAPBookmark::allBookmarks[i].participationRate;
		pushOrAddPush(advSelMap, ss.str(), EndPWAPBookmark::allBookmarks[i].advSelDollarBars);
	}
}

void AdverseSelection::calculateAndOutput(string basicStatOutputFile, string advStatOutputFile){
	vector<float> partRates;
	partRates.push_back(0.3f);
	partRates.push_back(0.1f);
	partRates.push_back(0.03f);
	partRates.push_back(0.01f);
	partRates.push_back(0.003f);
	partRates.push_back(0.001f);
	
	EndPWAPBookmark::allBookmarks.clear();
	EndPWAPBookmark::cumulativeVolume.clear();
	calculatePWPPrice(partRates);
	calcAdverseSelection();
	aggregateUseTickerEx();
	writeBasicStats(basicStatOutputFile);
	writeAdvSelStats(advStatOutputFile);
}



