#ifndef SORTED_FILE_H
#define SORTED_FILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "GenDBFile.h"
#include "Defs.h"
#include "Pipe.h"
#include "BigQ.h"
#include <fstream>

#define IN_OUT_PIPE_BUFF_SIZE 100

enum SortedFileMode
{
  READING,
  WRITING
};

class SortedFile:public GenDBFile {
	int       counter;
	int       flag =0;
	int       pageReadInProg; 
	int       currPageIndex; 
	FILE      *dbFile;       
	FILE      *tblFile;      
	int       numRecordsRead; 
	int       numPagesRead;   
	Record    *currRecord;    
	Page      currPage;       
	File      currFile;       
	fstream   checkIsFileOpen;
	char      *file_path;
	BigQ      *bigQ;
	Pipe      *inPipe;
	Pipe      *outPipe;
	OrderMaker *sortOrder;
	int       runLen;

	OrderMaker *query;
	OrderMaker literalOrder;
	SortedFileMode  currMode;
	int found;
	void toggleCurrMode();
	void mergeInflghtRecs();
	int BinarySearch(Record& fetchme,CNF &cnf,Record &literal);
	int hasSortOrder;
	int isQueryDoneAtleastOnce;
	int getRecordWithoutSort (Record &fetchme, CNF &cnf, Record &literal);
	int getRecordWithoutSort (Record &fetchme);
	int getRecordWithSort(Record &fetchme, CNF &cnf, Record &literal);
	void createMetaDataFile(char *fpath, fType file_type, OrderMaker *sortOrder,int runLen);

	public:
	SortedFile ();
	int Create (char *fpath, fType file_type, void *startup);
	int Open (char *fpath);
	int Close ();
	void Load (Schema &myschema, char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	void *setupBq(void *ptr);
	void start();
	void AppendSequential(Record &appendme);

};
#endif
