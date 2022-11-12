#include "ecs.h"
#include "fs.h"
#include "gpu.h"
#include "heap.h"
#include "render.h"
#include "timer_object.h"
#include "transform.h"
#include "wm.h"
#include "frogger_game.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#include "debug.h"
#include "vulkan/vulkan.h"

typedef struct obstacle_component_t 
{
	int index;
	int direction;
}obstacle_component_t;

typedef struct transform_component_t
{
	transform_t transform;
} transform_component_t;

typedef struct camera_component_t
{
	mat4f_t projection;
	mat4f_t view;
} camera_component_t;

typedef struct model_component_t
{
	gpu_mesh_info_t* mesh_info;
	gpu_shader_info_t* shader_info;
} model_component_t;

typedef struct player_component_t
{
	int index;
} player_component_t;

typedef struct name_component_t
{
	char name[32];
} name_component_t;

typedef struct frogger_game_t
{
	heap_t* heap;
	fs_t* fs;
	wm_window_t* window;
	render_t* render;

	timer_object_t* timer;

	ecs_t* ecs;
	int transform_type;
	int camera_type;
	int model_type;
	int player_type;
	int name_type;
	int obstacle_type;
	ecs_entity_ref_t player_ent;
	ecs_entity_ref_t camera_ent;
	ecs_entity_ref_t obstacle_ent;

	gpu_mesh_info_t player_mesh;
	gpu_mesh_info_t obstacle1_mesh;
	gpu_mesh_info_t obstacle2_mesh;
	gpu_mesh_info_t obstacle3_mesh;
	gpu_shader_info_t cube_shader;
	fs_work_t* vertex_shader_work;
	fs_work_t* fragment_shader_work;
} frogger_game_t;

static void load_resources(frogger_game_t* game);
static void unload_resources(frogger_game_t* game);
static void spawn_player(frogger_game_t* game, int index);
static void spawn_obstacle(frogger_game_t* game, int index, float offset, float height, int obstacle_variation);
static void spawn_camera(frogger_game_t* game);
static void update_players(frogger_game_t* game);
static void update_obstacles(frogger_game_t* game);
static void draw_models(frogger_game_t* game);

