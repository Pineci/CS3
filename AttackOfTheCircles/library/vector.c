#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "vector.h"

const Vector VEC_ZERO = { .x = 0, .y = 0};
const int deg_to_rad = M_PI / 180.0;

Vector vec_add(Vector v1, Vector v2){
    Vector to_ret = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return to_ret;
}

Vector vec_subtract(Vector v1, Vector v2){
    return vec_add(v1, vec_negate(v2));
}

Vector vec_negate(Vector v){
    return vec_multiply(-1, v);
}

Vector vec_multiply(double scalar, Vector v){
    Vector to_ret = {
        .x = v.x * scalar,
        .y = v.y * scalar
    };
    return to_ret;
}

double vec_dot(Vector v1, Vector v2){
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(Vector v1, Vector v2){
    return v1.x * v2.y - v2.x * v1.y;
}

Vector vec_rotate(Vector v, double angle){
    double rad_ang = angle;
    Vector to_ret = {
        .x = v.x * cos(rad_ang) - v.y * sin(rad_ang),
        .y = v.x * sin(rad_ang) + v.y * cos(rad_ang)
    };
    return to_ret;
}

double vec_distance_squared(Vector v1, Vector v2){
    Vector diff = vec_subtract(v2, v1);
    return vec_dot(diff, diff);
}

double vec_magnitude_squared(Vector v){
    return vec_dot(v, v);
}

double vec_distance(Vector v1, Vector v2){
    return sqrt(vec_distance_squared(v1, v2));
}

double vec_magnitude(Vector v){
    return sqrt(vec_magnitude_squared(v));
}

Vector vec_project(Vector v1, Vector v2){
    double dot = vec_dot(v1, v2);
    double norm_term = vec_magnitude_squared(v2);
    return vec_multiply(dot / norm_term, v2);
}
