#ifndef BIGQ_H
#define BIGQ_H
#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include <fstream>
#include <stdlib.h>
#include "Record.h"
#include <vector>
#include <algorithm>
#include "DBFile.h"
#include "Defs.h"

using namespace std;

class BigQ {
private:
Pipe *inPipe, *outPipe;
OrderMaker *sortedOrder;
int runLength;
Page currPage;
Page tempPage;
int pageFilled;
int pNum;
//string 
//ComparisionEngine
//vector<int> 


public:
void insertInFile(vector<Record*>&);
BigQ();
File sortFile;
string sortFileName;
static void* beginSortProcess(void*);
void internalSort();



struct CompareMyRecords
        {
        OrderMaker *pSortOrder;
                CompareMyRecords(OrderMaker *pOM): pSortOrder(pOM) {}

        bool operator()(Record* const& r1, Record* const& r2)
        {
                Record* r11 = const_cast<Record*>(r1);
            Record* r22 = const_cast<Record*>(r2);
	
            ComparisonEngine ce;
            if (ce.Compare(r11, r22, pSortOrder) < 0)
                return true;
            else
                return false;
        }
        };


public:

	BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);
	~BigQ ();
};

#endif
