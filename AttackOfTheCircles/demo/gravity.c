#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "sdl_wrapper.h"
#include "shape.h"
#include "body.h"
#include "list.h"
#include "color.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500
#define LARGE_SCALE 144.0
#define SMALL_SCALE 89.0

const Vector grav = (Vector){0, -1000};
const int rate = 3; //Rate at which shapes spawn in seconds
const int rotation_rate = 6;

List* gen_shapes(int num_shapes, Vector spawn_point){
    srand(time(NULL));
    List* to_ret = list_init(num_shapes + 1, (FreeFunc)body_free);

    int initial_spokes = 2;
    for(int i = num_shapes - 1; i >= 0; i--){
        int num_points = i + initial_spokes;
        int initial_x = 200;
        int initial_y = -200;
        Vector velocity = (Vector){initial_x, initial_y};

        Body* to_add = body_init(shape_star(num_points, 1.0, LARGE_SCALE, SMALL_SCALE), 1.0, gen_color());
        body_set_velocity(to_add, velocity);
        body_set_centroid(to_add, spawn_point);

        list_add(to_ret, to_add);
    }

    return to_ret;
}

void draw_body(List *bodies, int ind){
    Body* body = list_get(bodies, ind);
    List* shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body), false);
    list_free(shape);
}

void body_rotate_velocity(Body* body, double speed, double total_time){
    body_set_rotation(body, speed*total_time);
}

void body_accelerate(Body* body, Vector dir, double dt){
    body_set_velocity(body, vec_add(body_get_velocity(body), vec_multiply(dt, dir)));
}

void body_motion(Body* body, Vector dir, double dt, double speed, double total_time){
    body_tick(body, dt);
    body_accelerate(body, dir, dt);
    body_rotate_velocity(body, speed, total_time);

}

bool body_check_touch(Body* body, double threshold, bool yDirection){
    bool collided = false;
    List* shape = body_get_shape(body);
    int num_vertices = list_size(shape);
    for(int i = 0; i < num_vertices; i++){
      Vector v = *(Vector*)list_get(shape, i);
      if(!yDirection){
        if(v.x > threshold){
          collided = true;
        }
      } else {
          if(v.y < threshold){
              collided = true;
          }
      }
    }
    list_free(shape);
    return collided;
}

bool body_check_offscreen(Body* body, double threshold, bool yDirection){
    List* shape = body_get_shape(body);
    int num_vertices = list_size(shape);
    for(int i = 0; i < num_vertices; i++){
        Vector v = *(Vector*)list_get(shape, i);
        if(!yDirection){
            if(!(v.x > threshold)){
                return false;
            }
        }
        else{
            if(!(v.y > threshold)){
                return false;
            }
        }
    }
    list_free(shape);
    return true;
}

void body_bounce_check(Body* body, Vector max_diff){
    double damp = fmod(rand(), .4) + .6;
    Vector velocity = body_get_velocity(body);
    if(body_check_touch(body, max_diff.y, 1) && velocity.y < 0){
        velocity.y *= -1 * damp;
        body_set_velocity(body, velocity);
    }
}

int main(int argc, const char* argv[]){
    Vector min_corn = {.x = -1 * WINDOW_WIDTH, .y = -1 * WINDOW_HEIGHT};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    Vector upper_left = {min_corn.x, max_corn.y};
    sdl_init(min_corn, max_corn);
    double total_time = 0;

    int num_shapes = 6;
    List *list_of_shapes = gen_shapes(num_shapes, upper_left);
    while(!sdl_is_done()){
        sdl_clear();
        double dt = time_since_last_tick();
        total_time += dt;
        bool left_screen = false;


        size_t size = list_size(list_of_shapes);
        for(int i = size - 1; i > 0; i--){
            if(total_time > rate * (num_shapes - i - 1)){
                Body* curr_body = list_get(list_of_shapes, i);

                body_motion(curr_body, grav, dt, rotation_rate, total_time);
                body_bounce_check(curr_body, min_corn);
                draw_body(list_of_shapes, i);

                if(body_check_offscreen(curr_body, max_corn.x, 0)){
                    left_screen = true;
                }

            }
        }


        if(left_screen){
            body_free(list_remove(list_of_shapes, list_size(list_of_shapes)-1));
        }

        sdl_show();
    }
    list_free(list_of_shapes);
}