frogger_game_t* frogger_game_create(heap_t* heap, fs_t* fs, wm_window_t* window, render_t* render)
{
	frogger_game_t* game = heap_alloc(heap, sizeof(frogger_game_t), 8);
	game->heap = heap;
	game->fs = fs;
	game->window = window;
	game->render = render;

	game->timer = timer_object_create(heap, NULL);
	
	game->ecs = ecs_create(heap);
	game->transform_type = ecs_register_component_type(game->ecs, "transform", sizeof(transform_component_t), _Alignof(transform_component_t));
	game->camera_type = ecs_register_component_type(game->ecs, "camera", sizeof(camera_component_t), _Alignof(camera_component_t));
	game->model_type = ecs_register_component_type(game->ecs, "model", sizeof(model_component_t), _Alignof(model_component_t));
	game->player_type = ecs_register_component_type(game->ecs, "player", sizeof(player_component_t), _Alignof(player_component_t));
	game->obstacle_type = ecs_register_component_type(game->ecs, "obstacle", sizeof(obstacle_component_t), _Alignof(obstacle_component_t));
	game->name_type = ecs_register_component_type(game->ecs, "name", sizeof(name_component_t), _Alignof(name_component_t));

	load_resources(game);
	spawn_player(game, 0);
	//lane1 at height 9
	spawn_obstacle(game,1, -30.0f, 9.0f, 1);
	spawn_obstacle(game,1, -7.5f, 9.0f, 2);
	spawn_obstacle(game,1, -17.0f, 9.0f, 0);
	spawn_obstacle(game,1, -23.0f, 9.0f, 2);
	spawn_obstacle(game,1, 0.0f, 9.0f, 1);
	spawn_obstacle(game,1, 7.5f, 9.0f, 2);
	spawn_obstacle(game,1, 17.0f, 9.0f, 0);
	spawn_obstacle(game,1, 23.0f, 9.0f, 1);
	//lane2 at height 3
	spawn_obstacle(game,1, 1.0f, 3.0f, 1);
	spawn_obstacle(game,1, -17.0f, 3.0f, 1);
	spawn_obstacle(game,1, -8.0f, 3.0f, 0);
	spawn_obstacle(game,1, -3.0f, 3.0f, 1);
	spawn_obstacle(game,1, 1.0f, 3.0f, 1);
	spawn_obstacle(game,1, 8.0f, 3.0f, 2);
	spawn_obstacle(game,1, 15.0f, 3.0f, 0);
	spawn_obstacle(game,1, 20.0f, 3.0f, 1);
	spawn_obstacle(game,1, 30.0f, 3.0f, 1);
	//lane3 at height -3
	spawn_obstacle(game,1, 25.0f, -3.0f, 0);
	spawn_obstacle(game,1, 17.0f, -3.0f, 1);
	spawn_obstacle(game,1, 13.0f, -3.0f, 2);
	spawn_obstacle(game,1, 5.0f, -3.0f, 1);
	spawn_obstacle(game,1, -4.0f, -3.0f, 0);
	spawn_obstacle(game,1, -10.0f, -3.0f, 0);
	spawn_obstacle(game,1, -18.0f, -3.0f, 1);
	spawn_obstacle(game,1, -25.0f, -3.0f, 2);
	spawn_obstacle(game,1, -30.0f, -3.0f, 1);
	//lane4 at height -9
	spawn_obstacle(game,1, -22.0f, -9.0f, 1);
	spawn_obstacle(game,1, -5.0f, -9.0f, 1);
	spawn_obstacle(game,1, -14.0f, -9.0f, 2);
	spawn_obstacle(game,1, 0.0f, -9.0f, 0);
	spawn_obstacle(game,1, 4.0f, -9.0f, 1);
	spawn_obstacle(game,1, 13.0f, -9.0f, 1);
	spawn_obstacle(game,1, 23.0f, -9.0f, 0);
	spawn_obstacle(game,1, 30.0f, -9.0f, 2);
	//spawn_player(game, 1);
	spawn_camera(game);

	return game;
}

void frogger_game_destroy(frogger_game_t* game)
{
	ecs_destroy(game->ecs);
	timer_object_destroy(game->timer);
	unload_resources(game);
	heap_free(game->heap, game);
}

void frogger_game_update(frogger_game_t* game)
{
	timer_object_update(game->timer);
	ecs_update(game->ecs);
	update_players(game);
	update_obstacles(game);
	draw_models(game);
	render_push_done(game->render);
}

