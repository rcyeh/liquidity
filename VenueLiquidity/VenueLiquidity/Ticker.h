#include <string>
#include <vector>

using namespace std;
class Ticker
{
public:
	Ticker(string tic, char ex);
	vector<float> pwps;
	float price;
	string ticker;
	char exchange;
	string getData();
};

