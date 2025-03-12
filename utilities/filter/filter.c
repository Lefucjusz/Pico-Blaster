#include "filter.h"
#include <errno.h>
#include <math.h>
#include <stddef.h>

int filter_init(filter_t *filter, float initial_value, float alpha)
{
	if (filter == NULL) {
		return -EINVAL;
	}

	filter->alpha = alpha;
	filter->current_output = initial_value;

	return 0;
}

float filter_update(filter_t *filter, float value)
{
	if (filter == NULL) {
		return NAN;
	}

	filter->current_output = filter->alpha * value + (1.0f - filter->alpha) * filter->current_output;

	return filter->current_output;
}

float filter_get_value(filter_t *filter)
{
	if (filter == NULL) {
		return NAN;
	}
	return filter->current_output;
}
