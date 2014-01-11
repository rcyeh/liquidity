#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     
#include <stdio.h>
//#include <boost/thread.hpp>

void mergeFiles(vector<string> files){
	if (files.size()==0){return;}

	string fileName = files.at(0);
	ofstream fileWriter(fileName);
	cout<<"Append to file: "<<fileName<<endl;
	if (fileWriter.is_open()){ fileWriter.close(); }
	fileWriter.open(fileName.c_str(),std::ofstream::out | std::ofstream::app);
	if (fileWriter.fail()) {
		cerr << "open failure: " << strerror(errno) << '\n';
	}

	string line;
	for (int i=1; i<files.size(); ++i){
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
	if (argc == 1){
		//First populate all stock list
		AdverseSelection::populateStockLists(file);
		end = AdverseSelection::allStocks.size();
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
			selection.outputAdvSelToFile(true, ss.str());
		}
	}

	/*else{
		vector<vector<string> > files;
		char exchanges[17] = {'A','B','C','D','E','I','J','K','M','N','P','Q','W','X','Y','Z','\0'};

		int threadNum = 1;
		int begin = 0;
		int increment = AdverseSelection::allStocks.size()/threadNum;
		int end = increment;
		for (int i=0; i<threadNum; ++i){
			if (i == threadNum -1){ // last iteration
				end = AdverseSelection::allStocks.size();
			}
			stringstream ss;
			vector<string> fs;
			vector<int> partRates;
			partRates.push_back(1);
			partRates.push_back(3);
			partRates.push_back(10);
			partRates.push_back(30);
			partRates.push_back(100);
			partRates.push_back(300);
			for (int i=0; i<strlen(exchanges); ++i){
				for (int j=0; j<partRates.size(); ++j){
					ss << exchanges[i] <<partRates.at(j) << "_" << begin << "_" << end << ".csv";;
					fs.push_back(ss.str());
				}
			}
			files.push_back(fs);
			begin += increment;
			end += increment;
		}
		for (int i=0; i<files.size(); ++i){
			mergeFiles(files.at(i));
		}
	}*/
}