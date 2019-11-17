#include "shape.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

List* shape_star(int num_spokes, double scale, double ld, double sd){
    Vector unit = {.x = 0, .y = scale};
    List* points = list_init(num_spokes * 2, free);
    double rotation_angle = (2 * M_PI) / (num_spokes * 2);

    for(int i = 0; i < num_spokes; i++){
        Vector *v = malloc(sizeof(Vector));
        *v = vec_multiply(ld, unit);
        list_add(points, v);
        unit = vec_rotate(unit, rotation_angle);
        v = malloc(sizeof(Vector));
        *v = vec_multiply(sd, unit);
        list_add(points, v);
        unit = vec_rotate(unit, rotation_angle);
    }
    return points;
}

List *shape_triangle(double radius){
    List *points = list_init(3, free);
    double rotation_angle = (2 * M_PI) / 3;
    Vector init = {.x = 0, .y = radius};

    for(int i =  0; i < 3; i++){
        Vector *v = malloc(sizeof(Vector));
        *v = init;
        list_add(points, v);
        init = vec_rotate(init, rotation_angle);
    }
    return points;
}

List *shape_regular_star(int num_spokes, double scale){
    return shape_star(num_spokes, scale, .144*scale, .089*scale);
}

List *shape_estrella(double radius){
    return shape_star(4, 1, radius, radius/2);
}

List* shape_partial_circle(double radius, int num_vertices, double proportion){
    assert(num_vertices >= 3);
    Vector *v = malloc(sizeof(Vector));
    *v = (Vector){0, radius};
    List *l = list_init(num_vertices, free);
    for(int i = 0; i < num_vertices; i++){
        list_add(l, v);
        Vector *new_v = malloc(sizeof(Vector));
        *new_v = vec_rotate(*v, 2 * M_PI/num_vertices * proportion);
        v = new_v;
    }
    return l;
}

List *shape_circle(double radius, int num_vertices){
    return shape_partial_circle(radius, num_vertices, 1.0);
}

List *shape_rectangle(double width, double height){
    List *l = list_init(4, free);
    Vector *v = malloc(sizeof(Vector));
    *v = (Vector){width/2.0, height/2.0};
    list_add(l, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){-width/2.0, height/2.0};
    list_add(l, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){-width/2.0, -height/2.0};
    list_add(l, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){width/2.0, -height/2.0};
    list_add(l, v);
    return l;
}


double gen_single_color(){
    return  1.0 * rand() / RAND_MAX;
}

RGBColor gen_color(){
    return (RGBColor){gen_single_color(), gen_single_color(), gen_single_color()};
}
