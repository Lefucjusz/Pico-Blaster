#pragma once

typedef struct
{
	float alpha;
	float current_output;
} filter_t;

int filter_init(filter_t *filter, float initial_value, float alpha);

float filter_update(filter_t *filter, float value);

float filter_get_value(filter_t *filter);
