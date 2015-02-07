#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"


DBFile::DBFile () {
	pNum = 0;
	is_pDirty = false;
}

int DBFile::Create (char *f_path, fType f_type, void *startup) {
	string mfile(f_path);

	mfile.append(".metadata");
	ofstream mfile_ofs(mfile.c_str());

	if (!mfile_ofs.is_open()) {
		cerr << "Open metadata file failed\n";
		return 0;
	}

	file.Open(0, f_path);
	mfile_ofs << f_type;
	mfile_ofs.close();
}

void DBFile::Load (Schema &f_schema, char *loadpath) {	
	FILE *tblFile = fopen(loadpath, "r");	
	Record temp;

	while (temp.SuckNextRecord (&f_schema, tblFile) == 1)
		Add(temp);
}

int DBFile::Open (char *f_path) {
}

void DBFile::MoveFirst () {
}

int DBFile::Close () {
	if (is_pDirty)
		file.AddPage(&currPage, pNum);	
	 return file.Close();
}

void DBFile::Add (Record &rec) {
	is_pDirty = true;
	if (!(currPage.Append(&rec))) {
		file.AddPage(&currPage, pNum);	
		pNum++;
		currPage.EmptyItOut();
		currPage.Append(&rec);
	}
}

int DBFile::GetNext (Record &fetchme) {
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
}
