#include <string>

using namespace std;

class ExegyRow
{
public:
	int row_num;
	float ask;
	char ask_exchange; //char ask_exchange[2];
	long ask_size;
	float bid;
	char bid_exchange[2];
	long bid_size;
	char exchange; //char exchange[2];
	long exchange_time;
	unsigned char instrument_status;
	long latency; 
	unsigned int line;
	unsigned char market_status;
	float prev_close;
	float price;
	unsigned int quals;
	char refresh; //char refresh[2];
	long seq_no;
	long size;
	char sub_market; //char sub_market[2];
	string symbol; //char symbol[9];
	unsigned char thru_exempt;
	string time; //char time[19];
	char type; //char type[2];
	long volume;
};