static void load_resources(frogger_game_t* game)
{
	game->vertex_shader_work = fs_read(game->fs, "shaders/triangle.vert.spv", game->heap, false, false);
	game->fragment_shader_work = fs_read(game->fs, "shaders/triangle.frag.spv", game->heap, false, false);
	game->cube_shader = (gpu_shader_info_t)
	{
		.vertex_shader_data = fs_work_get_buffer(game->vertex_shader_work),
		.vertex_shader_size = fs_work_get_size(game->vertex_shader_work),
		.fragment_shader_data = fs_work_get_buffer(game->fragment_shader_work),
		.fragment_shader_size = fs_work_get_size(game->fragment_shader_work),
		.uniform_buffer_count = 1,
	};

	static vec3f_t cube_verts[] =
	{
		// depth	L or R	  height    rgb
		{ -1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f,  0.0f }, //back, left, bottom
		{  1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f,  0.0f }, //front, left, bottom
		{  1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f,  0.0f }, //front, right, bottom
		{ -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f,  0.0f }, //back, right, bottom
		{ -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f,  0.0f }, //back, left, top
		{  1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f,  0.0f }, //front, left, top
		{  1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f,  0.0f }, //front, right, top
		{ -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f,  0.0f }, //back, right top
	};
	static vec3f_t obs1_verts[] =
	{
		// depth	L or R	  height    rgb
		{ -1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f,  1.0f }, //back, left, bottom
		{  1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f,  1.0f }, //front, left, bottom
		{  1.0f,  1.0f,  1.0f }, { 1.0f, 0.0f,  1.0f }, //front, right, bottom
		{ -1.0f,  1.0f,  1.0f }, { 1.0f, 0.0f,  1.0f }, //back, right, bottom
		{ -1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f,  1.0f }, //back, left, top
		{  1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f,  1.0f }, //front, left, top
		{  1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f,  1.0f }, //front, right, top
		{ -1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f,  1.0f }, //back, right top
	};
	static vec3f_t obs2_verts[] =
	{
		// depth	L or R	  height    rgb
		{ -1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f,  1.0f }, //back, left, bottom
		{  1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f,  1.0f }, //front, left, bottom
		{  1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f,  1.0f }, //front, right, bottom
		{ -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f,  1.0f }, //back, right, bottom
		{ -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f,  1.0f }, //back, left, top
		{  1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f,  1.0f }, //front, left, top
		{  1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f,  1.0f }, //front, right, top
		{ -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f,  1.0f }, //back, right top
	};
	static vec3f_t obs3_verts[] =
	{
		// depth	L or R	  height    rgb
		{ -1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f,  1.0f }, //back, left, bottom
		{  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f,  1.0f }, //front, left, bottom
		{  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f,  1.0f }, //front, right, bottom
		{ -1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f,  1.0f }, //back, right, bottom
		{ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f,  1.0f }, //back, left, top
		{  1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f,  1.0f }, //front, left, top
		{  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f,  1.0f }, //front, right, top
		{ -1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f,  1.0f }, //back, right top
	};
	static uint16_t cube_indices[] =
	{
		0, 1, 2, //underside, top triangle
		2, 3, 0, //underside bottom triangle
		1, 5, 6, //face top triange
		6, 2, 1, //face bottom triangle
		7, 6, 5, //topside top triangle
		5, 4, 7, //topside bottom triangle
		4, 0, 3, //back top triangle
		3, 7, 4, //bakc bottom triangle
		4, 5, 1, //left top triangle
		1, 0, 4, //left bottom triangle
		3, 2, 6, //right top triangle
		6, 7, 3  //right bottom triangle
	};
	game->player_mesh = (gpu_mesh_info_t)
	{
		.layout = k_gpu_mesh_layout_tri_p444_c444_i2,
		.vertex_data = cube_verts,
		.vertex_data_size = sizeof(cube_verts),
		.index_data = cube_indices,
		.index_data_size = sizeof(cube_indices),
	};
	game->obstacle1_mesh = (gpu_mesh_info_t)
	{
		.layout = k_gpu_mesh_layout_tri_p444_c444_i2,
		.vertex_data = obs1_verts,
		.vertex_data_size = sizeof(obs1_verts),
		.index_data = cube_indices,
		.index_data_size = sizeof(cube_indices),
	};
	game->obstacle2_mesh = (gpu_mesh_info_t)
	{
		.layout = k_gpu_mesh_layout_tri_p444_c444_i2,
		.vertex_data = obs2_verts,
		.vertex_data_size = sizeof(obs1_verts),
		.index_data = cube_indices,
		.index_data_size = sizeof(cube_indices),
	};
	game->obstacle3_mesh = (gpu_mesh_info_t)
	{
		.layout = k_gpu_mesh_layout_tri_p444_c444_i2,
		.vertex_data = obs3_verts,
		.vertex_data_size = sizeof(obs1_verts),
		.index_data = cube_indices,
		.index_data_size = sizeof(cube_indices),
	};
}

static void unload_resources(frogger_game_t* game)
{
	fs_work_destroy(game->fragment_shader_work);
	fs_work_destroy(game->vertex_shader_work);
}

