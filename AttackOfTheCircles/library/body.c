#include "body.h"
#include "sdl_wrapper.h"
#include "polygon.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

typedef struct accel_info{
    int interval;
    Vector disp_prev;
    Vector v_prev;
    Vector v_curr;
    Vector v_next;
    double h_prev;
    double h_curr;
    double a;
    double b;
    double n;
} AccelInfo;

struct body{
    List* body_points;
    double mass;
    Vector velocity;
    double rotation_angle;
    RGBColor color;
    Vector centroid;
    Vector forces;
    Vector impulses;
    double largest_radius;
    void *info;
    FreeFunc info_freer;
    bool is_removed;
    bool camera_attachment;
    AccelInfo *accel_info;
};



double shape_largest_radius(List *vertices, Vector center){
    double max_sqr_distance = 0;
    for(size_t i = 0; i < list_size(vertices); i++){
        double distance_sqr = vec_distance_squared(*(Vector*)list_get(vertices, i), center);
        if(distance_sqr > max_sqr_distance){
            max_sqr_distance = distance_sqr;
        }
    }
    return sqrt(max_sqr_distance);
}

AccelInfo * accel_info_init(){
    AccelInfo *aInfo = malloc(sizeof(AccelInfo));
    aInfo->interval = 1;
    aInfo->disp_prev = VEC_ZERO;
    aInfo->v_prev = VEC_ZERO;
    aInfo->v_curr = VEC_ZERO;
    aInfo->v_next = VEC_ZERO;
    aInfo->h_prev = 1000.0;
    aInfo->h_curr = 1000.0;
    aInfo->a = 0;
    aInfo->b = 0;
    aInfo->n = 0;
    return aInfo;
}

Body *body_init(List *shape, double mass, RGBColor color){
    assert(mass > 0);
    Body *body = malloc(sizeof(Body));
    body->body_points = shape;
    body->centroid = polygon_centroid(body->body_points);
    body->mass = mass;
    body->color = color;
    body->velocity = VEC_ZERO;
    body->forces = VEC_ZERO;
    body->impulses = VEC_ZERO;
    body->rotation_angle = 0.0;
    body->largest_radius = shape_largest_radius(body->body_points, body->centroid);
    body->is_removed = false;
    body->info_freer = NULL;
    body->camera_attachment = true;
    body->accel_info = accel_info_init();
    return body;
}

Body *body_init_with_info(List *shape, double mass, RGBColor color, void *info,
  FreeFunc info_freer){
    Body *body = body_init(shape, mass, color);
    body->info = info;
    body->info_freer = info_freer;
    return body;
}

void body_free(Body *body){
    list_free(body->body_points);
    if (body->info_freer != NULL){
      body->info_freer(body->info);
    }
    free(body->accel_info);
    free(body);
}

List *body_get_shape(Body *body){
    size_t body_points_size = list_size(body->body_points);
    List *new_list = list_init(body_points_size, free);

    for (size_t i = 0; i < body_points_size; i++){
        Vector *element = list_get(body->body_points, i);
        Vector *new_element = malloc(sizeof(Vector));
        *new_element = *element;
        list_add(new_list, new_element);
    }
    return new_list;
}

Vector body_get_centroid(Body *body){
    return body->centroid;
}

Vector body_get_velocity(Body *body){
    return body->velocity;
}

double body_get_mass(Body *body){
    return body->mass;
}

RGBColor body_get_color(Body *body){
    return body->color;
}

void *body_get_info(Body *body){
    return body->info;
}

void body_set_camera_attatchment(Body *body, bool val){
    body->camera_attachment = val;
}

bool body_get_camera_attachment(Body *body){
    return body->camera_attachment;
}

void body_set_centroid(Body *body, Vector x){
    Vector translation = vec_subtract(x, body->centroid);
    body->centroid = x;
    polygon_translate(body->body_points, translation);
}

void body_set_velocity(Body *body, Vector v){
    body->velocity = v;
}

