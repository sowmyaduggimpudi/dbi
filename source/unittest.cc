
#include <limits.h>
#include "gtest/gtest.h"
#include "DBFile.h"
#include <iostream>
using namespace std;
#include <sys/stat.h>

static char *tbl_fpath = "test_data/lineitem.tbl";
static char *bin_fpath = "test_data/unittest.bin";
static char *binm_fpath = "test_data/unittest.bin.metadata";
static Schema mySchema ("catalog", "lineitem");
static Record myRec;

#define CLEAR_TEST_FILES remove("test_data/*.bin*")
TEST(DBFileUnitTest, check_create) {
	struct stat buf;
	static DBFile dbfile;
	EXPECT_EQ(true, dbfile.Create(bin_fpath, heap, NULL));
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
	remove(bin_fpath);
	remove(binm_fpath);
}

TEST(DBFileUnitTest, check_bin_file) {
	struct stat buf;
	static DBFile dbfile;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
       	EXPECT_EQ(0, stat(bin_fpath, &buf));
	remove(bin_fpath);
	remove(binm_fpath);
}

TEST(DBFileUnitTest, check_bin_metafile) {
	struct stat buf;
	static DBFile dbfile;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
       	EXPECT_EQ(0, stat(binm_fpath, &buf));
	remove(bin_fpath);
	remove(binm_fpath);
}

TEST(DBfileUnitTest, check_open_db_file_FAIL) {
	struct stat buff;
	static DBFile dbfile;
	EXPECT_EQ(0, dbfile.Open(bin_fpath));
	remove(bin_fpath);
	remove(binm_fpath);
}

TEST(DBFileUnitTest, check_open_db_file_PASS) {
	struct stat buf;
	static DBFile dbfile;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close();
	EXPECT_EQ(1, dbfile.Open(bin_fpath));
	remove(bin_fpath);
	remove(binm_fpath);
}

TEST(DBfileUNitTest, check_close_return) {
	static DBFile dbfile;
	static DBFile dbfile2;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile2.Create("test_data/r.bin", heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile2.Load (mySchema, tbl_fpath);
	EXPECT_EQ(dbfile.Close(), dbfile2.Close());
	remove(bin_fpath);
	remove(binm_fpath);
	remove("test_data/r.bin");
	remove("test_data/r.bin.metadata");
}
