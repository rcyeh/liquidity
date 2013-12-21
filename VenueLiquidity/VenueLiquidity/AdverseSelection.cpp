#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

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

void AdverseSelection::parseHdf5Source()
{
	/*H5File file2(hdf5Source, H5F_ACC_RDONLY);
	
	DataSet dataset = file2.openDataSet("/ticks/AMZN");
	
	size_t size = dataset.getInMemDataSize();

	ExegyRow *s = (ExegyRow*) malloc(size);
	CompType type = dataset.getCompType();
	cout<<sizeof(ExegyRow)<<endl;
	cout<<type.getNmembers()<<endl;
	
	dataset.read(s, type);
	ExegyRow r = s[0];*/
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
			case 8: row->exchange_time = atoi(csvItem.c_str()); break;
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
	for (int i=tickData.size()-1; i>0; --i){
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

vector<float> AdverseSelection::calcPartWeightAvg(double percent, char exchange){
	vector<ExegyRow*> trades = exchange_trades_m[exchange];
	vector<float> partWeightedPrices;
	//TODO Worst Case N^2, will modify to make it faster
	for (int i=0; i<trades.size(); ++i){
		long partVol = trades.at(i)->size/percent; // Calculate next X volumes to use
		double pwp = 0.0;
		long cumVol = 0;
		for (int j=i+1; j<trades.size(); ++j){
			//long cumVol = trades.at(j)->volume - trades.at(i)->volume;
			cumVol += trades.at(j)->size;
			//long weight = cumVol < partVol ? trades.at(j)->size :  partVol - (trades.at(j-1)->volume - trades.at(i)->volume);
			long weight = cumVol < partVol ? trades.at(j)->size :  partVol - (cumVol - trades.at(j)->size);
			pwp += trades.at(j)->price * weight;
			if (cumVol >= partVol){ 
				pwp /= partVol;
				partWeightedPrices.push_back(pwp);
				break; 
			}
		}
	}
	return partWeightedPrices;
}

vector<float> AdverseSelection::calcAdverseSelection(double percent, char exchange){
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

AdverseSelection::~AdverseSelection()
{
	std::for_each (tickData.begin(), tickData.end(), Deleter());
    tickData.clear();
}

//Test program
int main(int argc, char * argv[]){
	AdverseSelection selection("Resources/AMZN.csv");
	cout<<"Done Parsing"<<endl;
	vector<float> pwpVec = selection.calcPartWeightAvg(0.1,'Q');
	for (int i=0; i<30; ++i){
		cout<<pwpVec.at(i)<<endl;
	}
	selection.computeClassification(true);
	vector<float> advSelection = selection.calcAdverseSelection(0.1,'Q');
	for (int i=0; i<30; ++i){
		cout<<advSelection.at(i)<<endl;
	}
}
