#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>

using namespace std;
using namespace H5;

typedef struct exegyRow {
	float ask;
	char ask_exchange[2];
	long ask_size;
	float bid;
	char bid_exchange[2];
	long bid_size;
	char exchange[2];
	long exchange_time;
	unsigned char instrument_status;
	long latency; 
	unsigned int line;
	unsigned char market_status;
	float prev_close;
	float price;
	unsigned int quals;
	char refresh[2];
	long seq_no;
	long size;
	char sub_market[2];
	char symbol[9];
	unsigned char thru_exempt;
	char time[19];
	char type[2];
	long volume;
} exegyRow;

void AdverseSelection::parseHdf5Source()
{
	H5File file2(hdf5Source, H5F_ACC_RDONLY);
	
	DataSet dataset = file2.openDataSet("/ticks/AMZN");
	
	size_t size = dataset.getInMemDataSize();
	exegyRow *s = (exegyRow*) malloc(size);
	CompType type = dataset.getCompType();
	cout<<type.getNmembers()<<endl;
	dataset.read(s, type);
	exegyRow r = s[1];
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
