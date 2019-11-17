#include "gui.h"
#include "helpers.h"
#include "gen_levels.h"
#include "shape.h"
#include "body.h"

const double BULLET_RADIUS = 30.0;
const double BULLET_SPACING = 100.0;
const int BULLET_CIRCLE_POINTS = 50;

void add_bullet_indicator(Scene *scene, Vector location){
	List *circle = shape_circle(BULLET_RADIUS, BULLET_CIRCLE_POINTS);
	BodyInfo *info = create_body_info(GUI_BULLET, NONE);
	Body *bullet = body_init_with_info(circle, 1.0, (RGBColor){0.0, 0.7, 1.0}, info, (FreeFunc)body_info_free);
	body_set_camera_attatchment(bullet, false);
	body_set_centroid(bullet, location);
	scene_add_body(scene, bullet);
}

void add_gui(Scene *scene, Vector min_corn, Vector max_corn, int max_bullets){
	Body *player = get_first_body(scene, PLAYER);
	BodyInfo *info = body_get_info(player);
	int num_bullets = info->bullet_count;
	List *bullet_indicators = get_bodies_type(scene, GUI_BULLET);
	double width = max_corn.x - min_corn.y;
	double height = max_corn.y - min_corn.y;
	Vector start_location = vec_add(vec_add(min_corn, (Vector){0, height}), (Vector){width/15.0, -height/20.0});
	Vector shift = (Vector){BULLET_SPACING, 0};
	if(list_size(bullet_indicators) < max_bullets - num_bullets){
		for(int i = list_size(bullet_indicators); i < max_bullets; i++){
			add_bullet_indicator(scene, vec_add(start_location, vec_multiply(i, shift)));
		}
	} else {
		for(int i = list_size(bullet_indicators); i > max_bullets-num_bullets; i--){
			body_remove(list_get(bullet_indicators, i-1));
		}
	}
	list_free(bullet_indicators);
}
