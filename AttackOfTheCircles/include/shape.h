#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <stdbool.h>
#include "list.h"

#include "vector.h"
#include "vec_list.h"
#include "polygon.h"
#include "color.h"


List *shape_star(int num_spokes, double radius, double ld, double sd);
List *shape_estrella(double radius);
List *shape_partial_circle(double radius, int num_vertices, double proportion);
List *shape_circle(double radius, int num_vertices);
List *shape_regular_star(int num_spokes, double scale);
List *shape_rectangle(double width, double height);
List *shape_triangle(double radius);

RGBColor gen_color();


#endif
