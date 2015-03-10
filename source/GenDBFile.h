#ifndef GEN_DB_FILE_H
#define GEN_DB_FILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include <fstream>

class GenDBFile {
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
    GenDBFile ();
    virtual ~GenDBFile() =0;

    virtual int Create (char *fpath, fType file_type, void *startup)=0;
    virtual int Open (char *fpath)=0;
    virtual int Close ()=0;

    virtual void Load (Schema &myschema, char *loadpath)=0;

    virtual void MoveFirst ()=0;
    virtual void Add (Record &addme)=0;
    virtual int GetNext (Record &fetchme)=0;
    virtual int GetNext (Record &fetchme, CNF &cnf, Record &literal)=0;
    virtual void AppendSequential(Record &appendme)=0;

};
#endif
