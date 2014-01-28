#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     
#include <stdio.h>
#include <assert.h>

bool withinCloseRange(float a, float b){
	if (abs(a-b) / b < 0.03){
		return true;
	}return false;
}

void advSelectionTest(){
	std::string file = "Resources/ticks.20130423.h5";
	AdverseSelection::populateStockLists(file);
	AdverseSelection advSel(file, "AAPL", true);
	assert(advSel.trades.size(), 37);
	assert(advSel.trades.size(), 37);
	vector<CLASSIFICATION> classifications;
	for (int i=0; i<advSel.trades.size(); ++i){
		classifications.push_back(advSel.trades.at(i)->buy_sell);
	}
	int handCalced[37] = {1,-1,1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,-1,1,1,1,1,1,1,1,1};
	for (int j=0; j<37; ++j){
		assert(handCalced[j] == classifications[j]);
	}

	char exchanges[2] = {'P','\0'};
	vector<ExegyRow*> advSels = advSel.calcAdverseSelection(0.3f, exchanges);
	float s = advSels.at(0)->advSPrices[0];
	assert(withinCloseRange(s, 4.46676e-5));

	float partRate = 0.3f;
	vector<int> *noCalcIndicies = new vector<int>();
	vector<float> pwp = advSel.calcPartWeightAvg(partRate, exchanges, noCalcIndicies);
	assert(withinCloseRange(pwp.at(0), 403.03798));
	assert(withinCloseRange(pwp.at(1), 403.08002));
	assert(pwp.size() == 17);
	assert(find(noCalcIndicies->begin(), noCalcIndicies->end(), 16)!=noCalcIndicies->end()); 
	assert(find(noCalcIndicies->begin(), noCalcIndicies->end(), 17)==noCalcIndicies->end());

	//used for total volume weighting, excludes volumes for trades whose pwps are not calculated
	assert(advSel.getLastCumVol(exchanges, noCalcIndicies) == 2700); 
	float weightedAdvSel = advSel.calcWeightedAdverseSelection(partRate,exchanges);
	assert(withinCloseRange(weightedAdvSel, -1.62417e-6));
}

void mergeFiles(vector<string> files){
	if (files.size()==0){return;}

	string fileName = "MergedOutput.csv";
	ofstream fileWriter(fileName);
	cout<<"Append to file: "<<fileName<<endl;
	if (fileWriter.is_open()){ fileWriter.close(); }
	fileWriter.open(fileName.c_str(),std::ofstream::out | std::ofstream::app);
	if (fileWriter.fail()) {
		cerr << "open failure: " << strerror(errno) << '\n';
	}

	string line;
	string header = "Ticker,Exchange (the calculation takes place),Closing Price,AdvSel (30%),AdvSel (10%),AdvSel (3%),AdvSel (1%),AdvSel (0.3%),AdvSel (0.1%)";
	fileWriter << header << "\n";
	for (int i=0; i<files.size(); ++i){
		string fname = files.at(i);
		ifstream fileReader (fname); 
		while(getline(fileReader, line)){
			fileWriter << line << "\n";
		}
		fileReader.close();
		remove(fname.c_str());
	}
	fileWriter.close();
}

//Test program
int main(int argc, char * argv[]){
	cout<<"ARG #: "<<argc<<endl;
	int begin = 0;
	int end = 0;
	string file = "Resources/ticks.20130424.h5";

	if (argc == 2){
		vector<string> files;
		string location = string(argv[1]);

		if(strcmp(location.c_str(), string("TEST").c_str())==0){
			advSelectionTest();
			cout<<"Unit Test successful"<<endl;
		}

		else{
			files.push_back(location + "Output_0.csv");
			files.push_back(location + "Output_1000.csv");
			files.push_back(location + "Output_2000.csv");
			files.push_back(location + "Output_3000.csv");
			files.push_back(location + "Output_4000.csv");
			files.push_back(location + "Output_5000.csv");
			files.push_back(location + "Output_6000.csv");
			files.push_back(location + "Output_7000.csv");
			mergeFiles(files);
		}
	}

	else if (argc == 4){
		begin = atoi(argv[1]);
		end = atoi(argv[2]);
		file = string(argv[3]);
		
		AdverseSelection::populateStockLists(file);

		for (int i=begin; i<end; ++i){
			cout<<i<<endl;
			AdverseSelection selection(file, AdverseSelection::allStocks.at(i));
			stringstream ss;
			ss<<argv[1]<<".csv";
			selection.outputAdvSelToFile(true, ss.str(), true);
		}
	}
}