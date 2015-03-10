#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "HeapDBFile.h"
#include "GenDBFile.h"
#include "Defs.h"

using namespace std;
#include <fstream>
#include <iostream>
#include <string.h>

HeapFile::HeapFile () 
{
  pageReadInProg = 0;
  currPageIndex = 0;
}

int HeapFile::Create (char *f_path, fType f_type, void *startup) 
{
	checkIsFileOpen.open(f_path,ios_base::out | ios_base::in);
	if(checkIsFileOpen.is_open()) {
		currFile.Open(1, f_path);
	}

	else 
		currFile.Open(0, f_path);

	char path[100];
	sprintf(path, "%s.metadata", f_path);
	FILE *fptr = fopen(path, "wr");
	fwrite((int *)&f_type,1 , sizeof(f_type), fptr);
	fclose(fptr);
	return 1;
}

void HeapFile::Load (Schema &f_schema, char *loadpath)
{
	tblFile = fopen(loadpath, "rb");
	if(!tblFile) {
		cout << "\nFailed to Open the file: %s" << loadpath;
		return;
	}

	currRecord = new (std::nothrow) Record;
	int appendStatus = 1;
	while(currRecord->SuckNextRecord(&f_schema, tblFile)) {
		appendStatus = currPage.Append(currRecord);
		if(0 == appendStatus) {
			currFile.AddPage(&currPage, currFile.GetLength());
			appendStatus = 1;
			currPage.EmptyItOut();
			currPage.Append(currRecord);
		}
	}
	currFile.AddPage(&currPage, currFile.GetLength());
	delete currRecord;
}

int HeapFile::Open (char *f_path) 
{
	checkIsFileOpen.open(f_path,ios_base::out | ios_base::in);
	if(checkIsFileOpen.is_open()) {
		currFile.Open(1, f_path);
	}

	else 
		currFile.Open(0, f_path);

	return 1;
}

int HeapFile::Close ()
{
  currFile.Close();
}

void HeapFile::MoveFirst () 
{
	if(currFile.GetLength()==0){
		cout << "Bad operation , File Empty" ;
	}
	else{
		currPageIndex = 0;
		currFile.MoveFirst();
		currFile.GetPage(&currPage, currPageIndex++);
		pageReadInProg = 1;
	}
}

void HeapFile::AppendSequential(Record &appendme)
{
 Add(appendme);
}

void HeapFile::Add (Record &rec) 
{
	if(pageReadInProg==0) {
		currFile.AddPage(&currPage, currFile.GetLength());
		pageReadInProg = 1;
	}

	if(currFile.GetLength()>0) 
	{
		currFile.GetPage(&currPage,currFile.GetLength()-2);
		currPageIndex = currFile.GetLength()-2;
	}

	if(!currPage.Append(&rec)) 
	{
		currPage.EmptyItOut();
		currPage.Append(&rec);
		currPageIndex++;
	}

	currFile.AddPage(&currPage,currPageIndex);
}

int HeapFile::GetNext (Record &fetchme)
{

	if(pageReadInProg==0) {
		currFile.GetPage(&currPage, currPageIndex);
		currPageIndex= currPageIndex +1;
		pageReadInProg = 1;
	}

	if(currPage.GetFirst(&fetchme) ) 
		return 1;

	else{

		if(!(currPageIndex > currFile.GetLength()-2))
		{
			currFile.GetPage(&currPage, currPageIndex++);
			pageReadInProg++;
			currPage.GetFirst(&fetchme);
			return 1;
		}
		else{
			return 0;
		}
	}
}

int HeapFile::GetNext (Record &fetchme, CNF &myComparison, Record &literal)
{
	ComparisonEngine comp;
	while(GetNext(fetchme)){
		if(comp.Compare (&fetchme, &literal, &myComparison)==1)
			return 1;
	}

	return 0;
}
