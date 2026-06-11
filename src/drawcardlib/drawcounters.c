// -*- tab-width:4; c-basic-offset:2; -*-
// Drawcardlib: display card and mana cost graphics.
// drawcounters.c: draw counters

#include "drawcardlib.h"

WINGDIAPI BOOL WINAPI AlphaBlend(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);

int suppress_next_counters = 0;

static int
get_displayed_instance_copy(int player, int card, card_instance_t* dest)
{
  if (!dest || player < 0 || player > 1 || card < 0 || card >= 151)
	return 0;

  card_instance_t* instance = get_displayed_card_instance(player, card);	// unsafe to dereference
  if (!instance || !critical_section_for_display)
	return 0;

  EnterCriticalSection(critical_section_for_display);
  memcpy(dest, instance, sizeof(card_instance_t));
  LeaveCriticalSection(critical_section_for_display);

  return 1;
}

static int
enter_counter_drawing_section_if_needed(void)
{
  if (parent == PARENT_SHANDALAR && critical_section_for_drawing)
	{
	  EnterCriticalSection(critical_section_for_drawing);
	  return 1;
	}

  return 0;
}

static void
leave_counter_drawing_section_if_needed(int entered)
{
  if (entered)
	LeaveCriticalSection(critical_section_for_drawing);
}

static int
counter_image_slot_is_valid(int counter_type)
{
  if (counter_type < 0 || counter_type == 255)
	return 0;

  if (counters_num_columns <= 0 || counters_num_rows <= 0)
	return 0;

  return counter_type < counters_num_columns * counters_num_rows;
}

static int
get_counter_atlas_cell_size(unsigned int* src_width, unsigned int* src_height)
{
  if (src_width)
	*src_width = 0;
  if (src_height)
	*src_height = 0;

  if (!src_width || !src_height || counters_num_columns <= 0 || counters_num_rows <= 0)
	return 0;

  if (counters_renderer == RENDERER_GDIPLUS)
	{
	  if (!gpics[CARDCOUNTERS])
		make_gpic_from_pic(CARDCOUNTERS);

	  if (!gdip_get_image_size(gpics[CARDCOUNTERS], src_width, src_height))
		return 0;
	}
  else
	{
	  if (!pics[CARDCOUNTERS])
		return 0;

	  BITMAP bmp;
	  if (GetObject(pics[CARDCOUNTERS], sizeof(BITMAP), &bmp) != sizeof(BITMAP)
		  || bmp.bmWidth <= 0 || bmp.bmHeight <= 0)
		return 0;

	  *src_width = bmp.bmWidth;
	  *src_height = bmp.bmHeight;
	}

  *src_width /= counters_num_columns;
  *src_height /= counters_num_rows;

  return *src_width > 0 && *src_height > 0;
}

static int
calculate_scaled_counter_size(const RECT* rect, unsigned int src_width, unsigned int src_height,
							  int* scaled_width, int* scaled_height)
{
  if (scaled_width)
	*scaled_width = 0;
  if (scaled_height)
	*scaled_height = 0;

  if (!rect || !scaled_width || !scaled_height || src_width == 0 || src_height == 0)
	return 0;

  int height = rect->bottom - rect->top;
  if (height <= 0)
	return 0;

  int64_t width = (int64_t)height * src_width / src_height;
  if (width <= 0 || width > INT_MAX)
	return 0;

  *scaled_width = (int)width;
  *scaled_height = height;
  return 1;
}

static void
init_blendfunction(BLENDFUNCTION* blend)
{
  blend->BlendOp = AC_SRC_OVER;
  blend->BlendFlags = 0;
  blend->SourceConstantAlpha = 255;
  blend->AlphaFormat = AC_SRC_ALPHA;
}

