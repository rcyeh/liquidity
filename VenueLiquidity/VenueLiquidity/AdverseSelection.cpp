#include "AdverseSelection.h"
#include "H5Cpp.h"
#include <iostream>

using namespace std;
using namespace H5;

typedef struct exegyRow {
	float ask;
	char ask_exchange[4];
	int ask_size;
	float bid;
	char bid_exchange[4];
	int bid_size;
	char exchange[4];
	int exchange_time;
	int instrument_status;
	int latency; 
	int line;
	int market_status;
	float prev_close;
	float price;
	int quals;
	char refresh[4];
	int seq_no;
	int size;
	char sub_market[4];
	char symbol[4];
	int thru_exempt;
	char time[4];
	char type[4];
	int volume;
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
