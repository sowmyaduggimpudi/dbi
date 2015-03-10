
Compile and Run Instructions:

Part- I:

Compile and Run instructions:

1.Navigate to the dbi folder.
	cd dbi
2. To compile the test1, In the same folder
	make test1
3. To run the the test1, In the same folder execute the following.
	./bin/test1

The following will be displayed, when you run test1. 
(The test result we obtained on running with the nation table) 

Select test option:
	1. sort
	2. sort + display
	3. sort + write 
--- a) enter "1"

Select dbfile to use:
	1. nation
	2. region
	3. customer
	4. part
	5. partsupp
	6. orders
	7. lineitem
---- b) enter "1"

Specify runlength:
--- c) enter "1"

Specify sort ordering (when done press ctrl-D):
--- d) (n_name)

RESULT:: 
 	producer: opened DBFile nation.bin
	producer: inserted 25 recs into the pipe.
	consumer: removed 25 recs form the pipe.
	consumer: 25 recs out of 25 recs in sorted order.

------Similarly try for other options ( sort+display and sort+write) for other tables-----


Part-II:

Compile and Run instructions:

1.Navigate to the dbi folder.
	cd dbi
2. To compile the test1, In the same folder
	make test2
3. To run the the test2, In the same folder execute the following.
	./bin/test2

The following will be displayed, when you run test2. 
(The test result we obtained on running with the nation table) 

Select test option: 
	1. create sorted dbfile
	2. scan a dbfile
	3. run some query
--- a) enter "1"

Select table:
	1. nation
	2. region
	3. customer
	4. part
	5. partsupp
	6. orders
	7. lineitem	
--- b) enter "1"

Specify sort ordering (when done press ctrl-D):
--- c) (n_name)

Specify runlength:
--- c) enter "1"

DISPLAYED:
	output to dbfile: nation/bin
	input from file: /cise/tmp/dbi_sp11/DATA/10M/nation.tbl

Select option for: nation.bin
	1. add a few (1 to 1k recs)
	2. add a lot (1k to 1e+06 recs)
	3. run some query
--- d) enter "1"

RESULT:
	added 25 recs.. so far 25
	create finished .. 25 recs inserted

--Similarly try for other options ( scan a dbfile and run some query) for other tables---







 