void
draw_special_counters(HDC hdc, const RECT* rect, int player, int card)
{
  // magic.exe:0x4d3a00

  if (!hdc || !rect)
	return;

  card_instance_t actual;
  if (!get_displayed_instance_copy(player, card, &actual))
	return;

  card_instance_t* instance = &actual;

  int counter_type = instance->special_counter_type;
  if (counter_type < 0 || counter_type >= COUNTER_end)
	return;

  int num = instance->special_counters;
  if (num <= 0)
	return;

  if (suppress_next_counters && counter_type == COUNTER_LOYALTY)
	return;

  if (!counter_image_slot_is_valid(counter_type))
	return;

  RECT r;
  get_special_counter_rect(&r, rect, num);

  int drawing_locked = enter_counter_drawing_section_if_needed();

  unsigned int src_width, src_height;
  if (!get_counter_atlas_cell_size(&src_width, &src_height))
	{
	  leave_counter_drawing_section_if_needed(drawing_locked);
	  return;
	}

  int scaled_width, scaled_height;
  if (!calculate_scaled_counter_size(&r, src_width, src_height, &scaled_width, &scaled_height))
	{
	  leave_counter_drawing_section_if_needed(drawing_locked);
	  return;
	}

  int spacing;
  if (num <= 1)
	spacing = scaled_width;
  else
	{
	  spacing = (r.right - r.left - scaled_width) / (num - 1);
	  if (spacing < 1)
		spacing = 1;
	}

  int src_xpos = (counter_type / counters_num_rows) * src_width;
  int src_ypos = (counter_type % counters_num_rows) * src_height;

  int dest_x = r.right - scaled_width;

  BLENDFUNCTION blend;
  int savedc = 0;
  HGDIOBJ old_spare_bmp = NULL;

  if (counters_renderer != RENDERER_GDIPLUS)
	{
	  if (!spare_hdc || !*spare_hdc || !pics[CARDCOUNTERS])
		{
		  leave_counter_drawing_section_if_needed(drawing_locked);
		  return;
		}

	  savedc = SaveDC(hdc);
	  old_spare_bmp = SelectObject(*spare_hdc, pics[CARDCOUNTERS]);
	  init_blendfunction(&blend);
	}

  int i;
  for (i = 0; i < num; ++i)
	{
	  r.left = dest_x;
	  r.right = dest_x + scaled_width;

	  if (counters_renderer == RENDERER_GDIPLUS)
		gdip_blt(hdc, &r, CARDCOUNTERS, src_xpos, src_ypos, src_width, src_height, NULL);
	  else
		AlphaBlend(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top,
				   *spare_hdc, src_xpos, src_ypos, src_width, src_height, blend);

	  dest_x -= spacing;
	}

  if (counters_renderer != RENDERER_GDIPLUS)
	{
	  if (old_spare_bmp)
		SelectObject(*spare_hdc, old_spare_bmp);
	  if (savedc)
		RestoreDC(hdc, savedc);
	}

  leave_counter_drawing_section_if_needed(drawing_locked);
}

void
get_special_counter_rect(RECT* rect_dest, const RECT* rect_src, int num)
{
  // 0x4d3b60

  if (!rect_dest)
	return;

  if (!rect_src || num <= 0)
	{
	  SetRect(rect_dest, 0, 0, 0, 0);
	  return;
	}

  RECT r;
  r.left = rect_src->left + ((unsigned int)(18 * (rect_src->right - rect_src->left)) >> 8);
  r.right = rect_src->right - ((unsigned int)(51 * (rect_src->right - rect_src->left)) >> 8);
  r.top = rect_src->top + ((unsigned int)(20 * (rect_src->bottom - rect_src->top)) >> 8);
  r.bottom = r.top + 27 * (rect_src->bottom - rect_src->top) / 100;

  unsigned int src_width, src_height;
  if (!get_counter_atlas_cell_size(&src_width, &src_height))
	{
	  SetRect(rect_dest, 0, 0, 0, 0);
	  return;
	}

  int scaled_width, scaled_height;
  if (!calculate_scaled_counter_size(&r, src_width, src_height, &scaled_width, &scaled_height))
	{
	  SetRect(rect_dest, 0, 0, 0, 0);
	  return;
	}

  int spacing;
  for (spacing = scaled_width;
	   spacing > 1 && r.left + scaled_width + spacing * (num - 1) > r.right;
	   --spacing)
	{}

  SetRect(rect_dest, r.left, r.top, r.left + scaled_width + spacing * (num - 1), r.bottom);
}

