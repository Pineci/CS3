#include "forces.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "shape.h"

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const int NUM_CIRCLES = 50;
const int CIRCLE_POINTS = 50;
const double MASS = 3;
const double K = 16;
const double GAMMA = .8;

double *rainbow_maker(int i, double *prev_color){
  double delta_color = 6 / (double) NUM_CIRCLES;
  double flip = prev_color[3];
  int color_index = prev_color[4];
  if ((int)fmod(i, NUM_CIRCLES / 6) == 0 && i != 0){
    if (flip == -1){
      prev_color[color_index] = 0;
    }
    else{
      prev_color[color_index] = 1;
    }
    prev_color[3] *= -1;
    prev_color[4] = fmod(color_index + 2, 3);
    flip = prev_color[3];
    color_index = prev_color[4];
  }
  prev_color[color_index] += (delta_color * flip);
  return prev_color;
}

Scene *scene_mass_maker(){
  Scene *scene = scene_init();
  double radius = 2 * WINDOW_WIDTH/(2 * NUM_CIRCLES);
  Vector initial_pos = {radius, radius};
  Vector displacement = {radius, radius / (WINDOW_WIDTH / WINDOW_HEIGHT)};
  // Order of prev_color is: r, g, b, flip, color_index.
  double prev_color[5] = {1, 0, 1, -1, 2};

  for (int i = 0; i < NUM_CIRCLES; i++){
    Body *circle = body_init(shape_circle(radius, CIRCLE_POINTS), MASS,
        (RGBColor){prev_color[0], prev_color[1], prev_color[2]});

    memmove(prev_color, rainbow_maker(i, prev_color), sizeof(int *));
    Vector new_centroid = vec_add(initial_pos, vec_multiply(2 * i, displacement)); // circles diagonally up

    body_set_centroid(circle, new_centroid);
    scene_add_body(scene, circle);
  }
  return scene;
}

Scene *scene_anchor_maker(){
  Scene *scene = scene_init();
  double radius = 2 * WINDOW_WIDTH/(2 * NUM_CIRCLES);
  Vector initial_pos = {radius, WINDOW_HEIGHT};
  Vector position = {2 * radius, 0};

  for (int i = 0; i < NUM_CIRCLES; i++){
    Body *circle = body_init(shape_circle(radius, CIRCLE_POINTS), MASS, (RGBColor){1, 1, 1});
    Vector new_centroid = vec_add(initial_pos, vec_multiply(i, position)); // circles in middle
    body_set_centroid(circle, new_centroid);
    scene_add_body(scene, circle);
  }
  return scene;
}

int main(int argc, const char* argv[]){
    Vector min_corn = {.x = 0, .y = 0};
    Vector max_corn = {.x = 2 * WINDOW_WIDTH, .y = 2 * WINDOW_HEIGHT};
    sdl_init(min_corn, max_corn);

    Scene *mass_scene = scene_mass_maker();
    Scene *anchor_scene = scene_anchor_maker();

    for(int i = 0; i < NUM_CIRCLES; i++){
      create_spring(mass_scene, K * (i + 1) / 10, scene_get_body(mass_scene, i), scene_get_body(anchor_scene, i));
      create_drag(mass_scene, GAMMA * (i + 1) / 10, scene_get_body(mass_scene, i));
    }

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        scene_tick(mass_scene, dt);
        sdl_render_scene(mass_scene);
    }
    scene_free(mass_scene);
    scene_free(anchor_scene);
    return 0;
}
