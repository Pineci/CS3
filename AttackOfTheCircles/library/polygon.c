#include <math.h>
#include "polygon.h"

double polygon_area(VectorList *polygon){
  double area = 0.0;
  size_t numVertices = vec_list_size(polygon);
  for(size_t i = 0; i < numVertices; i++){
    Vector v1 = vec_list_get(polygon, i);
    Vector v2 = vec_list_get(polygon, (i+1)%numVertices);
    area += vec_cross(v1, v2);
  }
  area = 0.5 * area;
  return area;
}

Vector polygon_centroid(VectorList* polygon){
  Vector centroid = VEC_ZERO;
  size_t numVertices = vec_list_size(polygon);
  for(size_t i = 0; i < numVertices; i++){
    Vector v1 = vec_list_get(polygon, i);
    Vector v2 = vec_list_get(polygon, (i+1)%numVertices);
    double cross = vec_cross(v1, v2);
    centroid = vec_add(vec_multiply(cross, vec_add(v1, v2)), centroid);
  }
  centroid = vec_multiply(1.0/(6*polygon_area(polygon)), centroid);
  return centroid;
}


void polygon_translate(VectorList* polygon, Vector translation){
  for(size_t i = 0; i < vec_list_size(polygon); i++){
    Vector translatedVector = vec_add(vec_list_get(polygon, i), translation);
    vec_list_set(polygon, i, translatedVector);
  }
}


void polygon_rotate(VectorList* polygon, double angle, Vector point){
  polygon_translate(polygon, vec_negate(point));
  for(size_t i = 0; i < vec_list_size(polygon); i++){
    Vector rotatedVector = vec_rotate(vec_list_get(polygon, i), angle);
    vec_list_set(polygon, i, rotatedVector);
  }
  polygon_translate(polygon, point);
}
