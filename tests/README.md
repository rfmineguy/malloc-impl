# Running Tests
```bash
gcc test_main.c munit.c test_util.c ../mymalloc.c -o test -DTEST -DMUNIT_TEST_NAME_LEN=60
./test
```
