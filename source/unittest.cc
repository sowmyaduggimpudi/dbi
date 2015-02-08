
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
static DBFile dbfile;
TEST(DBFileUnitTest, check_create) {
	struct stat buf;
	EXPECT_EQ(true, dbfile.Create(bin_fpath, heap, NULL));
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
}

TEST(DBFileUnitTest, check_bin_file) {
	struct stat buf;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
	cout<<"FILE EXIT:"<<stat(bin_fpath, &buf) <<endl;
	
       	EXPECT_EQ(0, stat(bin_fpath, &buf));
}

TEST(DBFileUnitTest, check_bin_metafile) {
	struct stat buf;
	dbfile.Create(bin_fpath, heap, NULL);
	dbfile.Load (mySchema, tbl_fpath);
	dbfile.Close ();
	cout<<"FILE EXIT:"<<stat(bin_fpath, &buf) <<endl;

       	EXPECT_EQ(0, stat(binm_fpath, &buf));
}
