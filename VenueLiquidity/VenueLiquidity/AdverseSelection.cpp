#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace H5;


void AdverseSelection::parseHdf5Source()
{
	/*H5File file2(hdf5Source, H5F_ACC_RDONLY);
	
	DataSet dataset = file2.openDataSet("/ticks/AMZN");
	
	size_t size = dataset.getInMemDataSize();

	exegyRow *s = (exegyRow*) malloc(size);
	CompType type = dataset.getCompType();
	cout<<sizeof(exegyRow)<<endl;
	cout<<type.getNmembers()<<endl;
	
	dataset.read(s, type);
	exegyRow r = s[0];*/
}

exegyRow* AdverseSelection::createRow(string line){
	exegyRow* row = new exegyRow;
	string csvItem;
	istringstream myline(line);
	// TODO, filter out bad trades/quotes
    for(int i=0; i<25; ++i){
		getline(myline, csvItem, ',');
		switch(i){
			case 1: row->ask = atof(csvItem.c_str()); break;
			case 3: row->ask_size = atoi(csvItem.c_str()); break;
			case 4: row->bid = atof(csvItem.c_str()); break;
			case 6: row->bid_size = atoi(csvItem.c_str()); break;
			case 8: row->exchange_time = atoi(csvItem.c_str()); break;
			case 14: row->price = atof(csvItem.c_str()); break;
			case 18: row->size = atoi(csvItem.c_str()); break;
			case 20: row->symbol = csvItem; break;
			case 23: row->type = csvItem.at(1); break;
			case 24: row->volume = atoi(csvItem.c_str()); break;
		}
	}
	return row;
}

AdverseSelection::AdverseSelection(string source)
{
	ifstream file;
	file.open(source,ios::in);
	string line;
	while (getline(file,line)){
		exegyRow* row = createRow(line);
		tickData.push_back(row);
		if (row->type == 'T'){
			trades.push_back(row);
		}
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

vector<CLASSIFICATION> AdverseSelection::computeClassification(bool useLeeReady){
	vector<CLASSIFICATION> classifications;

	// For now assume Q-T-Q-T-Q...
	for (int i=1; i<tickData.size(); i+=2){
		float price = tickData.at(i)->price;
		float quote_mid = (tickData.at(i-1)->ask + tickData.at(i-1)->bid)/2;
		if (price < quote_mid){
			classifications.push_back(SELL);
		}else if (price > quote_mid){
			classifications.push_back(BUY);
		}else{
			if (useLeeReady){
				classifications.push_back(tickTest((i-1)/2));
			}else{
				classifications.push_back(NOT_CLASS);
			}
		}
	}

	return classifications;
}

vector<float> AdverseSelection::calcPartWeightAvg(double percent){
	vector<float> partWeightedPrices;
	//TODO Worst Case N^2, will modify to make it faster
	for (int i=0; i<trades.size(); ++i){
		long partVol = trades.at(i)->size/percent; // Calculate next X volumes to use
		double pwp = 0.0;
		for (int j=i+1; j<trades.size(); ++j){
			long cumVol = trades.at(j)->volume - trades.at(i)->volume;
			long weight = cumVol < partVol ? trades.at(j)->size :  partVol - (trades.at(j-1)->volume - trades.at(i)->volume);
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

AdverseSelection::~AdverseSelection()
{

}

int main(){
	AdverseSelection selection("sample_tick.csv");
	vector<float> pwpVec = selection.calcPartWeightAvg(0.5);
	for (int i=0; i<30; ++i){
		cout<<pwpVec.at(i)<<endl;
	}
	vector<CLASSIFICATION> classed = selection.computeClassification(true);
		for (int i=0; i<1000; ++i){
		cout<<classed.at(i)<<endl;
	}
}
