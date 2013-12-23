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

//TODO, make this work...
void AdverseSelection::parseHdf5Source()
{
	H5File file2(hdf5Source, H5F_ACC_RDONLY);
	
	DataSet dataset = file2.openDataSet("/ticks/AMZN");
	
	size_t size = dataset.getInMemDataSize();

	ExegyRawData *s = (ExegyRawData*) malloc(size);
	CompType type = dataset.getCompType();
	for (int i=0; i<24; ++i){
		//cout<<type.getMemberDataType(i).getTag()<<"|";
		cout<<type.getMemberDataType(i).getClass()<<endl;
		//dataset.read(s, type);
	}
	/*CompType mtype3( sizeof(ExegyRow) );
	mtype3.insertMember("ask", 0, PredType::NATIVE_FLOAT);
	mtype3.insertMember("ask_exchange", sizeof(float), PredType::NATIVE_CHAR);
	mtype3.insertMember("ask_size", sizeof(char[2]), PredType::NATIVE_LONG);
	mtype3.insertMember("bid", sizeof(long), PredType::NATIVE_FLOAT);
	mtype3.insertMember("bid_exchange", sizeof(float), PredType::NATIVE_CHAR);
	mtype3.insertMember("bid_size", sizeof(char[2]), PredType::NATIVE_LONG);
	mtype3.insertMember("exchange", sizeof(long), PredType::NATIVE_CHAR);
	mtype3.insertMember("exchange_time", sizeof(char[2]), PredType::NATIVE_LONG);
	mtype3.insertMember("instrument_status", sizeof(long), PredType::NATIVE_LONG);
	mtype3.insertMember("latency", sizeof(unsigned char), PredType::NATIVE_LONG);
	mtype3.insertMember("line", sizeof(long), PredType::NATIVE_LONG);
	mtype3.insertMember("market_status", sizeof(unsigned int), PredType::NATIVE_LONG);
	mtype3.insertMember("prev_close", sizeof(unsigned char), PredType::NATIVE_FLOAT);
	mtype3.insertMember("price", sizeof(float), PredType::NATIVE_FLOAT);
	mtype3.insertMember("quals", sizeof(float), PredType::NATIVE_LONG);
	mtype3.insertMember("refresh", sizeof(unsigned int), PredType::NATIVE_CHAR);
	mtype3.insertMember("seq_no", sizeof(char[2]), PredType::NATIVE_LONG);
	mtype3.insertMember("size", sizeof(long), PredType::NATIVE_LONG);
	mtype3.insertMember("sub_market", sizeof(long), PredType::NATIVE_CHAR);
	mtype3.insertMember("symbol", sizeof(char[2]), PredType::NATIVE_CHAR);
	mtype3.insertMember("thru_exempt", sizeof(char[9]), PredType::NATIVE_CHAR);
	mtype3.insertMember("time", sizeof(unsigned char), PredType::NATIVE_CHAR);
	mtype3.insertMember("type", sizeof(char[19]), PredType::NATIVE_CHAR);
	mtype3.insertMember("volume", sizeof(char[2]), PredType::NATIVE_LONG);*/
	memset(s,0,size);
	//cout<<sizeof(ExegyRow)<<endl;
	for (int i=0; i<24; ++i){
		cout<<sizeof(char[2])<<"|"<<sizeof(long)<<endl;
		cout<<sizeof(type.getMemberDataType(i).getClass())<<endl;
		//dataset.read(s, type);
	}
	dataset.read(s, type);
	ExegyRawData r = s[0];
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
			case 0: row->row_num = atoi(&csvItem.at(1)); break;
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
	hdf5Source = H5std_string("Resources/ticks.20130423.h5");
	ifstream file;
	file.open(fn,ios::in);
	string line;
	while (getline(file,line)){
		ExegyRow* row = createRow(line);
		if (row != NULL){
			tickData.push_back(row);
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
	file.close();
}

AdverseSelection::AdverseSelection(string source)
{
	parseCsv(source);
	for(map<char,vector<ExegyRow*> >::iterator it = exchange_trades_m.begin(); it != exchange_trades_m.end(); ++it) {
		vector<CLASSIFICATION> classed;
		exchange_classed_m[it->first] = classed; 
	}
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
				exchange_classed_m[ex].push_back(SELL);
			}else if (price > quote_mid){
				exchange_classed_m[ex].push_back(BUY);
			}else{
				if (useLeeReady){
					exchange_classed_m[ex].push_back(tickTest(j));
				}else{
					exchange_classed_m[ex].push_back(NOT_CLASS);
				}
			}
			++j;
		}
	}
}

