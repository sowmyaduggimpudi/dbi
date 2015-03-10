#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "GenDBFile.h"
#include <fstream>

class HeapFile:public GenDBFile {
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

	public:
	HeapFile ();
	int Create (char *fpath, fType file_type, void *startup);
	int Open (char *fpath);
	int Close ();
	void Load (Schema &myschema, char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	void AppendSequential(Record &appendme);
};
#endif
