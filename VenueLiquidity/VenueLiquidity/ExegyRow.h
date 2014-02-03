#include <string>
#include <vector>

using namespace std;

enum CLASSIFICATION{BUY=1, SELL=-1, NOT_CLASS=0};
enum TRADE_TYPE{MKT, LMT};

class ExegyRawData;
class ExegyRow
{
public:
	ExegyRow();
	ExegyRow(const ExegyRawData& data);
	string getTradeData();
	//int row_num;
	float adv_selection;
	float ask;
	char ask_exchange; 
	long ask_size;
	float bid;
	char bid_exchange;
	long bid_size;
	char exchange; 
	string time;
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

struct EndPWAPBookmark {
  long finalShareVolume;
  size_t trade_idx;
  char exchange;
  std::string ticker;
  double participationRate;
  double advSelDollarBars;

  static std::vector <EndPWAPBookmark> allBookmarks;
  static std::vector < std::pair< long, double > > cumulativeVolume; // and price * volume, indexed at the start of each trade
};