static void spawn_player(frogger_game_t* game, int index)
{
	uint64_t k_player_ent_mask =
		(1ULL << game->transform_type) |
		(1ULL << game->model_type) |
		(1ULL << game->player_type) |
		(1ULL << game->name_type);
	game->player_ent = ecs_entity_add(game->ecs, k_player_ent_mask);

	transform_component_t* transform_comp = ecs_entity_get_component(game->ecs, game->player_ent, game->transform_type, true);
	transform_identity(&transform_comp->transform);
	transform_comp->transform.translation.z = 13.0f;
	transform_comp->transform.scale = vec3f_new(1.0f, 1.0f, 1.0f);
	name_component_t* name_comp = ecs_entity_get_component(game->ecs, game->player_ent, game->name_type, true);
	strcpy_s(name_comp->name, sizeof(name_comp->name), "player");

	player_component_t* player_comp = ecs_entity_get_component(game->ecs, game->player_ent, game->player_type, true);
	player_comp->index = index;

	model_component_t* model_comp = ecs_entity_get_component(game->ecs, game->player_ent, game->model_type, true);
	model_comp->mesh_info = &game->player_mesh;
	model_comp->shader_info = &game->cube_shader;
}

static void spawn_camera(frogger_game_t* game)
{
	uint64_t k_camera_ent_mask =
		(1ULL << game->camera_type) |
		(1ULL << game->name_type);
	game->camera_ent = ecs_entity_add(game->ecs, k_camera_ent_mask);

	name_component_t* name_comp = ecs_entity_get_component(game->ecs, game->camera_ent, game->name_type, true);
	strcpy_s(name_comp->name, sizeof(name_comp->name), "camera");

	camera_component_t* camera_comp = ecs_entity_get_component(game->ecs, game->camera_ent, game->camera_type, true);
	mat4f_make_orthographic(&camera_comp->projection,16.0f / 8.0f, .1f, 14.0f);

	vec3f_t eye_pos = vec3f_scale(vec3f_forward(), -5.0f);
	vec3f_t forward = vec3f_forward();
	vec3f_t up = vec3f_up();
	mat4f_make_lookat(&camera_comp->view, &eye_pos, &forward, &up);
}

static void spawn_obstacle(frogger_game_t* game, int index, float offset, float height, int obstacle_variation)
{
	uint64_t k_object_ent_mask =
		(1ULL << game->transform_type) |
		(1ULL << game->model_type) |
		(1ULL << game->obstacle_type) |
		(1ULL << game->name_type);
	game->obstacle_ent = ecs_entity_add(game->ecs, k_object_ent_mask);

	transform_component_t* transform_comp = ecs_entity_get_component(game->ecs, game->obstacle_ent, game->transform_type, true);
	transform_identity(&transform_comp->transform);
	transform_comp->transform.translation.z = height;
	transform_comp->transform.translation.y = offset;

	name_component_t* name_comp = ecs_entity_get_component(game->ecs, game->obstacle_ent, game->name_type, true);
	strcpy_s(name_comp->name, sizeof(name_comp->name), "obstacle");

	obstacle_component_t* obstacle_comp = ecs_entity_get_component(game->ecs, game->obstacle_ent, game->obstacle_type, true);
	obstacle_comp->index = index;
	if(height == 3.0f || height == -9.0f)
	{
		obstacle_comp->direction = -1;
	}
	else
	{
		obstacle_comp->direction = 1;
	}
	//obstacle_comp->direction = 1;
	model_component_t* model_comp = ecs_entity_get_component(game->ecs, game->obstacle_ent, game->model_type, true);
	//int colorindex = rand() %3;
	switch(obstacle_variation)
	{
		case 0:
			model_comp->mesh_info = &game->obstacle1_mesh;
			transform_comp->transform.scale = vec3f_new(1.0f, 1.0f, .8f);
			break;
		case 1:
			model_comp->mesh_info = &game->obstacle2_mesh;
			transform_comp->transform.scale = vec3f_new(1.0f, 2.0f, .8f);
			break;
		case 2:
			model_comp->mesh_info = &game->obstacle3_mesh;
			transform_comp->transform.scale = vec3f_new(1.0f, 2.5f, .8f);
			break;
	default:
		model_comp->mesh_info = &game->obstacle1_mesh;
		transform_comp->transform.scale = vec3f_new(1.0f, 5.0f, 1.0f);
	}
	
	//model_comp->mesh_info = &game->obstacle1_mesh;
	//transform_comp->transform.scale = vec3f_new(1.0f, 2.0f, 1.0f);
	model_comp->shader_info = &game->cube_shader;
}

