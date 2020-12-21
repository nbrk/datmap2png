/*
 * Copyright 2020 Nikolay Burkov <nbrk@linklevel.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "internal.h"

#include <cairo.h>
#include <glib.h>

static struct ProgramContext program_context;

void
draw_terrain(cairo_t* cr, const Map* map)
{
  for (int i = 0; i < map->raster_height; i++) {
    for (int j = 0; j < map->raster_width; j++) {
      TerrainType ttype;
      TerrainFlags tflags;
      short terr = map_raster_read(map, j, i);
      terrain_decode(terr, &ttype, &tflags);

      // basic type
      switch (ttype) {
        case TERRAIN_TYPE_CLEAR:
          cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
          break;
        case TERRAIN_TYPE_ROUGH4:
          cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
          break;
        case TERRAIN_TYPE_ROUGH3:
          cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
          break;
        case TERRAIN_TYPE_ROUGH2:
          cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
          break;
        case TERRAIN_TYPE_ROUGH1:
          cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
          break;
        case TERRAIN_TYPE_WATER:
          cairo_set_source_rgb(cr, 0.1, 0.6, 1.0);
          break;
        default:
          cairo_set_source_rgb(cr, 1.0, 1.0, 0);
          break;
      }
      cairo_rectangle(cr, 10. * j + 0.5, 10. * i + 0.5, 10., 10.);
      cairo_set_line_width(cr, 1.0);
      cairo_fill_preserve(cr);

      // flags
      if (tflags & TERRAIN_FLAG_ROAD) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_fill_preserve(cr);
      }
      cairo_stroke(cr);

      if (tflags & TERRAIN_FLAG_ELEVATION) {
        cairo_set_source_rgb(cr, 0.6, 0.4, 0.2);
        cairo_rectangle(cr, 10. * j + 0.5, 10. * i + 0.5, 9., 9.);
        cairo_stroke(cr);
      }

      if (tflags & TERRAIN_FLAG_WOODS) {
        cairo_set_source_rgb(cr, 0.4, 0.8, 0.4);
        cairo_rectangle(cr, 10. * j + 4 - 0.5, 10. * i + 4 - 0.5, 3., 3.);
        cairo_fill_preserve(cr);
        cairo_stroke(cr);
      }
      if (tflags & TERRAIN_FLAG_TOWN) {
        cairo_set_source_rgb(cr, 0.6, 0, 0.2);
        cairo_rectangle(cr, 10. * j + 4 - 0.5, 10. * i + 4 - 0.5, 3., 3.);
        cairo_fill_preserve(cr);
        cairo_stroke(cr);
      }
    }
  }
}

void
draw_grid(cairo_t* cr, const Map* map, int spacing)
{
  int hlines = map->height / spacing + 1;
  int vlines = map->width / spacing + 1;
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb(cr, 0, 0, 0);
  for (int i = 0; i < vlines; i++) {
    cairo_move_to(cr, i * spacing + 0.5, 0 + 0.5);
    cairo_line_to(cr, i * spacing + 0.5, map->height - 1 + 0.5);
  }
  for (int i = 0; i < hlines; i++) {
    cairo_move_to(cr, 0 + 0.5, i * spacing + 0.5);
    cairo_line_to(cr, map->width - 1 + 0.5, i * spacing + 0.5);
  }
  cairo_stroke(cr);
}

void
draw_descr(cairo_t* cr, const Map* map)
{
  cairo_select_font_face(
    cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_move_to(cr, 0, map->height + 18);
  cairo_set_source_rgb(cr, 1., 1., 1.);
  char str[256];

  g_snprintf(
    str,
    sizeof(str),
    "Generated from TacOps map \"%s\" version %d (%d x %d pixels, %d x "
    "%d terrain raster)",
    map->name,
    map->version,
    map->width,
    map->height,
    map->raster_width,
    map->raster_height);
  cairo_show_text(cr, str);
}

void
draw_background(cairo_t* cr)
{
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);
}

void
draw_everything(cairo_t* cr, const Map* map)
{
  draw_background(cr);
  if (program_context.flags & FLAG_VERBOSE)
    g_print("Wrote background\n");

  draw_terrain(cr, map);
  if (program_context.flags & FLAG_VERBOSE)
    g_print("Wrote terrain\n");

  if (program_context.flags & FLAG_ADD_GRID_1KM) {
    draw_grid(cr, map, 100);
    if (program_context.flags & FLAG_VERBOSE)
      g_print("Wrote 1km grid\n");
  }

  if (program_context.flags & FLAG_ADD_GRID_5KM) {
    draw_grid(cr, map, 500);
    if (program_context.flags & FLAG_VERBOSE)
      g_print("Wrote 5km grid\n");
  }

  if (program_context.flags & FLAG_ADD_GRID_10KM) {
    draw_grid(cr, map, 1000);
    if (program_context.flags & FLAG_VERBOSE)
      g_print("Wrote 10km grid\n");
  }

  if (program_context.flags & FLAG_ADD_TEXT_DESCR) {
    draw_descr(cr, map);
    if (program_context.flags & FLAG_VERBOSE)
      g_print("Wrote description\n");
  }
}

int
program_main(struct ProgramContext progctx)
{
  // save the context for the whole module
  program_context = progctx;

  if (program_context.flags & FLAG_VERBOSE) {
    g_print("Input file: %s\n", program_context.in_filename);
    g_print("Output file: %s\n", program_context.out_filename);
  }

  Map* map = map_load(program_context.in_filename);
  if (program_context.flags & FLAG_VERBOSE) {
    g_print("Loaded map %s version %d: %d x %d (%d x %d terrain raster)\n",
            map->name,
            map->version,
            map->width,
            map->height,
            map->raster_width,
            map->raster_height);
  }

  cairo_surface_t* surf = cairo_image_surface_create(
    CAIRO_FORMAT_RGB24,
    map->width,
    map->height + (program_context.flags & FLAG_ADD_TEXT_DESCR ? 20 : 0));
  cairo_t* cr = cairo_create(surf);

  draw_everything(cr, map);

  cairo_surface_write_to_png(surf, program_context.out_filename);
  if (program_context.flags & FLAG_VERBOSE)
    g_print("Wrote PNG to %s\n", program_context.out_filename);

  map_free(map);
  cairo_surface_destroy(surf);
  free(program_context.in_filename);
  free(program_context.out_filename);

  return EXIT_SUCCESS;
}
