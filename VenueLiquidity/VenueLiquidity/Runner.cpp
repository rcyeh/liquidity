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
#include "ASParser.h"

void mergeFiles(vector<string> files, string fileName){
	if (files.size()==0){return;}
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

		files.push_back(location + "BasicState0.csv");
		files.push_back(location + "BasicState1000.csv");
		files.push_back(location + "BasicState2000.csv");
		files.push_back(location + "BasicState3000.csv");
		files.push_back(location + "BasicState4000.csv");
		files.push_back(location + "BasicState5000.csv");
		files.push_back(location + "BasicState6000.csv");
		files.push_back(location + "BasicState7000.csv");
		mergeFiles(files, "BasicStat.csv");

		files.clear();
		files.push_back(location + "AdvSelState0.csv");
		files.push_back(location + "AdvSelState1000.csv");
		files.push_back(location + "AdvSelState2000.csv");
		files.push_back(location + "AdvSelState3000.csv");
		files.push_back(location + "AdvSelState4000.csv");
		files.push_back(location + "AdvSelState5000.csv");
		files.push_back(location + "AdvSelState6000.csv");
		files.push_back(location + "AdvSelState7000.csv");
		mergeFiles(files, "AdvSelStat.csv");
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
			selection.calculateAndOutput("BasicState"+ss.str(), "AdvSelState"+ss.str());
		}
	}
}