static uint32_t last_move_time = 0;
static void update_players(frogger_game_t* game)
{
	//float dt = (float)timer_object_get_delta_ms(game->timer) * 0.001f;
	uint32_t current_time = timer_object_get_ms(game->timer);
	uint32_t key_mask = wm_get_key_mask(game->window);
	
	uint64_t k_player_query_mask = (1ULL << game->transform_type) | (1ULL << game->player_type);
	uint64_t k_obstacle_query_mask = (1ULL << game->transform_type) | (1ULL << game->obstacle_type);

	for (ecs_query_t player_query = ecs_query_create(game->ecs, k_player_query_mask);
		ecs_query_is_valid(game->ecs, &player_query);
		ecs_query_next(game->ecs, &player_query))
	{
		transform_component_t* transform_comp = ecs_query_get_component(game->ecs, &player_query, game->transform_type);
		player_component_t* player_comp = ecs_query_get_component(game->ecs, &player_query, game->player_type);

		if (player_comp->index && transform_comp->transform.translation.z > 1.0f)
		{
			ecs_entity_remove(game->ecs, ecs_query_get_entity(game->ecs, &player_query), false);
		}
		for (ecs_query_t obstacle_query = ecs_query_create(game->ecs, k_obstacle_query_mask);
			ecs_query_is_valid(game->ecs, &obstacle_query);
			ecs_query_next(game->ecs, &obstacle_query))
		{
			transform_component_t* obs_transform = ecs_query_get_component(game->ecs, &obstacle_query, game->transform_type);
			model_component_t* model_component = ecs_query_get_component(game->ecs, &obstacle_query, game->model_type);
			float collider_scaler = model_component->mesh_info == &game->obstacle1_mesh ? 2.0f :   model_component->mesh_info == &game->obstacle2_mesh ? 1.5f : 1.4f;
		
			if (transform_comp->transform.translation.y <= obs_transform->transform.translation.y + obs_transform->transform.scale.y * collider_scaler &&// *(2/obs_transform->transform.scale.y)&&
				transform_comp->transform.translation.y >= obs_transform->transform.translation.y - obs_transform->transform.scale.y * collider_scaler  &&
				transform_comp->transform.translation.z == obs_transform->transform.translation.z)
			{
				
				transform_comp->transform.translation = vec3f_new(0.0f,0.0f,13.0f);
			}
		}
		if(transform_comp->transform.translation.z <= -13.0f)
		{
			transform_comp->transform.translation = vec3f_new(0.0f,0.0f,13.0f);
		}
		
		if(current_time - last_move_time > 100)
		{
			if (key_mask & k_key_up && transform_comp->transform.translation.z > -13.0f) // bound check is redundant as player will be moved to start position
			{
				transform_comp->transform.translation.z = transform_comp->transform.translation.z - 2.0f ;//vec3f_add(move.translation, vec3f_scale(vec3f_up(), -dt));
				last_move_time = current_time;
			}
			if (key_mask & k_key_down && transform_comp->transform.translation.z < 13.0f)
			{
				transform_comp->transform.translation.z = transform_comp->transform.translation.z + 2.0f ;//vec3f_add(move.translation, vec3f_scale(vec3f_up(), -dt));
				last_move_time = current_time;
			}
			if (key_mask & k_key_left && transform_comp->transform.translation.y > -26.0f)
			{
				transform_comp->transform.translation.y = transform_comp->transform.translation.y - 2.0f ;//vec3f_add(move.translation, vec3f_scale(vec3f_up(), -dt));
				last_move_time = current_time;
			}
			if (key_mask & k_key_right && transform_comp->transform.translation.y < 26.0f)
			{
				transform_comp->transform.translation.y = transform_comp->transform.translation.y + 2.0f ;
				last_move_time = current_time;
			}
		}
		//transform_multiply(&transform_comp->transform, &move);
		//transform_comp->transform = move;
		
	}
}

