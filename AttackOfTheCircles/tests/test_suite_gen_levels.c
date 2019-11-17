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

void test_square_area_centroid() {
    VectorList *sq = make_square();
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), VEC_ZERO));
    vec_list_free(sq);
}

void test_square_translate() {
    VectorList *sq = make_square();
    polygon_translate(sq, (Vector){2, 3});
    assert(vec_equal(vec_list_get(sq, 0), (Vector){3, 4}));
    assert(vec_equal(vec_list_get(sq, 1), (Vector){1, 4}));
    assert(vec_equal(vec_list_get(sq, 2), (Vector){1, 2}));
    assert(vec_equal(vec_list_get(sq, 3), (Vector){3, 2}));
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), (Vector){2, 3}));
    vec_list_free(sq);
}

void test_square_rotate() {
    VectorList *sq = make_square();
    polygon_rotate(sq, 0.25 * M_PI, VEC_ZERO);
    assert(vec_isclose(vec_list_get(sq, 0), (Vector){0, sqrt(2)}));
    assert(vec_isclose(vec_list_get(sq, 1), (Vector){-sqrt(2), 0}));
    assert(vec_isclose(vec_list_get(sq, 2), (Vector){0, -sqrt(2)}));
    assert(vec_isclose(vec_list_get(sq, 3), (Vector){sqrt(2), 0}));
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), VEC_ZERO));
    vec_list_free(sq);
}

// Make 3-4-5 triangle
VectorList *make_triangle() {
    VectorList *tri = vec_list_init(3);
    vec_list_add(tri, (Vector){0, 0});
    vec_list_add(tri, (Vector){4, 0});
    vec_list_add(tri, (Vector){4, 3});
    return tri;
}

void test_triangle_area_centroid() {
    VectorList *tri = make_triangle();
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){8.0 / 3.0, 1}));
    vec_list_free(tri);
}

void test_triangle_translate() {
    VectorList *tri = make_triangle();
    polygon_translate(tri, (Vector){-4, -3});
    assert(vec_equal(vec_list_get(tri, 0), (Vector){-4, -3}));
    assert(vec_equal(vec_list_get(tri, 1), (Vector){0,  -3}));
    assert(vec_equal(vec_list_get(tri, 2), (Vector){0,  0}));
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){-4.0 / 3.0, -2}));
    vec_list_free(tri);
}

void test_triangle_rotate() {
    VectorList *tri = make_triangle();

    // Rotate -acos(4/5) degrees around (4,3)
    polygon_rotate(tri, 2 * M_PI - acos(4.0 / 5.0), (Vector){4, 3});
    assert(vec_isclose(vec_list_get(tri, 0), (Vector){-1,  3}));
    assert(vec_isclose(vec_list_get(tri, 1), (Vector){2.2, 0.6}));
    assert(vec_isclose(vec_list_get(tri, 2), (Vector){4,   3}));
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){26.0 / 15.0, 2.2}));

    vec_list_free(tri);
}

#define CIRC_NPOINTS 1000000
#define CIRC_AREA (CIRC_NPOINTS * sin(2 * M_PI / CIRC_NPOINTS) / 2)

// Circle with many points (stress test)
VectorList *make_big_circ() {
    VectorList *c = vec_list_init(CIRC_NPOINTS);
    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        vec_list_add(c, (Vector){cos(angle), sin(angle)});
    }
    return c;
}

void test_circ_area_centroid() {
    VectorList *c = make_big_circ();
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), VEC_ZERO));
    vec_list_free(c);
}

void test_circ_translate() {
    VectorList *c = make_big_circ();
    Vector translation = {.x = 100, .y = 200};
    polygon_translate(c, translation);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        assert(vec_isclose(
            vec_list_get(c, i),
            vec_add(translation, (Vector){cos(angle), sin(angle)})
        ));
    }
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), translation));

    vec_list_free(c);
}

void test_circ_rotate() {
    // Rotate about the origin at an unusual angle
    const double rot_angle = 0.5;

    VectorList *c = make_big_circ();
    polygon_rotate(c, rot_angle, VEC_ZERO);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        assert(vec_isclose(
            vec_list_get(c, i),
            (Vector){cos(angle + rot_angle), sin(angle + rot_angle)})
        );
    }
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), VEC_ZERO));

    vec_list_free(c);
}

// Weird nonconvex polygon
VectorList *make_weird() {
    VectorList *w = vec_list_init(5);
    vec_list_add(w, (Vector){0,  0});
    vec_list_add(w, (Vector){4,  1});
    vec_list_add(w, (Vector){-2, 1});
    vec_list_add(w, (Vector){-5, 5});
    vec_list_add(w, (Vector){-1, -8});
    return w;
}

void test_weird_area_centroid() {
    VectorList *w = make_weird();
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){-223.0 / 138.0, -51.0 / 46.0}));
    vec_list_free(w);
}

void test_weird_translate() {
    VectorList *w = make_weird();
    polygon_translate(w, (Vector){-10, -20});

    assert(vec_isclose(vec_list_get(w, 0), (Vector){-10, -20}));
    assert(vec_isclose(vec_list_get(w, 1), (Vector){-6,  -19}));
    assert(vec_isclose(vec_list_get(w, 2), (Vector){-12, -19}));
    assert(vec_isclose(vec_list_get(w, 3), (Vector){-15, -15}));
    assert(vec_isclose(vec_list_get(w, 4), (Vector){-11, -28}));
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){-1603.0 / 138.0, -971.0 / 46.0}));

    vec_list_free(w);
}

void test_weird_rotate() {
    VectorList *w = make_weird();
    // Rotate 90 degrees around (0, 2)
    polygon_rotate(w, M_PI / 2, (Vector){0, 2});

    assert(vec_isclose(vec_list_get(w, 0), (Vector){2,  2}));
    assert(vec_isclose(vec_list_get(w, 1), (Vector){1,  6}));
    assert(vec_isclose(vec_list_get(w, 2), (Vector){1,  0}));
    assert(vec_isclose(vec_list_get(w, 3), (Vector){-3, -3}));
    assert(vec_isclose(vec_list_get(w, 4), (Vector){10, 1}));
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){143.0 / 46.0, 53.0 / 138.0}));

    vec_list_free(w);
}

int main(int argc, char *argv[]) {
    // Run all tests? True if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_square_area_centroid)
    DO_TEST(test_square_translate)
    DO_TEST(test_square_rotate)
    DO_TEST(test_triangle_area_centroid)
    DO_TEST(test_triangle_translate)
    DO_TEST(test_triangle_rotate)
    DO_TEST(test_circ_area_centroid)
    DO_TEST(test_circ_translate)
    DO_TEST(test_circ_rotate)
    DO_TEST(test_weird_area_centroid)
    DO_TEST(test_weird_translate)
    DO_TEST(test_weird_rotate)

    puts("polygon_test PASS");

    return 0;
}
