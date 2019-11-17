#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "color.h"
#include "sdl_wrapper.h"
#include "forces.h"
#include "shape.h"

const int CIRCLE_POINTS = 50;
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const int MAX_PLANET_RADIUS = 60;
const int MIN_PLANET_RADIUS = 30;
const int NUM_BODIES = 100;
const double G = 300;

Vector gen_random_location(Vector min, Vector max){
    double rand_x = fmod((double)rand(), max.x - min.x) + min.x;
    double rand_y = fmod((double)rand(), max.y - min.y) + min.y;
    return (Vector){rand_x, rand_y};
}


Body *create_planet(double radius, int num_vertices, Vector center){
    List *l = shape_estrella(radius);
    Body *b = body_init(l, radius * radius, gen_color());
    body_set_centroid(b, center);
    return b;
}

void add_planet(Scene *scene, Vector min_corn, Vector max_corn){
    Vector loc = gen_random_location(min_corn, max_corn);
    double radius = rand() % MAX_PLANET_RADIUS + MIN_PLANET_RADIUS;
    Body *planet = create_planet(radius, CIRCLE_POINTS, loc);
    scene_add_body(scene, planet);
}

void gen_n_bodies(Scene *scene, int n, Vector min_corn, Vector max_corn){
    srand(time(NULL));
    for(int i = 0; i < n; i++){
        add_planet(scene, min_corn, max_corn);
    }
}


int main(int argc, const char* argv[]){
    Vector min_corn = {.x = -1 * WINDOW_WIDTH, .y = -1 * WINDOW_HEIGHT};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};

    sdl_init(min_corn, max_corn);

    Scene *my_scene = scene_init();
    gen_n_bodies(my_scene, NUM_BODIES, min_corn, max_corn);

    for(int i = 0; i < NUM_BODIES; i++){
        Body *this_body = scene_get_body(my_scene, i);
        for(int j = 0; j < NUM_BODIES; j++){
            Body *that_body = scene_get_body(my_scene, j);
            create_newtonian_gravity(my_scene, G, this_body, that_body);
        }
    }

    while(!sdl_is_done()){
        double dt = time_since_last_tick();

        scene_tick(my_scene, dt);
        sdl_render_scene(my_scene);
    }

    scene_free(my_scene);
}
