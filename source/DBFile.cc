#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <unistd.h>


DBFile::DBFile ()
	:pNum(0), is_pDirty(false), mfile("")
{
}

int DBFile::Create (char *f_path, fType f_type, void *startup) {
	if (mfile.empty()) {
		mfile.append(f_path);
		mfile.append(".metadata");
	}

	file.Open(0, f_path);
	fileType = f_type;
	if(fileType != heap) {
		cerr << __func__ << "The file type is not heap\n";
		return 0;
	}

	return 1;
}

void DBFile::Load (Schema &f_schema, char *loadpath) {	
	FILE *tblFile = fopen(loadpath, "r");	
	Record temp;

	while (temp.SuckNextRecord (&f_schema, tblFile) == 1)
		Add(temp);
}

int DBFile::Open (char *f_path) {
	int f_type;
	struct stat buff;

	if (mfile.empty()) {
		mfile.append(f_path);
		mfile.append(".metadata");
	}

	if (stat(mfile.c_str(), &buff) != 0) {
		cerr << __func__ << "meta file doesn't exist\n";
		return 0;
	}

	ifstream mfile_ifs(mfile.c_str());

	if (!mfile_ifs.is_open()) {
		cerr << __func__ << "metadata file failed\n";
		return 0;
	}

	mfile_ifs >> f_type;
	fileType = (fType) f_type;
	mfile_ifs >> pNum ;
	mfile_ifs >> numPages;
	file.Open(numPages, f_path);
	mfile_ifs.close();

	return 1;
}

void DBFile::MoveFirst () {
	if (pNum == 0)
		return;

	if (is_pDirty)
		file.AddPage(&currPage, pNum);	
	pNum = 0;
	file.GetPage(&currPage, pNum);
}

int DBFile::Close () {
	if (is_pDirty) {
		file.AddPage(&currPage, pNum);	
		pNum++;
	}
	ofstream mfile_ofs(mfile.c_str());

	if (!mfile_ofs.is_open()) {
		cerr << __func__<<": metadata file failed\n";
		return 0;
	}

	numPages = file.Close();
	mfile_ofs << fileType << endl;
	mfile_ofs << pNum << endl;
	mfile_ofs << numPages << endl;
	mfile_ofs.close();
	
	return numPages;
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
	if (currPage.GetFirst(&fetchme)) {
		return 1;
	} else {
		if (pNum + 1 >= (file.GetLength()-1))
			return 0;
		pNum++;
		file.GetPage(&currPage, pNum);
		currPage.GetFirst(&fetchme);
		return 1;
	}
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	ComparisonEngine ceng;
	while (GetNext(fetchme))
		if (ceng.Compare(&fetchme, &literal, &cnf))
			return 1;
	return 0;
}
