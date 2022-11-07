#version 460 core

#define PI 3.1415926535

// local group size
layout (local_size_x = 32, local_size_y = 8, local_size_z = 1) in;

// image textures
layout (binding = 1, rgba32f) uniform image2D trail_map;
layout (binding = 2, rgba32f) uniform image2D agent_map;

// settings SSBO
struct settings_struct {
	float move_speed;
	float turn_speed;
	float sensor_angle;
	float sensor_distance;

	int width;
	int height;

	float r;
	float g;
	float b;
	float decay_rate;
	float diffuse_rate;
};
layout (std430, binding = 3) buffer settings_buffer {
	settings_struct settings;
};

// agents SSBO
struct agent {
	float x;
	float y;
	float angle;
};
layout(std430, binding = 4) buffer agent_buffer {
	agent agent_array[];
};

uint hash(uint state) {
	state ^= 2747636419u;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	return state;
}

float normalize(uint state) {
	float res = state / 4294967295.f;
	return res;
}

float sense_trail(agent a, float sensor_offset, float sensor_distance) {
	float sensor_angle = a.angle + sensor_offset;

	int sensor_x = int(a.x + cos(sensor_angle) * sensor_distance);
	int sensor_y = int(a.y + sin(sensor_angle) * sensor_distance);

	float sense_sum = 0;
	for(int offset_x = -1; offset_x <= 1; offset_x++) {
		for(int offset_y = -1; offset_y <= 1; offset_y++) {
			int sample_x = min(settings.width - 1, max(0, sensor_x + offset_x));
			int sample_y = min(settings.height - 1, max(0, sensor_y + offset_y));

			sense_sum += dot(imageLoad(trail_map, ivec2(sample_x, sample_y)), vec4(1, 1, 1, 1));
		}
	}

	return sense_sum;
}

void main() {
	// set the width and height of the map
	int width = settings.width;
	int height = settings.height;

	// set the move and turn speed that will be associated with the sim
	float move_speed = settings.move_speed;
	float turn_speed = settings.turn_speed;

	// set the agent sensor angle and sensor distance
	float sensor_angle = settings.sensor_angle;
	float sensor_distance = settings.sensor_distance;

	ivec2 id = ivec2(gl_GlobalInvocationID.xy);

	// check if the current position in the computer is larger than the array given to the computer
	if(id.x > agent_array.length()) {
		return;
	}

	// set the current agent we will work with
	agent current_agent = agent_array[id.x];

	// initialize a random value
	uint rand = hash(int(current_agent.y * width + current_agent.x) + hash(int(id.x * 824941)));

	// se the sense values for the agent
	float sense_f = sense_trail(current_agent, 0, sensor_distance);
	float sense_l = sense_trail(current_agent, sensor_angle, sensor_distance);
	float sense_r = sense_trail(current_agent, -sensor_angle, sensor_distance);

	float steer_strength = normalize(hash(rand));

	if (sense_f == 0 && sense_l == 0 && sense_r == 0) { // if there is no trail to sense, just go crazy
		current_agent.angle += (steer_strength - 0.5) * 2 * turn_speed;
	} else if(sense_f > sense_l && sense_f > sense_r) { // if there is trail in front, then stay the course
		current_agent.angle += 0;
	} else if (sense_f < sense_l && sense_f < sense_r) { // if there is equal parts left and right, turn in a random direction
		current_agent.angle += (steer_strength - 0.5) * 2 * turn_speed;
	} else if (sense_l > sense_r) { // if left is greater, then go left
		current_agent.angle += (steer_strength * turn_speed);
	} else if (sense_l < sense_r) { // if right is greater, then go right
		current_agent.angle -= (steer_strength * turn_speed);
	} else { // otherwise go crazy
		current_agent.angle += (steer_strength - 0.5) * 2 * turn_speed;
	}

	// move the agent in its new angle
	current_agent.x += move_speed * cos(current_agent.angle);
	current_agent.y += move_speed * sin(current_agent.angle);

	// check if it hits the wall, then bounce it off the wall in a random direction
	if (current_agent.x <= 0 || current_agent.x >= width || current_agent.y <= 0 || current_agent.y >= height) {
		rand = hash(rand);
		float rand_angle = normalize(rand) * 2 * PI;

		current_agent.x = min(width - 1, max(0, current_agent.x));
		current_agent.y = min(height - 1, max(0, current_agent.y));
		current_agent.angle = rand_angle;
	}

	// store the agent map
	agent_array[id.x] = current_agent;

	vec4 agent_color = vec4(settings.r, settings.g, settings.b, 1);

	imageStore(agent_map, ivec2(current_agent.x, current_agent.y), agent_color);

	// store the trail map
	vec4 deposit = vec4(agent_color / 5);
	deposit.a = 1;

	vec4 previous_trail = imageLoad(trail_map, ivec2(current_agent.x, current_agent.y));
	vec4 new_trail = vec4(min(previous_trail + deposit, agent_color));

	imageStore(trail_map, ivec2(current_agent.x, current_agent.y), new_trail);
}
