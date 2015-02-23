#include "BigQ.h"

#define PRINT cout<<__func__<<": "<<__LINE__<<endl;
static int actRecs;
//static Schema mySchema("catalog", "nation");

BigQ::BigQ(){
inPipe = new Pipe(100);
outPipe = new Pipe(100);
runLength=0; 
}

BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen)
{
        runLength = runlen;
        inPipe= &in;
        outPipe= &out;
        sortedOrder= &sortorder;
	pthread_t thread;
	pNum = 0;
	pageFilled = 0;
        pthread_create(&thread, NULL, &beginSortProcess,(void*)this);
	sortFile.Open(0, "sortFile_temp.bin");  
}

void BigQ::internalSort() {
        
        Record recFromPipe;
        vector<Record*> sortVector;
        int noOfRuns=0;

while(true)

{
         int ret = inPipe->Remove(&recFromPipe);
	Record *tempRecord = new Record();
        if(ret ==0 )
        {
		if(sortVector.empty())
                break;
		else
		{
			sort(sortVector.begin(),sortVector.end(), CompareMyRecords(sortedOrder));
			
			noOfRuns=noOfRuns+1;
			insertInFile(sortVector);
			break;
		}
        }
	tempRecord->Copy(&recFromPipe);
	if(!tempPage.Append(&recFromPipe))
	{
	pageFilled=pageFilled+1;
	tempPage.EmptyItOut();
		if(pageFilled == runLength)
		{ 
		noOfRuns=noOfRuns+1;
		sort(sortVector.begin(),sortVector.end(), CompareMyRecords(sortedOrder));
		pageFilled = 0;
		insertInFile(sortVector);
		}

	tempPage.Append(&recFromPipe);
	}
		sortVector.push_back(tempRecord);


}
	cout<< "Act Records : "<<actRecs<<endl ;
	outPipe->ShutDown();
}
void BigQ:: insertInFile(vector<Record*> &sortVector)
{

	int length= sortVector.size();
        vector<Record*> dupVector;
	int currRunLen = 0;
	while(!sortVector.empty())
	{
		//sortVector.front()->Print(&mySchema);
		if(!currPage.Append(sortVector.front()))
		{
			if(currRunLen==runLength)
			{	
				actRecs += currPage.GetNumRecs();
				sortFile.AddPage(&currPage, pNum);
				pNum++;
				currPage.EmptyItOut();
				break;
			}
			else{
				actRecs += currPage.GetNumRecs();
				sortFile.AddPage(&currPage, pNum);
				pNum++;
				currRunLen++;
				currPage.EmptyItOut();
				currPage.Append(sortVector.front());
			}
		}
		sortVector.erase(sortVector.begin());
	}
	
	if(currRunLen==runLength)
	{
		while(!sortVector.empty())
		{
			dupVector.push_back(sortVector.front());
			if(!tempPage.Append(sortVector.front()))
				{
					pageFilled++;
					tempPage.EmptyItOut();
					tempPage.Append(sortVector.front());
				}
			sortVector.erase(sortVector.begin());
		}

	sortVector.clear();
	sortVector=dupVector;
	}
		
	else
	{
	 sortFile.AddPage(&currPage, pNum);
	 actRecs += currPage.GetNumRecs();
       	 pNum++;
	 currRunLen++;
     	 currPage.EmptyItOut();
	 while(currRunLen!=runLength)						 
	{
		sortFile.AddPage(&currPage, pNum);
		pNum++;
		currRunLen++;
				
		actRecs += currPage.GetNumRecs();
//		PRINT
         
	}
	}
	sortVector.clear();
//	PRINT

}


void* BigQ:: beginSortProcess(void *thread)
{
        BigQ *bigQ = (BigQ *) thread;
        bigQ->internalSort();
}

BigQ::~BigQ () {

}
