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

/*
void task(int begin, int end, string file){
	for (int i=begin; i<end; ++i){
		cout<<i<<endl;
		stringstream ss;
		ss << "AllTradesInfo" << begin << "_" << end << ".csv";
		AdverseSelection selection(file, AdverseSelection::allStocks.at(i));	
		selection.outputAdvSelToFile(false, ss.str());
	}
}

void task2(int begin, int end, string file){
	int i=begin;
	cout<<i<<endl;
	stringstream ss;
	ss << "_" << begin << "_" << end << ".csv";
	string f = ss.str();

	stringstream s2;
	s2 << begin << ".csv";
	ifstream fileReader (s2.str());
	if (fileReader){ // continue leftover work
		string s;
		while(getline(fileReader, s));
		int progress = atoi(s.c_str());
		i = progress+1;
	}
	fileReader.close();
	for (; i<end; ++i){
		AdverseSelection selection(file, AdverseSelection::allStocks.at(i), i, begin);	
		selection.outputAdvSelToFile(true, f);
	}
}
*/

//Test program
int main(int argc, char * argv[]){
	string file = "Resources/ticks.20130424.h5";
	//First populate all stock list
	AdverseSelection::populateStockLists(file);
	int begin = 0;
	int end = AdverseSelection::allStocks.size();
	if (argc > 1){
		begin = atoi(argv[1]);
		end = atoi(argv[2]);
	}
	for (int i=0; i<AdverseSelection::allStocks.size(); ++i){
		cout<<i<<endl;
		AdverseSelection selection(file, AdverseSelection::allStocks.at(i));
		stringstream ss;
		ss<<argv[1]<<".csv";
		selection.outputAdvSelToFile(true, ss.str());
	}
	
	/*
	vector<shared_ptr<boost::thread> > threads;
	vector<vector<string> > files;
	//vector<string> files;
	int threadNum = 1;
	int begin = 0;
	int increment = AdverseSelection::allStocks.size()/threadNum;
	int end = increment;
	for (int i=0; i<threadNum; ++i){
		if (i == threadNum -1){ // last iteration
			end = AdverseSelection::allStocks.size();
		}
		//threads.push_back(make_shared<boost::thread>(task, begin, end, file));
		threads.push_back(make_shared<boost::thread>(task2, begin, end, file));

		char exchanges[17] = {'A','B','C','D','E','I','J','K','M','N','P','Q','W','X','Y','Z','\0'};
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
	for (int i=0; i<threadNum; ++i){
		threads.at(i)->join();
	}
	//mergeFiles(files);
	for (int i=0; i<files.size(); ++i){
		mergeFiles(files.at(i));
	}
	*/
}