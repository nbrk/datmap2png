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

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Some useful offsets into the .dat header
 */
#define DAT_HDR_RASTER_WIDTH 0x06
#define DAT_HDR_RASTER_HEIGHT 0x08
#define DAT_HDR_WIDTH 0x0e
#define DAT_HDR_HEIGHT 0x10
#define DAT_HDR_EASTING 0x32
#define DAT_HDR_NORTHING 0x34
#define DAT_HDR_VERSION 0x36
#define DAT_HDR_NAME 0x38
#define DAT_HDR_RASTER_START 0x40

/*
 * Raw terrain (msb)
 */
#define RAW_TERRAIN_CLEAR 0x00
#define RAW_TERRAIN_NOGO1 0x01
#define RAW_TERRAIN_NOGO2 0x02
#define RAW_TERRAIN_NOGO3 0x04
#define RAW_TERRAIN_ROUGH1 0x08
#define RAW_TERRAIN_ROUGH2 0x10
#define RAW_TERRAIN_ROUGH3 0x18
#define RAW_TERRAIN_ROUGH4 0x20
#define RAW_TERRAIN_WATER 0x30

/*
 * Raw terrain flags (lsb)
 */
#define RAW_TERRAIN_FLAG_LOSBLOCK 0x02
#define RAW_TERRAIN_FLAG_ELEVATION 0x08
#define RAW_TERRAIN_FLAG_ROAD 0x20
#define RAW_TERRAIN_FLAG_WOODS 0x40
#define RAW_TERRAIN_FLAG_TOWN 0x80

Map*
map_load(const char* path)
{
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return NULL;

  /*
   * Header
   */
  int16_t rastwidth, rastheight, width, height, easting, northing, version;
  char name[8];
  fseek(fp, DAT_HDR_RASTER_WIDTH, SEEK_SET);
  fread(&rastwidth, 2, 1, fp);
  fseek(fp, DAT_HDR_RASTER_HEIGHT, SEEK_SET);
  fread(&rastheight, 2, 1, fp);
  fseek(fp, DAT_HDR_WIDTH, SEEK_SET);
  fread(&width, 2, 1, fp);
  fseek(fp, DAT_HDR_HEIGHT, SEEK_SET);
  fread(&height, 2, 1, fp);
  fseek(fp, DAT_HDR_EASTING, SEEK_SET);
  fread(&easting, 2, 1, fp);
  fseek(fp, DAT_HDR_NORTHING, SEEK_SET);
  fread(&northing, 2, 1, fp);
  fseek(fp, DAT_HDR_VERSION, SEEK_SET);
  fread(&version, 2, 1, fp);
  fseek(fp, DAT_HDR_NAME, SEEK_SET);
  fread(&name, 8, 1, fp);

  /*
   * Terrain raster data
   */
  size_t nrastcells = rastwidth * rastheight;
  size_t rastmemsize = 2 * nrastcells;
  int16_t* rastcells = malloc(rastmemsize);
  fseek(fp, DAT_HDR_RASTER_START, SEEK_SET);
  size_t ret = fread(rastcells, 2, nrastcells, fp);
  fclose(fp);

  if (ret != nrastcells) {
    free(rastcells);
    return NULL;
  } else {
    Map* map = malloc(sizeof(*map));
    map->raster = malloc(rastmemsize);
    map->raster_width = rastwidth;
    map->raster_height = rastheight;
    map->width = width;
    map->height = height;
    map->utm_easting = easting;
    map->utm_northing = northing;
    map->version = version;
    memcpy(map->name, name, sizeof(name));
    memcpy(map->raster, rastcells, rastmemsize);
    free(rastcells);
    return map;
  }
}

void
map_free(Map* m)
{
  free(m->raster);
  free(m);
}

short
map_raster_read(const Map* m, short col, short row)
{
  return m->raster[row * m->raster_width + col];
}

short
map_get(const Map* m, short x, short y, short* rcol, short* rrow)
{
  short col = x / 10;
  short row = y / 10;
  if (rcol != NULL)
    *rcol = col;
  if (rrow != NULL)
    *rrow = row;
  return map_raster_read(m, col, row);
}

void
terrain_decode(short terr, TerrainType* ttype, TerrainFlags* tflags)
{
  /*
   *  MSB
   */
  switch ((terr & 0xff00) >> 8) {
    case RAW_TERRAIN_CLEAR:
      *ttype = TERRAIN_TYPE_CLEAR;
      break;
    case RAW_TERRAIN_NOGO1:
      *ttype = TERRAIN_TYPE_NOGO1;
      break;
    case RAW_TERRAIN_NOGO2:
      *ttype = TERRAIN_TYPE_NOGO2;
      break;
    case RAW_TERRAIN_NOGO3:
      *ttype = TERRAIN_TYPE_NOGO3;
      break;
    case RAW_TERRAIN_ROUGH1:
      *ttype = TERRAIN_TYPE_ROUGH1;
      break;
    case RAW_TERRAIN_ROUGH2:
      *ttype = TERRAIN_TYPE_ROUGH2;
      break;
    case RAW_TERRAIN_ROUGH3:
      *ttype = TERRAIN_TYPE_ROUGH3;
      break;
    case RAW_TERRAIN_ROUGH4:
      *ttype = TERRAIN_TYPE_ROUGH4;
      break;
    case RAW_TERRAIN_WATER:
      *ttype = TERRAIN_TYPE_WATER;
      break;
    default:
      /* XXX error! */
      *ttype = 0xff;
      break;
  }

  /*
   * LSB
   */
  *tflags = 0;
  if (terr & RAW_TERRAIN_FLAG_LOSBLOCK)
    *tflags |= TERRAIN_FLAG_LOSBLOCK;
  else
    *tflags &= ~TERRAIN_FLAG_LOSBLOCK;

  if (terr & RAW_TERRAIN_FLAG_ELEVATION)
    *tflags |= TERRAIN_FLAG_ELEVATION;
  else
    *tflags &= ~TERRAIN_FLAG_ELEVATION;

  if (terr & RAW_TERRAIN_FLAG_ROAD)
    *tflags |= TERRAIN_FLAG_ROAD;
  else
    *tflags &= ~TERRAIN_FLAG_ROAD;

  if (terr & RAW_TERRAIN_FLAG_TOWN)
    *tflags |= TERRAIN_FLAG_TOWN;
  else
    *tflags &= ~TERRAIN_FLAG_TOWN;

  if (terr & RAW_TERRAIN_FLAG_WOODS)
    *tflags |= TERRAIN_FLAG_WOODS;
  else
    *tflags &= ~TERRAIN_FLAG_WOODS;
}
