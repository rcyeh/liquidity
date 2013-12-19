#pragma once
#include <string>
#include "hdf5.h"
#include "H5Cpp.h"

using namespace std;

class AdverseSelection
{
private: 
	H5std_string hdf5Source;
public:
	AdverseSelection(const H5std_string source);
	void parseHdf5Source();
	~AdverseSelection();
};

