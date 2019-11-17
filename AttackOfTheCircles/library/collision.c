#include "collision.h"
#include "polygon.h"
#include "math.h"

typedef struct projection_info{
	bool intersected;
	double intersection;
	Vector projection_axis;
} ProjectionInfo;

typedef struct min_and_max{
	double min;
	double max;
} MinMax;

ProjectionInfo projection_info_init(bool intersected, double intersection, Vector projection_axis){
	ProjectionInfo projection_info;
	projection_info.intersected = intersected;
	projection_info.intersection = intersection;
	projection_info.projection_axis = projection_axis;
	return projection_info;
}

MinMax shape_project(List *shape, Vector axis){
	double min = INFINITY;
	double max = -INFINITY;
	MinMax min_and_max;
	for(size_t i = 0; i < list_size(shape); i++){
		Vector v = *(Vector*)list_get(shape, i);
		double project = vec_dot(v, axis);
			if(project < min){
				min = project;
			}
			if(project > max){
				max = project;
			}
	}
	min_and_max.min = min;
	min_and_max.max = max;
	return min_and_max;
}

List *shape_edges(List *shape){
	size_t size = list_size(shape);
	List *edges = list_init(size, free);
	for(size_t i = 0; i < size; i++){
		Vector from = *(Vector*)list_get(shape, i);
		Vector to = *(Vector*)list_get(shape, (i + 1) % size);
		Vector *edge = malloc(sizeof(Vector));
		*edge = vec_subtract(to, from);
		list_add(edges, edge);
	}
	return edges;
}

ProjectionInfo get_projection_intersection(List *shape1, List *shape2, Vector axis){
	ProjectionInfo projection_info = projection_info_init(false, 0.0, axis);
	MinMax m1 = shape_project(shape1, axis);
	MinMax m2 = shape_project(shape2, axis);
	double min1 = m1.min;
	double max1 = m1.max;
	double min2 = m2.min;
	double max2 = m2.max;

	if(max2 > max1){
		projection_info.intersected = min2 < max1;
		projection_info.intersection = fabs(min2 - max1);
	}

	else{
		projection_info.intersected = min1 < max2;
		projection_info.intersection = fabs(min1 - max2);
	}
	return projection_info;
}

Vector get_projection_edge(Vector edge, List *shape1, List *shape2){
	Vector projection_edge = {-edge.y, edge.x};
	Vector centroid_1 = polygon_centroid(shape1);
	Vector centroid_2 = polygon_centroid(shape2);
	Vector one_to_two = vec_subtract(centroid_2, centroid_1);
	double dotted_two_vectors = vec_dot(one_to_two, projection_edge);
	double mag_dist = vec_magnitude(one_to_two);
	double mag_2 = vec_magnitude(projection_edge);
	double divided = (1/(mag_dist * mag_2) * dotted_two_vectors);
	if (divided < 0){
		return vec_negate(projection_edge);
	}
	return projection_edge;
}

ProjectionInfo get_smallest_projection_shape_axis (List *shape1, List* shape2, List* edges){
	bool separating_axis = false;
	int i = 0;
	double smallest_projection = INFINITY;
	Vector smallest_projection_axis = {0.0, 0.0};

	while(!separating_axis && i < list_size(edges)){
		Vector edge = *(Vector*)list_get(edges, i);
		Vector projection_edge = get_projection_edge(edge, shape1, shape2);
		ProjectionInfo projection_info = get_projection_intersection(shape1, shape2, projection_edge);
		separating_axis = !projection_info.intersected;
		if (smallest_projection > projection_info.intersection){
			smallest_projection = projection_info.intersection;
			smallest_projection_axis = projection_edge;
		}
		i++;
	}

	smallest_projection_axis = vec_multiply(1 / vec_magnitude(smallest_projection_axis),
		smallest_projection_axis);

	ProjectionInfo info = projection_info_init(separating_axis, smallest_projection, smallest_projection_axis);
	return info;
}

CollisionInfo find_collision(List *shape1, List *shape2){
	List *edges1 = shape_edges(shape1);
	List *edges2 = shape_edges(shape2);

	ProjectionInfo projection_info_1 = get_smallest_projection_shape_axis(shape1, shape2, edges1);
	ProjectionInfo projection_info_2 = get_smallest_projection_shape_axis(shape1, shape2, edges2);

	CollisionInfo collision_info;

	bool separating_axis_1 = projection_info_1.intersected;
	bool separating_axis_2 = projection_info_2.intersected;
	collision_info.collided = !separating_axis_1 && !separating_axis_2;

	if (projection_info_1.intersection < projection_info_2.intersection){
		collision_info.axis = projection_info_1.projection_axis;
	}
	else{
		collision_info.axis = projection_info_2.projection_axis;
	}

	list_free(edges1);
	list_free(edges2);
	return collision_info;
}
