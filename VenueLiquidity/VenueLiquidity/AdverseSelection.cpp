#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>

using namespace std;
using namespace H5;

void AdverseSelection::parseHdf5Source()
{
	H5File file2(hdf5Source, H5F_ACC_RDONLY);
	
	DataSet dataset = file2.openDataSet("/ticks/AMZN");

	/*DataSpace space = dataset.getSpace();
	space.selectAll();*/
}

AdverseSelection::AdverseSelection(H5std_string source)
{
	hdf5Source = source;
}


AdverseSelection::~AdverseSelection()
{
}

int main(){
	AdverseSelection selection("C:/Users/demon4000/Documents/R/CFEM_Data/ticks.20130423.h5");
	selection.parseHdf5Source();
}
