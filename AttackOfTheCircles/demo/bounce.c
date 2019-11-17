#include <math.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "color.h"

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

const int outer_r = 233;
const int inner_r = 144;
const int num_points = 5;
const int deg_rot = 15000;
const double r = 1;
const double g = .75;
const double b = .796;

double x_vel = 400;
double y_vel = 400;
Vector max_diff;

double get_point(int point){
    return (point * (2 * M_PI)/ num_points);
}

VectorList* make_star(){
    VectorList* points = vec_list_init(num_points * 2);
    int i;
    for(i = num_points; i > 0; i--){
        double point = get_point((i+1) % num_points);
        Vector outer = {.x = outer_r * cos(point), .y = outer_r * sin(point)};

        point = get_point((i+4) % num_points);
        Vector inner = {.x = inner_r * cos(point), .y = inner_r  * sin(point)};

        vec_list_add(points, inner);
        vec_list_add(points, outer);
    }
    return points;

}

void move(VectorList* star, double dt){
    Vector motion = {.x = x_vel * dt, .y = y_vel * dt};
    polygon_translate(star, motion);
    polygon_rotate(star, M_PI/deg_rot, polygon_centroid(star));
}

int check_touch(VectorList *polygon, Vector edges, int dir){
    int i, poly_size = vec_list_size(polygon);

    for(i = 0; i < poly_size; i++){
        Vector point = vec_list_get(polygon, i);
        double curr_x = point.x;
        double curr_y = point.y;

        if(!dir){
            if(fabs(curr_x) > fabs(edges.x)){
                return 1;
            }
        } else {
            if(fabs(curr_y) > fabs(edges.y)){
                return 1;
            }
        }
    }
    return 0;
}

void check_edge(VectorList* star){
    if(check_touch(star, max_diff, 0)){
        x_vel *= -1;
    }
    if(check_touch(star, max_diff, 1)){
        y_vel *= -1;
    }
}

int main(int argc, const char* argv[]){
    Vector min_corn = {.x = -WINDOW_WIDTH, .y = -   WINDOW_HEIGHT};
    Vector min_max = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    sdl_init(min_corn, min_max);
    max_diff = vec_subtract(min_max, vec_multiply(.5, vec_add(min_corn, min_max)));
    VectorList* star = make_star();
    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        move(star, dt);
        check_edge(star);

        sdl_clear();
        sdl_draw_polygon(star, (RGBColor){r, g, b}, false);
        sdl_show();
    }
    vec_list_free(star);
}