void
draw_standard_counters(HDC hdc, const RECT* rect, int player, int card)
{
  if (!hdc || !rect || rect->bottom <= rect->top || rect->right <= rect->left)
	return;

  card_instance_t actual;
  if (!get_displayed_instance_copy(player, card, &actual))
	return;

  card_instance_t* instance = &actual;

  int counters = instance->counters;
  int counters2 = instance->counters2;
  int counters3 = instance->counters3;
  int counters4 = instance->counters4;
  int counters5 = instance->counters5;
  int counters_m1m1 = instance->counters_m1m1;

  int counter_type_2;
  int counter_type_3;
  int counter_type_4;
  int counter_type_5;

  if (parent == PARENT_SHANDALAR)
	{
	  counter_type_2 = instance->unk52;
	  counter_type_3 = instance->unk53;
	  counter_type_4 = instance->card_color;
	  counter_type_5 = instance->unk5a;
	}
  else
	{
	  uint8_t* t18 = (void*)(&instance->targets[18].player);
	  counter_type_2 = t18[0];
	  counter_type_3 = t18[1];
	  counter_type_4 = t18[2];
	  counter_type_5 = t18[3];
	}

  if (!counters && !counters2 && !counters3 && !counters4 && !counters5 && !counters_m1m1)
	return;

  int drawing_locked = enter_counter_drawing_section_if_needed();

  unsigned int src_width, src_height;
  if (!get_counter_atlas_cell_size(&src_width, &src_height))
	{
	  leave_counter_drawing_section_if_needed(drawing_locked);
	  return;
	}

  int scaled_width, scaled_height;
  if (!calculate_scaled_counter_size(rect, src_width, src_height, &scaled_width, &scaled_height))
	{
	  leave_counter_drawing_section_if_needed(drawing_locked);
	  return;
	}

  int one_third_card_width = (rect->right - rect->left) / 3;
  if (one_third_card_width <= 0)
	{
	  leave_counter_drawing_section_if_needed(drawing_locked);
	  return;
	}

  BLENDFUNCTION blend;
  RECT r;
  int spacing, dest_x, src_xpos, src_ypos, i, savedc = 0;
  HGDIOBJ old_spare_bmp = NULL;

  if (counters_renderer != RENDERER_GDIPLUS)
	{
	  if (!spare_hdc || !*spare_hdc || !pics[CARDCOUNTERS])
		{
		  leave_counter_drawing_section_if_needed(drawing_locked);
		  return;
		}

	  savedc = SaveDC(hdc);
	  old_spare_bmp = SelectObject(*spare_hdc, pics[CARDCOUNTERS]);
	  init_blendfunction(&blend);
	}

#define COUNTERS(which, init_x, init_y, imgnum, lambda)	do {																\
  if ((which) > 0 && counter_image_slot_is_valid(imgnum))																	\
	{																														\
	  for (spacing = scaled_width; scaled_width + spacing * ((which) - 1) > one_third_card_width && spacing > 2; --spacing)	\
		{}																													\
	  dest_x = (init_x) + spacing * ((which) - 1);																			\
	  src_xpos = ((imgnum) / counters_num_rows) * src_width;																\
	  src_ypos = ((imgnum) % counters_num_rows) * src_height;																\
	  r.top = (init_y);																										\
	  r.bottom = r.top + scaled_height;																						\
	  for (i = 0; i < (which); ++i)																							\
		{																													\
		  r.left = dest_x;																									\
		  r.right = r.left + scaled_width;																					\
																															\
		  { lambda; }																										\
																															\
		  if (counters_renderer == RENDERER_GDIPLUS)																		\
			gdip_blt(hdc, &r, CARDCOUNTERS, src_xpos, src_ypos, src_width, src_height, NULL);								\
		  else																												\
			AlphaBlend(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top,												\
					   *spare_hdc, src_xpos, src_ypos, src_width, src_height, blend);										\
																															\
		  dest_x -= spacing;																								\
		}																													\
	}																														\
  } while (0)

  int left_edge_of_dest = rect->left;
  int top_edge_of_dest = rect->top;
  COUNTERS(counters4, left_edge_of_dest, top_edge_of_dest, counter_type_4,);

  int left_third_of_dest = left_edge_of_dest + one_third_card_width;
  COUNTERS(counters3, left_third_of_dest, top_edge_of_dest, counter_type_3,);

  int right_third_of_dest = left_third_of_dest + one_third_card_width;
  COUNTERS(counters2, right_third_of_dest, top_edge_of_dest, counter_type_2,);

  int middle_of_dest = (rect->right + rect->left) / 2;
  int left_sixth_of_dest = middle_of_dest - one_third_card_width;
  int bottom_third_of_dest = 2 * scaled_height / 3 + top_edge_of_dest;
  int p1p1m1m1 = counters + counters_m1m1, changed = 0;
  COUNTERS(p1p1m1m1, left_sixth_of_dest, bottom_third_of_dest, COUNTER_P1_P1,
		   {
			 if (!changed && i >= counters)
			   {
				 changed = 1;
				 src_xpos = (COUNTER_M1_M1 / counters_num_rows) * src_width;
				 src_ypos = (COUNTER_M1_M1 % counters_num_rows) * src_height;
			   }
		   });

  COUNTERS(counters5, middle_of_dest, bottom_third_of_dest, counter_type_5,);

  if (counters_renderer != RENDERER_GDIPLUS)
	{
	  if (old_spare_bmp)
		SelectObject(*spare_hdc, old_spare_bmp);
	  if (savedc)
		RestoreDC(hdc, savedc);
	}

  leave_counter_drawing_section_if_needed(drawing_locked);

#undef COUNTERS
}