#include <string>

using namespace std;

enum CLASSIFICATION{BUY=1, SELL=-1, NOT_CLASS=0};

class ExegyRawData;
class ExegyRow
{
public:
	ExegyRow();
	ExegyRow(const ExegyRawData& data);
	//int row_num;
	float ask;
	char ask_exchange; 
	long ask_size;
	float bid;
	char bid_exchange;
	long bid_size;
	char exchange; 
	long long exchange_time;
	unsigned char instrument_status;
	long latency; 
	unsigned int line;
	unsigned char market_status;
	float prev_close;
	float price;
	unsigned int quals;
	long size;
	string symbol; 
	char type;
	long volume;
	CLASSIFICATION buy_sell;
};


class ExegyRawData
{
public:
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
};
