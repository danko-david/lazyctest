
# LazyCtest - lightweight C testing utility

## Into
I found other test frameworks complicated because the rest requires to create
one more unit where you can register functions, create suites, etc.  

I just needed something simple like [Novaprova](https://github.com/novaprova/novaprova),
but after a year of omission, when i get a newer version doesn't worked anymore.
There was a problem with [symbol relocation](https://github.com/novaprova/novaprova/issues/30)
I can't find the root of the cause in the heavy code, so i created the simplified
version of it. 

Of course simplicity came some drawback: There's not suites, parametric tests,
function mocking or test fixtures.
Now the only thing it does just loads the given shared libraries run the tests 
then outputs a lct\_result\_*.junit.xml result file.

## How it works?

* The point is to get all the test codes into one address space
	(lazyctester.c: open\_dl\_file(...)),
* Collect the function symbols start with test\_, or it's C++ signature void XX::test*(void).
	(lazyctester.c: symbols(collect\_all\_function\_symbol, ...))  
* Creates as many threads as we specified as concurrency, every thread connects
	to the queue where we collected the symbols.
* Every thread runs the same function (lazyctester.c: thread\_function):
	takes element from the queue, creates a SHM area, fork(), the child executes
	the test and puts the result to the SHM area (lct\_testing.c: do\_test\_in\_child).
	The parent waits for the child. When all child are done and all result collected
	the result written into file. (lazyctester.c: write\_test\_result\_junit(LCT\_TASK\_QUEUES, ...);)

## How to write tests?

For examples see: [./src/c/tests.c](./src/c/tests.c)

Because there's no definition file where you can specify the desired behavior of the 
test function (eg.: normal return, have to fail an assertion, should die by sigsegv,
must exit with a specified status) you have to set these at the beginning of the
test function. 

For this option see [lazyctest.h](./src/c/lazyctest.h): lct\_test\_set\_excepted\_*
For assertion macros see [lazyctest.h](./src/c/lazyctest.h): TEST\_ASSERT\_*

## How to use?

* Download or refresh the dependencies by invoking ./scripts/get\_c\_deps.sh
* Compile the lct command by invoking ./src/c/build.sh
* Use the lazyctest.h in your test file, write the test and compile a shared
	library that results a single so file (for example see: ./src/c/build\_tests.sh)
* Run the tests: `lct my_tests.so`.
* Evaluate the `lct\_result_*.junit.xml` result files.

