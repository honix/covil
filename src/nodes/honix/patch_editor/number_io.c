#include <stdio.h>

#include <GLFW/glfw3.h>

#include "nanovg/src/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/src/nanovg_gl.h"

#include "core/node.h"
#include "core/link.h"
#include "core/loop.h"
#include "core/dl_loader.h"

void number_io(struct node *node)
{
}

void number_io_draw(struct NVGcontext *vg, struct node *node)
{
	int x = node->rect.pos.x;
	int y = node->rect.pos.y;

	int number = *(int *)node->in_pins[0]->data;

	// Draw node name
	nvgBeginPath(vg);
	nvgFillColor(vg, nvgRGBA(0, 0, 250, 75));
	nvgRect(vg, x, y,
			node->rect.size.x * ((float)number / 100), node->rect.size.y);
	nvgFill(vg);

	nvgFontSize(vg, 15.0f);
	nvgFontFace(vg, "sans");
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
	char s[32];
	sprintf(s, "%d", number);
	nvgText(vg, x + 15, y + 30 / 2, s, NULL);
}

void number_io_input_key_func(
	struct node *node,
	int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	switch (key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_KP_8:
		*(int *)node->in_pins[0]->data += 1;
		break;

	case GLFW_KEY_DOWN:
	case GLFW_KEY_KP_2:
		*(int *)node->in_pins[0]->data -= 1;
		break;

	default:
		break;
	}
}

void register_library(reg_function_t reg)
{
	reg((struct function_note){
		.name = "number_io",
		.main_func = number_io,
		.draw_func = number_io_draw,
		.input_key_func = number_io_input_key_func});
}