#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "GenDBFile.h"
#include "SortedDBFile.h"
#include "Defs.h"
#include "BigQ.h"
#include <cstdio>
#include <sys/time.h>
#include <sstream>

using namespace std;
#include <fstream>
#include <iostream>
#include <string.h>

SortedFile::SortedFile()
{
	pageReadInProg = 0;
	currPageIndex = 0;
	counter = 0;
	hasSortOrder = 1;
	flag = 0;
	found = 0;
}

int SortedFile::Create(char *f_path, fType f_type, void *startup)
{
	checkIsFileOpen.open(f_path, ios_base::out | ios_base::in);
	currFile.Open(0, f_path);
	file_path = f_path;
	char path[100];
	sprintf(path, "%s.metadata", f_path);
	FILE *fptr = fopen(path, "wr");

	fwrite((int *)&f_type, 1, sizeof(f_type), fptr);

	SortInfo *si = new SortInfo();

	si->myOrder = new OrderMaker();
	memcpy(si->myOrder, ((SortInfo *)startup)->myOrder, sizeof(OrderMaker));

	si->runLength = ((SortInfo *)startup)->runLength;

	fwrite((int *)&si->runLength, 1, sizeof(int), fptr);
	fwrite((OrderMaker *)si->myOrder, 1, sizeof(OrderMaker), fptr);
	fclose(fptr);
	return 1;
}

void SortedFile::Load(Schema &f_schema, char *loadpath) {
	tblFile = fopen(loadpath, "rb");
	if (!tblFile) {
		cerr << "\nFailed to Open the file: %s" << loadpath;
		return;
	}

	currRecord = new (std::nothrow) Record;
	while (currRecord->SuckNextRecord(&f_schema, tblFile)) {
		Add(*currRecord);
	}

	delete currRecord;
}

int SortedFile::Open(char *f_path) {
	char path[100];
	fType f_type;
	sprintf(path, "%s.metadata", f_path);
	FILE *fptr = fopen(path, "r");
	SortInfo si;
	si.myOrder = new OrderMaker();
	if (!fread(&f_type, 1, sizeof(fType), fptr)) {
		cerr << "\n f_type Read Error";
		exit(1);
	}

	fread((int *)&si.runLength, 1, sizeof(int), fptr);
	fread((OrderMaker *)si.myOrder, 1, sizeof(OrderMaker), fptr);
	file_path = f_path;
	sortOrder = new OrderMaker();
	memcpy(sortOrder, si.myOrder, sizeof(OrderMaker));
	runLen = si.runLength;
	fclose(fptr);
	checkIsFileOpen.open(f_path, ios_base::out | ios_base::in);
	if (checkIsFileOpen.is_open()) {
		currFile.Open(1, f_path);
	} else {
		currFile.Open(0, f_path);
	}

	currMode = READING;
	MoveFirst();
	return 1;
}

int SortedFile::Close()
{
	if (!outPipe || !inPipe)
		return currFile.Close();

	if(WRITING == currMode){
		mergeInflghtRecs();
	}

	if(inPipe!=NULL) {
		delete inPipe;
		inPipe = NULL;
	}

	if(outPipe!=NULL) {
		delete outPipe  ;
		outPipe = NULL;
	}

}

void SortedFile::MoveFirst()
{

	if (currFile.GetLength() == 0) {
		return;
	}
	currPageIndex = 0;
	currFile.MoveFirst();
	currFile.GetPage(&currPage, currPageIndex++);
	pageReadInProg = 1;
}

void* bigQueue1(void *vptr)
{
	threadParams_t *inParams = (threadParams_t *) vptr;
	BigQ bq(*inParams->inPipe, *inParams->outPipe, *inParams->sortOrder,
			inParams->runLen);
}

void SortedFile::Add(Record &rec) {
	counter++;
	if (READING == currMode) {
		currMode = WRITING;

		if (flag == 0) {
			inPipe = new Pipe(IN_OUT_PIPE_BUFF_SIZE);
			outPipe = new Pipe(IN_OUT_PIPE_BUFF_SIZE);

			pthread_t thread1;

			threadParams_t *tp = new (std::nothrow) threadParams_t;

			/*
			 * use a container to pass arguments to worker thread
			 */
			tp->inPipe = inPipe;
			tp->outPipe = outPipe;
			tp->sortOrder = sortOrder;
			tp->runLen = runLen;
			//bigQ = new BigQ(*inPipe, *outPipe, *sortOrder, runLen);
			inPipe->Insert(&rec);
			pthread_create(&thread1, NULL, bigQueue1, (void *) tp);
			flag = 1;
		}
	} else if (WRITING == currMode) {
	  inPipe->Insert(&rec);
	} else {
	}
}

