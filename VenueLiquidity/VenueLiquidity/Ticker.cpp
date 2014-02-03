#include "Ticker.h"
#include <sstream>

Ticker::Ticker(string tic, char ex)
{
	ticker = tic;
	exchange = ex;
}


string Ticker::getData(void)
{
	stringstream ss;
	ss << ticker << "," << exchange << "," << price << ",";
	for (int i=0; i<pwps.size(); ++i){
		ss << pwps.at(i) << ",";
	}
	return ss.str();
}
