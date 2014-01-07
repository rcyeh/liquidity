#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>     

//Test program
int main(int argc, char * argv[]){
	string file = "Resources/ticks.20130423.h5";
	//First populate all stock list
	AdverseSelection::populateStockLists(file);
	//cout<<AdverseSelection::allStocks.at(15)<<endl;
	for (int i=0; i<AdverseSelection::allStocks.size(); ++i){
		cout<<i<<endl;
		AdverseSelection selection(file, AdverseSelection::allStocks.at(i));
		//AdverseSelection selection(file, "ABW_hyphen_B");
		selection.outputAdvSelToFile();
	}
}