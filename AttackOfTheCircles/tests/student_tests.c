#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const double G = 6.67408e-11;
const double DT = 1e-12;
const int STEPS = 100;

double distance_vec(Vector v1, Vector v2){
  Vector vec_diff = vec_subtract(v1, v2);
  return sqrt(vec_dot(vec_diff, vec_diff));
}

double get_distance_between_centroids(Body *body1, Body *body2){
  return distance_vec(body_get_centroid(body1), body_get_centroid(body2));
}

List *make_shape() {
  List *shape = list_init(4, free);
  Vector *v = malloc(sizeof(*v));
  *v = (Vector) {-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (Vector) {+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (Vector) {+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (Vector) {-1, +1};
  list_add(shape, v);
  return shape;
}

// Gets new velocity of body1 due to body2 after time DT
Vector get_new_velocity_grav(Body *body1, Body *body2, Vector prev_vel_1){
  double acceleration_1 = G * body_get_mass(body2) / pow(
      get_distance_between_centroids(body1, body2), 3);
  Vector direction_1_to_2 = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  Vector delta_v_1 = vec_multiply(acceleration_1 * DT, direction_1_to_2);
  return vec_add(delta_v_1, prev_vel_1);
}

void assert_grav(Scene *scene, Body *body1, Body *body2){
  for (int i = 0; i < STEPS; i++) {
      Vector prev_vel_1 = body_get_velocity(body1);
      Vector prev_vel_2 = body_get_velocity(body2);
      create_newtonian_gravity(scene, G, body1, body2);

      assert(vec_within(1e-3, body_get_velocity(body1), get_new_velocity_grav(body1, body2, prev_vel_1)));
      assert(vec_within(1e-3, body_get_velocity(body2), get_new_velocity_grav(body2, body1, prev_vel_2)));

      scene_tick(scene, DT);
  }
  scene_free(scene);
}

Scene *scene_maker(Vector distance, double m1, double m2){
  Scene *scene = scene_init();
  Body *mass1 = body_init(make_shape(), m1, (RGBColor) {0, 0, 0});
  scene_add_body(scene, mass1);
  Body *mass2 = body_init(make_shape(), m2, (RGBColor) {0, 0, 0});
  body_set_centroid(mass2, distance);
  scene_add_body(scene, mass2);
  return scene;
}

// Tests the gravity between different pairs of bodies. Bodies that are close
// should not have a force
void test_newtonian_gravity(){

  //Case where two bodies of the same size are relatively close
  Scene *scene = scene_maker((Vector){30, 50}, 3, 3);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where there are different bodies that are relatively close
  scene = scene_maker((Vector){30, 50}, 3, 40);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where the different bodies are far apart
  scene = scene_maker((Vector){400, 340}, 3, 40);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where they are really close
  scene = scene_maker((Vector){3, 5}, 3, 40);
  Body *body0 = scene_get_body(scene, 0);
  Body *body1 = scene_get_body(scene, 1);
  Vector prev_vel_1 = body_get_velocity(body0);
  Vector prev_vel_2 = body_get_velocity(body1);
  create_newtonian_gravity(scene, G, body0, body1);
  assert(vec_within(1e-3, prev_vel_1, body_get_velocity(body0)));
  assert(vec_within(1e-3, prev_vel_2, body_get_velocity(body1)));
  scene_free(scene);
}

// Gets new velocity of mass after time DT
Vector get_new_velocity_spring(Body *mass, Body *anchor, double k, Vector prev_vel){
  double scalar = -k / body_get_mass(mass);
  Vector distance = vec_subtract(body_get_centroid(anchor), body_get_centroid(mass));
  Vector acceleration = vec_multiply(scalar, distance);
  return vec_add(vec_multiply(DT, acceleration), prev_vel);
}

typedef struct {
  Scene *scene;
  double k;
} Spring;


void create_spring_wrapper(Spring *spring){
  create_spring(spring->scene, spring->k, scene_get_body(spring->scene, 0),
      scene_get_body(spring->scene, 1));
}

Spring *spring_init(Scene *scene, double k){
  Spring *spring = malloc(sizeof(Spring));
  spring->scene = scene;
  spring->k = k;
  return spring;
}

void spring_free(Spring *spring){
  scene_free(spring->scene);
  free(spring);
}

// Mass will oscillate according to anchor. This checks the velocity of the mass.
// Anchor is at equilibrium point.
void assert_spring(Scene *scene, double k, Body *mass, Body *anchor){
  for (int i = 0; i < STEPS; i++) {
      Vector prev_vel = body_get_velocity(mass);
      create_spring(scene, k, mass, anchor);
      assert(vec_within(1e-3, body_get_velocity(mass), get_new_velocity_spring(mass, anchor, k, prev_vel)));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_spring(){
  // Case 1: Assert fails because negative k.
  double k0 = -234823905234682905;
  Scene *scene = scene_maker((Vector){1, 1}, 3, 3);
  Spring *spring = spring_init(scene, k0);
  test_assert_fail((ForceCreator) create_spring_wrapper, (void *)spring);
  spring_free(spring);

  // Case 2: Assert fails because zero k.
  double k1 = 0;
  scene = scene_maker((Vector){1, 1}, 3, 3);
  spring = spring_init(scene, k1);
  test_assert_fail((ForceCreator) create_spring_wrapper, (void *)spring);
  spring_free(spring);

  // Case 3: Very weak spring.
  double k2 = 1;
  scene = scene_maker((Vector){50, 100}, 10, 20);
  assert_spring(scene, k2, scene_get_body(scene, 0), scene_get_body(scene, 1));

  // Case 4: Strong spring, close distance.
  double k3 = 50;
  scene = scene_maker((Vector){25, 50}, 10, 20);
  assert_spring(scene, k3, scene_get_body(scene, 0), scene_get_body(scene, 1));

  // Use k3 with 2 different distances
  scene = scene_maker((Vector){300, 500}, 3, 3);
  assert_spring(scene, k3, scene_get_body(scene, 0), scene_get_body(scene, 1));
}

void assert_drag(Scene *scene, double gamma, Vector velocity){
  Body *body = scene_get_body(scene, 0);
  body_set_velocity(body, velocity);

  for (int i = 0; i < STEPS; i++) {
    Vector prev_vel = body_get_velocity(body);
    create_drag(scene, gamma, body);
    Vector force = vec_multiply(-1 * gamma, prev_vel);
    Vector new_vel = vec_multiply(DT / body_get_mass(body), force);
    new_vel = vec_add(new_vel, prev_vel);
    scene_tick(scene, DT);
    assert(vec_within(1e-3, body_get_velocity(body), new_vel));
  }
}

void test_drag(){
  Scene *scene = scene_maker((Vector){50, 100}, 3, 3);
  Vector velocity = {0, 400};
  double gamma = 10;

  // Case 1: Velocity 0, no drag.
  assert_drag(scene, gamma, VEC_ZERO);

  // Case 2: North velocity
  assert_drag(scene, gamma, velocity);

  // Case 3: Northeast velocity
  assert_drag(scene, gamma / 10, vec_rotate(velocity, - M_PI / 4));

  // Case 4: Southeast velocity
  assert_drag(scene, gamma * 3, vec_rotate(velocity, - M_PI / 2));

  // Case 5: Southwest velocity
  assert_drag(scene, gamma * 10, vec_rotate(velocity, - M_PI / 2));

  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
      read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_newtonian_gravity)
  DO_TEST(test_spring)
  DO_TEST(test_drag)

  puts("forces_test PASS");
  return 0;
}
