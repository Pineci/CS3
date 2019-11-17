#include "list.h"
#include "test_util.h"
#include <assert.h>
#include <stdlib.h>

void test_list_size0() {
    List *l = list_init(0, free);
    assert(list_size(l) == 0);
    list_free(l);
}

void test_list_size1() {
    List *l = list_init(1, free);
    assert(list_size(l) == 0);
    // Add
    Vector *origin = malloc(sizeof(Vector));
    *origin = (Vector){0, 0};
    list_add(l, (void *)origin);
    assert(list_size(l) == 1);
    // Remove
    assert(vec_equal(*(Vector *)list_remove(l, list_size(l) - 1), (Vector){0, 0}));
    assert(list_size(l) == 0);


    // Set
    /*
    Vector *random = malloc(sizeof(Vector));
    *random = (Vector){1, 2};
    list_set(l, 0, random);
    assert(list_size(l) == 1);
    assert(vec_equal(*(Vector *)list_get(l,0), (Vector){1, 2}));
    */
    list_free(l);
    free(origin);
    //free(random);
}

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_list_size0)
    DO_TEST(test_list_size1)

    puts("list_test PASS");
    return 0;
}
