#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "shape.h"

const double PACMAN_RADIUS = 100;
const double PELLET_RADIUS = 10;
const double COLLISION_THRESHOLD = 5.0;
const int CIRCLE_POINTS = 50;
const double SPAWN_RATE = 2.0;
const int INIT_PELLETS = 7;
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double PACMAN_MOUTH_ANGLE = M_PI/3.0;
const Vector PACMAN_INITIAL_VELOCITY = (Vector){0, 200.0};
const RGBColor SHAPE_COLORS = (RGBColor){1.0, 0.75, 0.0};
const double ACCELERATION = 3.0;

Scene *my_scene;

Body *create_pellet(double radius, int num_vertices, Vector center){
    List *l = shape_circle(radius, num_vertices);
    Body *b = body_init(l, 1.0, SHAPE_COLORS);
    body_set_centroid(b, center);
    return b;

}

Body *create_pacman(double radius, int num_vertices, Vector center){
    List *l = shape_partial_circle(radius, num_vertices, (1 - PACMAN_MOUTH_ANGLE/(2 * M_PI)));
    Vector *mouth = malloc(sizeof(Vector));
    *mouth = (Vector){0, 0};
    list_add(l, mouth);
    polygon_rotate(l, PACMAN_MOUTH_ANGLE/2, polygon_centroid(l));
    Body *b = body_init(l, 1.0, SHAPE_COLORS);
    body_set_centroid(b, center);
    body_set_velocity(b, PACMAN_INITIAL_VELOCITY);
    return b;
}

void wrap_screen(Body *body, Vector min, Vector max){
    Vector centroid = body_get_centroid(body);
    if(max.x <= centroid.x){
        Vector new_centroid = (Vector){min.x, centroid.y};
        body_set_centroid(body, new_centroid);
    }
    if(min.x >= centroid.x){
        Vector new_centroid = (Vector){max.x, centroid.y};
        body_set_centroid(body, new_centroid);
    }
    if(max.y <= centroid.y){
        Vector new_centroid = (Vector){centroid.x, min.y};
        body_set_centroid(body, new_centroid);
    }
    if(min.y >= centroid.y){
        Vector new_centroid = (Vector){centroid.x, max.y};
        body_set_centroid(body, new_centroid);
    }
}

bool can_pacman_eat(Body *pacman, Body *pellet){
    bool collided = false;
    Vector pellet_centroid = body_get_centroid(pellet);
    double centroid_distance = vec_distance(body_get_centroid(pacman), pellet_centroid);

    if (centroid_distance <= PACMAN_RADIUS + PELLET_RADIUS){

        List *points = body_get_shape(pacman);

        for(int i = 0; i < list_size(points); i++){
            Vector v = *(Vector *)(list_get(points, i));
            double distance = vec_distance(v, pellet_centroid);
            if(distance < COLLISION_THRESHOLD){
                collided = true;
            }
        }
        list_free(points);
    }
    return collided;
}

Vector gen_random_location(Vector min, Vector max){
    double rand_x = fmod((double)rand(), max.x - min.x) + min.x;
    double rand_y = fmod((double)rand(), max.y - min.y) + min.y;
    return (Vector){rand_x, rand_y};
}

void add_pellet(Scene *scene, Vector min_corn, Vector max_corn){
    Vector loc = gen_random_location(min_corn, max_corn);
    Body *pellet = create_pellet(PELLET_RADIUS, CIRCLE_POINTS, loc);
    scene_add_body(scene, pellet);
}

void gen_scene_bodies(Scene *scene, Vector min_corn, Vector max_corn){
    srand(time(NULL));
    Body *pac_man = create_pacman(PACMAN_RADIUS, CIRCLE_POINTS, (Vector){0, 0});

    scene_add_body(scene, pac_man);

    for(int i = 0; i < INIT_PELLETS; i++){
        add_pellet(scene, min_corn, max_corn);
    }
}

void on_key(char key, KeyEventType type, double held_time, void *data){
    Body* pacman = scene_get_body(my_scene, 0);
    double angle = 0.0;
    switch(key){
        case LEFT_ARROW:
        angle = M_PI/2;
        break;
        case RIGHT_ARROW:
        angle = -M_PI/2;
        break;
        case UP_ARROW:
        angle = 0.0;
        break;
        case DOWN_ARROW:
        angle = M_PI;
        break;
    }
    if(type == KEY_PRESSED){
        body_set_rotation(pacman, angle);
        Vector velocity = vec_multiply(1 + ACCELERATION * held_time, vec_rotate(PACMAN_INITIAL_VELOCITY, angle));
        body_set_velocity(pacman, velocity);
    }
    else{
        Vector velocity = vec_rotate(PACMAN_INITIAL_VELOCITY, angle);
        body_set_velocity(pacman, velocity);
    }
}

void check_collision(Scene *my_scene, Body *pacman){
    int i = 1;
    while(i < scene_bodies(my_scene)){
        if(can_pacman_eat(pacman, scene_get_body(my_scene, i))){
            scene_remove_body(my_scene, i);
        }
        else {
            i++;
        }
    }
}

int main(int argc, const char* argv[]){
    Vector min_corn = {.x = -1 * WINDOW_WIDTH, .y = -1 * WINDOW_HEIGHT};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    sdl_init(min_corn, max_corn);
    double time_since_last_spawn = 0;

    my_scene = scene_init();
    gen_scene_bodies(my_scene, min_corn, max_corn);
    sdl_on_key(on_key, my_scene);

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        time_since_last_spawn += dt;

        Body *pacman = scene_get_body(my_scene, 0);
        scene_tick(my_scene, dt);
        wrap_screen(pacman, min_corn, max_corn);
        sdl_render_scene(my_scene);

        check_collision(my_scene, pacman);

        if(time_since_last_spawn > SPAWN_RATE){
            time_since_last_spawn = 0;
            add_pellet(my_scene, min_corn, max_corn);
        }
    }
    scene_free(my_scene);
    return 0;
}
