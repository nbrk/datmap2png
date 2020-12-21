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
#pragma once

#include <stdint.h>
#include <stdlib.h>

/*
 *
 * Part of free and open-source clone of the TacOps.
 *
 */

/*
 *
 * DATA
 *
 */
/*
 * User-given flags
 */
#define FLAG_VERBOSE 1 << 0
#define FLAG_ADD_TEXT_DESCR 1 << 1
#define FLAG_ADD_GRID_1KM 1 << 2
#define FLAG_ADD_GRID_5KM 1 << 3
#define FLAG_ADD_GRID_10KM 1 << 4

struct ProgramContext
{
  char* in_filename;
  char* out_filename;
  int flags;
};

/**
 * @brief Map and the terrain raster
 */
typedef struct
{
  short width;
  short height;
  short raster_width;
  short raster_height;
  short utm_easting;
  short utm_northing;
  short version;
  char name[8];
  short* raster;
} Map;

/**
 * @brief Terrain base type
 */
typedef enum
{
  TERRAIN_TYPE_CLEAR = 100,
  TERRAIN_TYPE_NOGO1 = 201,
  TERRAIN_TYPE_NOGO2 = 202,
  TERRAIN_TYPE_NOGO3 = 203,
  TERRAIN_TYPE_ROUGH1 = 301,
  TERRAIN_TYPE_ROUGH2 = 302,
  TERRAIN_TYPE_ROUGH3 = 303,
  TERRAIN_TYPE_ROUGH4 = 304,
  TERRAIN_TYPE_WATER = 400
} TerrainType;

/**
 * @brief Terrain flags
 */
typedef enum
{
  TERRAIN_FLAG_LOSBLOCK = 1,
  TERRAIN_FLAG_ELEVATION = 2,
  TERRAIN_FLAG_ROAD = 4,
  TERRAIN_FLAG_WOODS = 8,
  TERRAIN_FLAG_TOWN = 16
} TerrainFlags;

/*
 *
 * FUNCTIONS
 *
 */

Map*
map_load(const char* path);

void
map_free(Map* m);

short
map_raster_read(const Map* m, short col, short row);

short
map_get(const Map* m, short x, short y, short* rcol, short* rrow);

void
terrain_decode(short terr, TerrainType* ttype, TerrainFlags* tflags);

int
program_main(struct ProgramContext progctx);
