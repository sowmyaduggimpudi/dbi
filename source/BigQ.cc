#include "BigQ.h"

//#define PRINT cout<<__func__<<": "<<__LINE__<<endl;
#define PRINT cout<<__LINE__<<endl;
#define debug 0
static int actRecs;
//static Schema mySchema("catalog", "nation");

BigQ::BigQ(){
}

BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen)
{
        runLength = runlen;
        inPipe= &in;
        outPipe= &out;
        sortedOrder= &sortorder;
	pthread_t thread;
	pNum = 0;
	noOfRuns=0;
	pageFilled = 0;
        pthread_create(&thread, NULL, &beginSortProcess,(void*)this);
	sortFile.Open(0, "sortFile_temp.bin");  
}

void BigQ::internalSort() {
        
        Record recFromPipe;
        vector<Record*> sortVector;
//       int noOfRuns=0;
//	int totalPages=0;

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
	mergeRuns();
	outPipe->ShutDown();
	sortFile.Close();
	remove("sortFile_temp.bin");  

}
void BigQ:: insertInFile(vector<Record*> &sortVector)
{

	int length= sortVector.size();
        vector<Record*> dupVector;
	int currRunLen = 0;
	Record testRec;
	while(!sortVector.empty())
	{
		
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
	return;
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
         
	}
	}
	sortVector.clear();
	
}

int BigQ::mergeRuns()
{
	
	Record firstRecord;
	Page *topPage = new Page[noOfRuns];
	int *pageNum = new int[noOfRuns];
	Record *R = new Record[noOfRuns];
	bool *isRunDone= new bool[noOfRuns];
	int minRun;
	int recsInOutPipe=0;

#if debug
	off_t wPage = 0;
	Record RR;
	Page PP;
	sortFile.GetPage(&PP, wPage); 
	wPage++;
	cout<<"Tot Pages pNum: "<<pNum<<endl;
	while(wPage < pNum) {
		while(!PP.GetFirst(&RR)) {
			if(wPage == pNum -1)
				goto out;
			sortFile.GetPage(&PP, wPage); 
			wPage++;
		}

                outPipe->Insert(&RR);
	}
out:
return 0;
#endif

	for(int i=0; i<noOfRuns; i++)
	{
                pageNum[i]=0;
		isRunDone[i]=0;
		sortFile.GetPage(&topPage[i],(off_t)(i*runLength));
		topPage[i].GetFirst(&R[i]);
	}

	while(true)
	{
		minRun = minIndex(R, isRunDone);
		if(minRun== -1)
			break;
                outPipe->Insert(&R[minRun]);
		recsInOutPipe++;
		while(!topPage[minRun].GetFirst(&R[minRun]))
		{	
			if(pageNum[minRun] == runLength -1)
				{isRunDone[minRun]=1; break;}
			else
			{
			pageNum[minRun]++;
			sortFile.GetPage(&topPage[minRun],(off_t)((minRun*runLength)+pageNum[minRun]));

			}

		}

	}

	delete [] topPage;
	delete [] R;
	delete [] pageNum;
	delete [] isRunDone;
}

int BigQ:: minIndex(Record*R, bool* isRunDone)
{

        int i = 0;
        int min = -1;
        ComparisonEngine ceng;

        i = 0;
        while (i < noOfRuns) {
                if (!isRunDone[i]) {
                        min = i;
                        i++;
                        break;
                }
                i++;
        }

        if (min == -1)
                return min;

        while (i < noOfRuns) {
	//cout<<"i:  "<< i << "NoOfRuns: "<< noOfRuns<<endl;
                if (isRunDone[i]) {
                	i++;
                        continue;
		}

                if (ceng.Compare(&R[min], &R[i], sortedOrder) > 0)
                        min = i;
                i++;
        }

        return min;
}


void* BigQ:: beginSortProcess(void *thread)
{
        BigQ *bigQ = (BigQ *) thread;
        bigQ->internalSort();
}

BigQ::~BigQ () {

}