void SortedFile::AppendSequential(Record &appendme){


	  if(pageReadInProg==0) {
	    // currPageIndex = 460;
	    currFile.AddPage(&currPage, currFile.GetLength());
	    pageReadInProg = 1;
	  }


	  if(currFile.GetLength()>0) //existing page
	  {
	    currFile.GetPage(&currPage,currFile.GetLength()-2);
	    currPageIndex = currFile.GetLength()-2;
	  }
	  if(!currPage.Append(&appendme)) //full page
	  {
	    currPage.EmptyItOut();
	    currPage.Append(&appendme);
	    currPageIndex++;
	  }

	  currFile.AddPage(&currPage,currPageIndex);


}

void SortedFile::createMetaDataFile(char *fpath, fType f_type, OrderMaker *sortOrder,int runLen)
{
	char path[100];
	sprintf(path, "%s.metadata", file_path);
	FILE *fptr = fopen(path, "wr");
	fwrite((int *)&f_type, 1, sizeof(f_type), fptr);
	fwrite((int *)&runLen, 1, sizeof(int), fptr);
	fwrite((OrderMaker *)sortOrder, 1, sizeof(OrderMaker), fptr);
	fclose(fptr);

}

void SortedFile::mergeInflghtRecs() 
{
	inPipe->ShutDown();
	Record *pipeRec;
	Record *fileRec;
	ComparisonEngine comp;
	time_t seconds;
	seconds = time(NULL);
	struct timeval tval;
	gettimeofday(&tval, NULL);
	stringstream ss;
	ss << tval.tv_sec;
	ss << ".";
	ss << tval.tv_usec;
	string filename = "mergeFile" + ss.str();
	struct {OrderMaker *o; int l;} startup = {sortOrder, runLen};
	DBFile tmp;
	tmp.Create (strdup(filename.c_str()), heap, &startup);
	int fromPipe = 0, fromFile = 0;
	tmp.MoveFirst();
	MoveFirst();
	int k=0,l=0,m=0,n=0,o=0,p=0,q=0;
	int pipeYes=1,fileYes=1;
	pipeRec = new Record;
	fileRec = new Record;
	fromPipe = outPipe->Remove(pipeRec);
	fromFile = getRecordWithoutSort(*fileRec);
	while (1) {
		if(fromPipe)
			++k;

		if(fromFile)
			++l;

		if (fromPipe && fromFile) {
			++m;
			if (comp.Compare(pipeRec, fileRec, sortOrder) >= 0) {
				tmp.Add(*fileRec);
				fileRec = new Record;
				fromFile = getRecordWithoutSort(*fileRec);
				pipeYes=0;fileYes=1;
				++p;
			} else {
				tmp.Add(*pipeRec);
				pipeRec = new Record;
				fromPipe = outPipe->Remove(pipeRec);
				fileYes=0;pipeYes=1;
				++q;
			}

			continue;

		}
		else if (fromPipe && !fromFile) {
			++n;
			tmp.Add(*pipeRec);
			pipeRec = new Record;
			fromPipe = outPipe->Remove(pipeRec);
			fileYes=0;pipeYes=1;
		} else  if(!fromPipe && fromFile){
			++o;
			tmp.Add(*fileRec);
			fileRec = new Record;
			fromFile = getRecordWithoutSort(*fileRec);
			pipeYes=0;fileYes=1;
		} else {
			break;
		}
	}

	currFile.Close();
	tmp.Close();
	remove(file_path);
	rename((strdup(filename.c_str())), file_path);
	createMetaDataFile(file_path,sorted , sortOrder,runLen);
	Open(file_path);
}

void SortedFile::toggleCurrMode() 
{

	if (READING == currMode) 
		currMode = WRITING;
	else 
		currMode = READING;
	
}