static void update_obstacles(frogger_game_t* game)
{
	float dt = (float)timer_object_get_delta_ms(game->timer) * 0.001f;
	uint64_t k_query_mask = (1ULL << game->transform_type) | (1ULL << game->obstacle_type);

	for (ecs_query_t query = ecs_query_create(game->ecs, k_query_mask);
		ecs_query_is_valid(game->ecs, &query);
		ecs_query_next(game->ecs, &query))
	{
		transform_component_t* transform_comp = ecs_query_get_component(game->ecs, &query, game->transform_type);
		obstacle_component_t* obstacle_comp = ecs_query_get_component(game->ecs, &query, game->obstacle_type);

		//obstacles of scale.y 1 disappear as if their scale.y was .5, or offcenter from their transform
		if(obstacle_comp->direction * transform_comp->transform.translation.y > 26.0f+(2*transform_comp->transform.scale.y))
		{
			if(obstacle_comp->direction > 0)
			{
				transform_comp->transform.translation.y = -26 - (10 - transform_comp->transform.scale.y);
			}
			else
			{
				transform_comp->transform.translation.y = 26 + (10 - transform_comp->transform.scale.y);
			}
		}


		//the offsets become closer together each run - i think its tied to the default persective and scale? im really not sure
		transform_t move;
		transform_identity(&move);
		if(obstacle_comp->direction > 0)
		{
			move.translation = vec3f_add(move.translation, vec3f_scale(vec3f_right(), dt));
		}
		else
		{
			move.translation = vec3f_add(move.translation, vec3f_scale(vec3f_negate(vec3f_right()), dt ));
		}
		transform_multiply(&transform_comp->transform, &move);

	}
}

static void draw_models(frogger_game_t* game)
{
	uint64_t k_camera_query_mask = (1ULL << game->camera_type);
	for (ecs_query_t camera_query = ecs_query_create(game->ecs, k_camera_query_mask);
		ecs_query_is_valid(game->ecs, &camera_query);
		ecs_query_next(game->ecs, &camera_query))
	{
		camera_component_t* camera_comp = ecs_query_get_component(game->ecs, &camera_query, game->camera_type);

		uint64_t k_model_query_mask = (1ULL << game->transform_type) | (1ULL << game->model_type);
		for (ecs_query_t query = ecs_query_create(game->ecs, k_model_query_mask);
			ecs_query_is_valid(game->ecs, &query);
			ecs_query_next(game->ecs, &query))
		{
			transform_component_t* transform_comp = ecs_query_get_component(game->ecs, &query, game->transform_type);
			model_component_t* model_comp = ecs_query_get_component(game->ecs, &query, game->model_type);
			ecs_entity_ref_t entity_ref = ecs_query_get_entity(game->ecs, &query);

			struct
			{
				mat4f_t projection;
				mat4f_t model;
				mat4f_t view;
			} uniform_data;
			uniform_data.projection = camera_comp->projection;
			uniform_data.view = camera_comp->view;
			transform_to_matrix(&transform_comp->transform, &uniform_data.model);
			gpu_uniform_buffer_info_t uniform_info = { .data = &uniform_data, sizeof(uniform_data) };

			render_push_model(game->render, &entity_ref, model_comp->mesh_info, model_comp->shader_info, &uniform_info);
		}
	}
}