void body_set_rotation(Body *body, double angle){
    polygon_rotate(body->body_points, angle - body->rotation_angle, body->centroid);
    body->rotation_angle = angle;
}

void body_add_force(Body *body, Vector force){
    body->forces = vec_add(body->forces, force);
}

void body_add_impulse(Body *body, Vector impulse){
    body->impulses = vec_add(body->impulses, impulse);
}

/*
void compute_even_coefficients(AccelInfo *info){
    double h0 = info->h_prev;
    double h1 = info->h_curr;
    info->a = (2*h1*h1*h1 - h0*h0*h0 + 3*h0*h1*h1)/(6*h1*(h1+h0));
    info->b = (h1*h1*h1 + h0*h0*h0 + 3*h1*h0*(h1+h0))/(6*h1*h0);
    info->n = (2*h0*h0*h0 - h1*h1*h1 + 3*h1*h0*h0)/(6*h0*(h1+h0));
}

void compute_odd_coefficients(AccelInfo *info){
    double h0 = info->h_prev;
    double h1 = info->h_curr;
    info->a = (2*h1*h1 + 3*h1*h0)/(6*(h0+h1));
    info->b = (h1*h1 + 3*h1*h0)/(6*h0);
    info->n = (-h1*h1*h1)/(6*h0*(h0+h1));
}


void body_tick(Body *body, double dt){
    AccelInfo *info = body->accel_info;
    Vector dv_inst = vec_multiply(1/body_get_mass(body), body->impulses);
    Vector dv_accel = vec_multiply(dt/body_get_mass(body), body->forces);
    Vector inst_velocity = vec_add(body->velocity, dv_inst);
    Vector final_velocity = vec_add(inst_velocity, dv_accel);
    info->v_next = final_velocity;
    info->h_curr = dt;

    Vector displacement;
    if(info->interval == 1){
        displacement = vec_multiply(0.5*dt, vec_add(body->velocity, final_velocity));
        info->v_curr = body->velocity;
    } else if(info->interval % 2 == 0){
        compute_even_coefficients(info);
        Vector term1 = vec_multiply(info->a, info->v_next);
        Vector term2 = vec_multiply(info->b, info->v_curr);
        Vector term3 = vec_multiply(info->n, info->v_prev);
        displacement = vec_subtract(vec_add(vec_add(term1, term2), term3), info->disp_prev);
    } else{
        compute_odd_coefficients(info);
        Vector term1 = vec_multiply(info->a, info->v_next);
        Vector term2 = vec_multiply(info->b, info->v_curr);
        Vector term3 = vec_multiply(info->n, info->v_prev);
        displacement = vec_add(vec_add(term1, term2), term3);
    }

    info->disp_prev = displacement;
    info->interval += 1;
    info->v_prev = info->v_curr;
    info->v_curr = final_velocity;
    info->h_prev = info->h_curr;
    body_set_velocity(body, final_velocity);

    polygon_translate(body->body_points, displacement);
    body->centroid = vec_add(body->centroid, displacement);

    body_set_velocity(body, final_velocity);
    body->forces = VEC_ZERO;
    body->impulses = VEC_ZERO;
}*/

void body_tick(Body *body, double dt){
    Vector dv_inst = vec_multiply(1/body_get_mass(body), body->impulses);
    Vector dv_accel = vec_multiply(dt/body_get_mass(body), body->forces);
    Vector inst_velocity = vec_add(body->velocity, dv_inst);
    Vector final_velocity = vec_add(inst_velocity, dv_accel);

    Vector displacement = vec_multiply(0.5*dt, vec_add(body->velocity, final_velocity));
    polygon_translate(body->body_points, displacement);
    body->centroid = vec_add(body->centroid, displacement);

    body_set_velocity(body, final_velocity);
    body->forces = VEC_ZERO;
    body->impulses = VEC_ZERO;
}

void body_remove(Body *body){
    body->is_removed = true;
}

bool body_is_removed(Body *body){
    return body->is_removed;
}

double body_radius(Body *b){
    return b->largest_radius;
}
