#version 460 core

out vec4 frag_color;

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

void main() {
	// set the width and height of the map
	int width = settings.width;
	int height = settings.height;

	// handle the decay rate and the diffuse rate
	float decay_rate = settings.decay_rate;
	float diffuse_rate = settings.diffuse_rate;

	// load the color originally in the image
	vec4 original_color = imageLoad(trail_map, ivec2(gl_FragCoord.xy)).rgba;

	// blur the image
	vec4 blurred_color = vec4(0);
	int total_weight = 0;
	for(int offset_x = -1; offset_x <= 1; offset_x++) {
		for(int offset_y = -1; offset_y <= 1; offset_y++) {
			int sample_x = min(width - 1, max(0, int(gl_FragCoord.x) + offset_x));
			int sample_y = min(width - 1, max(0, int(gl_FragCoord.y) + offset_y));

			blurred_color += imageLoad(trail_map, ivec2(sample_x, sample_y)).rgba;
			total_weight += 1;
		}
	}

	blurred_color /= total_weight;

	float diffuse_weight = clamp(diffuse_rate, 0, 1);

	// set the trail color and store the trail map
	vec4 trail_color = original_color * (1 - diffuse_weight) + blurred_color * diffuse_weight;

	trail_color -= decay_rate;
	trail_color.a = 1;

	imageStore(trail_map, ivec2(gl_FragCoord.xy), max(trail_color, 0.0f));

	// handle the agent color and store the agent map
	vec4 agent_color = imageLoad(agent_map, ivec2(gl_FragCoord.xy)).rgba;

	if(agent_color.a > 0.1) {
		frag_color = agent_color;
	} else {
		frag_color = trail_color;
	}

	imageStore(agent_map, ivec2(gl_FragCoord.xy), vec4(0, 0, 0, 0));
}