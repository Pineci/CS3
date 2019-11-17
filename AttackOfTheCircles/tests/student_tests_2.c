#include "body.h"
#include "color.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "stdio.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

// The CI setup requires that we not import any identifiers which the reference
// solution does not have. Thus, we must define them inline in this file (and
// give them new names so that we can compile this file locally).

const RGBColor COLOR_YELLOW_temp = {.r = 1, .g = 1, .b = 0};

Vector *vec_clone_temp(Vector v) {
  Vector *on_heap = malloc(sizeof(Vector));
  on_heap->x = v.x;
  on_heap->y = v.y;
  return on_heap;
}

double uniform_angle_spacing_temp(size_t num_vertices) {
  return 2 * M_PI / num_vertices;
}

List *polygon_init_regular_temp(size_t num_vertices, double radius) {
  List *polygon = list_init(num_vertices, free);

  double vertex_spacing_angle = uniform_angle_spacing_temp(num_vertices);
  Vector vector = {.x = radius, .y = 0};

  for (size_t i = 0; i < num_vertices; i++) {
    vector = vec_rotate(vector, vertex_spacing_angle);
    list_add(polygon, vec_clone_temp(vector));
  }

  return polygon;
}

// Test that no impulse or force does not change anything when tick
void simple_test(Body *test) {
  for (int i = 1; i < 10; i++) {
    body_tick(test, i);
    assert(body_get_velocity(test).x == VEC_ZERO.x);
    assert(body_get_centroid(test).x == VEC_ZERO.x);
    assert(body_get_velocity(test).y == VEC_ZERO.y);
    assert(body_get_centroid(test).y == VEC_ZERO.y);
  }
  body_free(test);
}

void opposing_forces(Body *test) {
  Vector force = {.x = 90, .y = 50};
  body_add_force(test, force);
  body_add_force(test, vec_negate(force));
  body_tick(test, 3.0);
  assert(body_get_velocity(test).x == VEC_ZERO.x);
  assert(body_get_centroid(test).x == VEC_ZERO.x);
  assert(body_get_velocity(test).y == VEC_ZERO.y);
  assert(body_get_centroid(test).y == VEC_ZERO.y);
  body_free(test);
}

void opposing_force_and_impulse(Body *test) {
  Vector force = {.x = 90, .y = 50};
  body_add_force(test, force);
  body_add_impulse(test, vec_negate(force));
  body_tick(test, 1.0);
  assert(body_get_velocity(test).x == VEC_ZERO.x);
  assert(body_get_centroid(test).x == VEC_ZERO.x);
  assert(body_get_velocity(test).y == VEC_ZERO.y);
  assert(body_get_centroid(test).y == VEC_ZERO.y);
  body_free(test);
}

void constant_acceleration(Body *test) {
  Vector force = {.x = 10, .y = 10};
  Vector velocity_base = {.x = 1, .y = 1};
  for (int i = 1; i < 11; i++) {
    body_add_force(test, force);
    body_tick(test, 1.0);
    assert(body_get_velocity(test).x == vec_multiply(i, velocity_base).x);
    assert(body_get_velocity(test).y == vec_multiply(i, velocity_base).y);
  }
  body_free(test);
}

// Test simple forces
void force_tests() {
  Body *body1 =
      body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  simple_test(body1);
  Body *body2 =
      body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body2, VEC_ZERO);
  opposing_forces(body2);
  Body *body3 =
      body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body3, VEC_ZERO);
  opposing_force_and_impulse(body3);
  Body *body4 =
      body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body4, VEC_ZERO);
  constant_acceleration(body4);
}

void grav_tests() {
  Scene *scene = scene_init();
  Vector trans = {.x = 10, .y = 0};
  Body *body1 =
      body_init(polygon_init_regular_temp(1, 30), 100, COLOR_YELLOW_temp);
  Body *body2 =
      body_init(polygon_init_regular_temp(1, 30), 100, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  body_set_centroid(body2, trans);
  scene_add_body(scene, body1);
  scene_add_body(scene, body2);
  create_newtonian_gravity(scene, 1, body1, body2);
  scene_tick(scene, 1);
  Vector velocity_1 = {.x = 1, .y = 0};

  assert(vec_isclose(body_get_velocity(body1), velocity_1));
  assert(vec_isclose(body_get_velocity(body2), vec_negate(velocity_1)));
  scene_free(scene);
}

void spring_tests() {
  Scene *scene = scene_init();
  Vector trans = {.x = 10, .y = 0};
  Body *body1 =
      body_init(polygon_init_regular_temp(6, 30), 100, COLOR_YELLOW_temp);
  Body *body2 =
      body_init(polygon_init_regular_temp(6, 30), 100, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  body_set_centroid(body2, trans);
  scene_add_body(scene, body1);
  scene_add_body(scene, body2);
  create_spring(scene, 10, body1, body2);
  scene_tick(scene, 1.0);
  Vector velocity_1 = {.x = 1, .y = 0};
  assert(body_get_velocity(body1).x == velocity_1.x);
  assert(body_get_velocity(body2).x == vec_negate(velocity_1).x);
  assert(body_get_velocity(body1).y == velocity_1.y);
  assert(body_get_velocity(body2).y == vec_negate(velocity_1).y);
  scene_free(scene);
}

void drag_tests() {
  Scene *scene = scene_init();
  Body *body =
      body_init(polygon_init_regular_temp(6, 30), 1, COLOR_YELLOW_temp);
  scene_add_body(scene, body);
  Vector velocity = {.x = 100, .y = 0};
  body_set_velocity(body, velocity);
  create_drag(scene, 10, body);
  for (int i = 0; i < 10; i++) {
    scene_tick(scene, 1.0);
    Vector del_v = vec_multiply(-10, velocity);
    velocity = vec_add(velocity, del_v);
    assert(body_get_velocity(body).x == velocity.x);
    assert(body_get_velocity(body).y == velocity.y);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  bool all_tests =
      argc == 1;      /* Run all tests if there are no command-line arguments */
  char testname[100]; /* Read test name from file */
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(force_tests)
  DO_TEST(grav_tests)
  DO_TEST(spring_tests)
  DO_TEST(drag_tests)

  puts("student_test PASS");

  return 0;
}
