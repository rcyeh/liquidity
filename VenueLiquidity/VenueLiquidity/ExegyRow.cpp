#include "ExegyRow.h"
#include <sstream>

ExegyRow::ExegyRow()
{
}

string ExegyRow::getTradeData(){
	stringstream ss;
	ss <<buy_sell<<","<<size<<","<<price<<","<<exchange;
	return ss.str();
}

ExegyRow::ExegyRow(const ExegyRawData& data)
{
	ask = data.ask;
	ask_exchange = data.ask_exchange[0]; 
	ask_size = data.ask_size;
	bid = data.bid;
	bid_exchange = data.bid_exchange[0];
	bid_size = data.bid_size;
	exchange = data.exchange[0]; 
	time = data.time;
	instrument_status = data.instrument_status;
	latency = data.latency; 
	line = data.line;
	market_status = data.market_status;
	prev_close = data.prev_close;
	price = data.price;
	quals = data.quals;
	size = data.size;
	symbol = string(data.symbol); 
	type = data.type[0];
	volume = data.volume;
}


