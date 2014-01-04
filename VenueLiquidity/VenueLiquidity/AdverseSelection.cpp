#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     

using namespace std;
using namespace H5;

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

void AdverseSelection::parseHdf5Source()
{
	conProcess = false;

	H5File file(hdf5Source, H5F_ACC_RDONLY);

	DataSet dataset = file.openDataSet("/ticks/" + ticker);
	
	size_t size = dataset.getInMemDataSize();

	//int len = size / sizeof(dataset.getCompType());
	int len = size / 144;

	ExegyRawData *s = (ExegyRawData*) malloc(size);

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
	dataset.read(s, mtype3);
	for (int i=0;i<len; ++i){
		ExegyRow *row = new ExegyRow(s[i]);
		tickData.push_back(row);
		if (row->type == 'T'){
			trades.push_back(row);
		}
	}
	computeClassification(true); //assign classifications for all tick data
	if (trades.size() >= 1){
		conProcess = true;
		cout<<"Total daily volume: "<<trades.at(trades.size()-1)->volume<<endl; 
		trimTradeSize(trades.at(trades.size()-1)->volume);
		trades.clear();
		for (int i=0; i<tickData.size(); ++i){
			ExegyRow* row = tickData.at(i);
			if (row->type == 'T'){
				trades.push_back(row);
				char ex = row->exchange;
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

void AdverseSelection::parseCsv(string fn){
	ifstream file;
	file.open(fn,ios::in);
	string line;
	while (getline(file,line)){
		ExegyRow* row = createRow(line);
		if (row != NULL){
			tickData.push_back(row);
			if (row->type == 'T'){
				trades.push_back(row);
			}
		}
    }
	file.close();
	computeClassification(true); //assign classifications for all tick data
	cout<<"Total daily volume: "<<trades.at(trades.size()-1)->volume; 
	trimTradeSize(trades.at(trades.size()-1)->volume);
	trades.clear();
	for (int i=0; i<tickData.size(); ++i){
		ExegyRow* row = tickData.at(i);
		if (row->type == 'T'){
			trades.push_back(row);
			char ex = row->exchange;
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

AdverseSelection::AdverseSelection(string source)
{
	parseCsv(source);
}

AdverseSelection::AdverseSelection(string hdf5, string t){
	cout<<"Processing ticker: "<<t<<endl;
	hdf5Source = H5std_string(hdf5);
	ticker = t;
	parseHdf5Source();
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
			char ex = tickData.at(i)->exchange;
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

vector<long> AdverseSelection::getCumVolPerEx(char exchanges[]){
	vector<long> cumVols;
	vector<ExegyRow*> trades = getRowsForExchanges(exchanges);
	long vol = 0;
	for (int i=0; i<trades.size(); ++i){
		vol += trades.at(i)->size;
		cumVols.push_back(vol);
	}
	return cumVols;
}

void AdverseSelection::trimTradeSize(long totalDailyVol){
	for (int i=0; i<tickData.size(); ++i){
		tickData.at(i)->size = min(tickData.at(i)->size, static_cast<long>(totalDailyVol*0.001)); //For now, cap at a maximum of 0.1% daily volume
	}
}

void AdverseSelection::populateStockLists(string hdf5){
	H5File file(H5std_string(hdf5), H5F_ACC_RDONLY);
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
vector<float> AdverseSelection::calcPartWeightAvg(float percent, char exchanges[]){
	vector<ExegyRow*> trades = getRowsForExchanges(exchanges);
	vector<float> partWeightedPrices;

	vector<long> totalSum = getTotalSumPerEx(exchanges);
	vector<long> cumVols = getCumVolPerEx(exchanges);

	float pwp = 0.0;
	for (int i=0; i<trades.size(); ++i){
		long partVol = trades.at(i)->size/percent; // Calculate next X volumes to use	

		long nV = partVol + cumVols.at(i);
		std::vector<long>::iterator low = lower_bound (cumVols.begin(), cumVols.end(), nV); 
		int pos = low-cumVols.begin();
		for (int j=pos; j<trades.size(); ++j){
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

vector<float> AdverseSelection::calcAdverseSelection(float percent, char exchanges[]){	
	vector<float> adverseSelection;
	if (conProcess){
		vector<float> pwp = calcPartWeightAvg(percent, exchanges);
		vector<ExegyRow*> egrs = getRowsForExchanges(exchanges);
		//vector<CLASSIFICATION> classed = exchange_classed_m[exchange];
		for (int i=0; i<pwp.size(); ++i){
			float ads = egrs.at(i)->buy_sell * (egrs.at(i)->price - pwp.at(i));
			adverseSelection.push_back(ads);
		}
	}
	return adverseSelection;
}

float AdverseSelection::calcWeightedAdverseSelection(float percent, char exchanges[]){
	float advSelection = 9999999.0; //default value, if this ticker does not occur at the exchanges
	if (conProcess){
		vector<float> pwp = calcPartWeightAvg(percent, exchanges);
		if (pwp.size() > 0){
			advSelection = 0.0;
		}
		vector<ExegyRow*> egrs = getRowsForExchanges(exchanges);
		//vector<CLASSIFICATION> classed = exchange_classed_m[exchange];
		for (int i=0; i<pwp.size(); ++i){
			advSelection += (egrs.at(i)->size + 0.0) / trades.at(trades.size()-1)->volume * (egrs.at(i)->buy_sell * (egrs.at(i)->price - pwp.at(i)));
		}
	}
	return advSelection;
}

AdverseSelection::~AdverseSelection(){
	std::for_each (tickData.begin(), tickData.end(), Deleter());
    tickData.clear();
}

void AdverseSelection::writeToFile(float advSelection, string name){
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

void AdverseSelection::writeToFile(vector<float> advSelection, string name){
  ofstream myfile;
  cout<<"Outputting to file: "<<name<<endl;
  if (myfile.is_open()){ myfile.close(); }
  myfile.open(name.c_str(),std::ofstream::out | std::ofstream::app);
  if (myfile.fail()) {
        cerr << "open failure: " << strerror(errno) << '\n';
  }
  for (int i=0; i<advSelection.size(); ++i){
	myfile << advSelection.at(i)<<"\n";
  } 
  myfile.close();
}

void AdverseSelection::outputAdvSelToFile(){
	if (!conProcess){
		return;
	}
	char exchanges[17] = {'A','B','C','D','E','I','J','K','M','N','P','Q','W','X','Y','Z','\0'};
	//char exchanges_noD[16] = {'A','B','C','E','I','J','K','M','N','P','Q','W','X','Y','Z','\0'};
	vector<float> partRates;
	partRates.push_back(0.3);
	partRates.push_back(0.1);
	partRates.push_back(0.03);
	partRates.push_back(0.01);
	partRates.push_back(0.003);
	partRates.push_back(0.001);
	
	//vector<float> pwpVec = selection.calcPartWeightAvg(0.1,exchanges);
	
	for (int i=0; i<partRates.size(); ++i){
		for (int j=0; j<strlen(exchanges); ++j){
			std::stringstream ss;
			char c = exchanges[j];
			char cA[2]; 
			cA[0] = c; 
			cA[1] = '\0';
			// Depending on if we want all raw data or just one adverse selection per stock
			// vector<float> advSelection = calcAdverseSelection(partRates.at(i), cA);
			float advSelection = calcWeightedAdverseSelection(partRates.at(i), cA);
			ss << c <<partRates.at(i)*1000<<".csv";
			writeToFile(advSelection, ss.str());
		}
	}
}

//Test program
int main(int argc, char * argv[]){
	string file = "Resources/ticks.20130423.h5";
	//First populate all stock list
	AdverseSelection::populateStockLists(file);
	for (int i=0; i<AdverseSelection::allStocks.size(); ++i){
		AdverseSelection selection(file, AdverseSelection::allStocks.at(i));
		selection.outputAdvSelToFile();
	}
}