int SortedFile::GetNext(Record &fetchme) 
{

	if (WRITING == currMode) {
		toggleCurrMode();
		mergeInflghtRecs();
	}

	if (pageReadInProg == 0) {
		currFile.GetPage(&currPage, currPageIndex);
		currPageIndex = currPageIndex + 1;
		pageReadInProg = 1;
	}

	if (currPage.GetFirst(&fetchme)) 
		return 1;
	else {

		if (!(currPageIndex > currFile.GetLength() - 2)) {
			currFile.GetPage(&currPage, currPageIndex++);
			pageReadInProg++;
			currPage.GetFirst(&fetchme);
			return 1;
		} else 	return 0;
	}
}

int SortedFile::getRecordWithSort(Record &fetchme, CNF &cnf, Record &literal) 
{
	while (1) {
		if (currPage.GetFirst(&fetchme) == 1) {
			ComparisonEngine ceng;
			if (ceng.Compare(&literal, query, &fetchme, sortOrder)
					== 0) {
				if (ceng.Compare(&fetchme, &literal, &cnf))
					return 1;
			}
			else return 0;
		}

		else {

			currPageIndex++;
			if (currPageIndex < currFile.GetLength() - 1)
				currFile.GetPage(&currPage, currPageIndex);
			else return 0;
		}
	}
}

int SortedFile::getRecordWithoutSort(Record &fetchme) 
{
	if (currPage.GetFirst(&fetchme) == 1) 
		return 1;

	else {
		currPageIndex++;
		if (currPageIndex < currFile.GetLength() - 1) {
			currFile.GetPage(&currPage, currPageIndex);
			if(currPage.GetFirst(&fetchme) == 1)
				return 1;
			else
				return 0;
		}
		else return 0;
	}
}

int SortedFile::getRecordWithoutSort(Record &fetchme, CNF &cnf, Record &literal) {
	while (true) {
		if (currPage.GetFirst(&fetchme) == 1)
		{
			ComparisonEngine comp;
			if (comp.Compare(&fetchme, &literal, &cnf))
				return 1;
		} else
		{
			currPageIndex++;
			if (currPageIndex < currFile.GetLength() - 1)
				currFile.GetPage(&currPage, currPageIndex);
			else return 0;
		}
	}
}

int SortedFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{

	if (WRITING == currMode) {
		isQueryDoneAtleastOnce = 0;
		hasSortOrder = 1;
		toggleCurrMode();
		mergeInflghtRecs();

	}
	ComparisonEngine comp;
	if(hasSortOrder) {
		if(!found) {
			query = new OrderMaker();
			if (cnf.GetSortOrderAttsFromCNF(*sortOrder, *query, literalOrder) > 0) {
				if (!BinarySearch(fetchme, cnf, literal)) {
					return 0;
				}
				else {
					found = 1;
					do {
						if (comp.Compare(&fetchme, query, &literal, &literalOrder) > 0) {
							return 0;
						}
						if(comp.Compare (&fetchme, &literal, &cnf))
							return 1;
					}while(GetNext(fetchme));
				}

				return 0;
			}

			else {
				found = 1;
				while(GetNext(fetchme)) {
					if(comp.Compare (&fetchme, &literal, &cnf))
						return 1;
				}
				return 0;
			}
		}

		else if(1 == found) {
			while(GetNext(fetchme)) {
				if(comp.Compare (&fetchme, &literal, &cnf))
					return 1;
			}
			return 0;
		}
	}
	else if(1 == found) {
		while(GetNext(fetchme)) {
			if(comp.Compare (&fetchme, &literal, &cnf))
				return 1;
		}
		return 0;
	}
	return 0;
}

int SortedFile::BinarySearch(Record& fetchme, CNF &cnf, Record &literal)
{
	int low = 0;
	int mid = 0;
	int high = currFile.GetLength();
	ComparisonEngine comp;

	while(1) {
		if ((high - low) > 2) {
			mid = (low + high) / 2;
			if ( mid % 2 != 0) {
				mid += 1;
			}
			currPage.EmptyItOut();
			currFile.GetPage(&currPage, mid);
			currPage.GetFirst(&fetchme);

			if (comp.Compare(&fetchme, query, &literal, &literalOrder) >= 0) {
				high = mid;
			}
			else {
				low = mid;
			}
		}
		else {
			break;
		}
	}
	currPage.EmptyItOut();
	currFile.GetPage(&currPage, low);
	int ctr = 0;
	while(1) {
		if(!GetNext(fetchme)) {
			return 0;
		}
		else {
			if (comp.Compare(&fetchme, query, &literal, &literalOrder) == 0) {
				return 1;
			}
			else
				continue;
		}
	}
}
