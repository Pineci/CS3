#include "polygon.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>

// Make square at (+/-1, +/-1)
VectorList *make_square() {
    VectorList *sq = vec_list_init(4);
    vec_list_add(sq, (Vector){1,  1});
    vec_list_add(sq, (Vector){-1, 1});
    vec_list_add(sq, (Vector){-1, -1});
    vec_list_add(sq, (Vector){1,  -1});
    return sq;
}