vector<long> AdverseSelection::getCumVolPerEx(char exchange){
	vector<long> cumVols;
	vector<ExegyRow*> trades = exchange_trades_m[exchange];
	long vol = 0;
	for (int i=0; i<trades.size(); ++i){
		vol += trades.at(i)->size;
		cumVols.push_back(vol);
	}
	return cumVols;
}

vector<long> AdverseSelection::getTotalSumPerEx(char exchange){
	vector<long> totalSum;
	vector<ExegyRow*> trades = exchange_trades_m[exchange];
	long s = 0.0;
	for (int i=0; i<trades.size(); ++i){
		s += trades.at(i)->size * trades.at(i)->price;
		totalSum.push_back(s);
	}
	return totalSum;
}

//O(N) Time
vector<float> AdverseSelection::calcPartWeightAvg(float percent, char exchange){
	vector<ExegyRow*> trades = exchange_trades_m[exchange];
	vector<float> partWeightedPrices;

	vector<long> totalSum = getTotalSumPerEx(exchange);
	vector<long> cumVols = getCumVolPerEx(exchange);

	float pwp = 0.0;
	int lastPt = 0;
	for (int i=0; i<trades.size(); ++i){
		int nextInd;
		long partVol = trades.at(i)->size/percent; // Calculate next X volumes to use		
		for (int j=i+1; j<trades.size(); ++j){
			long cumVol = (cumVols.at(j) - cumVols.at(i));
			if (cumVol >= partVol){ 
				if (cumVol > partVol){
					long diff = cumVol - partVol;
					pwp = (totalSum.at(j) - totalSum.at(i) - (trades.at(j)->price*diff))/(cumVols.at(j) - cumVols.at(i) - diff);
				}else{
					pwp = (totalSum.at(j) - totalSum.at(i))/(cumVols.at(j) - cumVols.at(i));
				}
				partWeightedPrices.push_back(pwp);
				break;
			}
		}
	}
	return partWeightedPrices;
}

vector<float> AdverseSelection::calcAdverseSelection(float percent, char exchange){
	vector<float> pwp = calcPartWeightAvg(percent, exchange);
	vector<float> adverseSelection;
	vector<ExegyRow*> egrs = exchange_trades_m[exchange];
	vector<CLASSIFICATION> classed = exchange_classed_m[exchange];
	for (int i=0; i<pwp.size(); ++i){
		float ads = classed.at(i) * (egrs.at(i)->price - pwp.at(i));
		adverseSelection.push_back(ads);
	}
	return adverseSelection;
}

AdverseSelection::~AdverseSelection(){
	std::for_each (tickData.begin(), tickData.end(), Deleter());
    tickData.clear();
}

//Test program
int main(int argc, char * argv[]){
	AdverseSelection selection("Resources/AMZN_sample.csv");
	selection.parseHdf5Source();
	cout<<"Done Parsing"<<endl;
	vector<float> pwpVec = selection.calcPartWeightAvg(0.1,'Q');
	for (int i=0; i<10; ++i){
		cout<<pwpVec.at(i)<<endl;
	}
	selection.computeClassification(true);
	vector<float> advSelection = selection.calcAdverseSelection(0.1,'Q');
	for (int i=0; i<10; ++i){
		cout<<advSelection.at(i)<<endl;
	}
}
