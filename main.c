#include "internal.h"

#include <glib.h>

/*
 * All data is local to the module
 */
static char* in_filename;
static char* out_filename;
static gboolean adddescr = FALSE;
static gboolean verbose = FALSE;
static gboolean grid_1km = FALSE;
static gboolean grid_5km = FALSE;
static gboolean grid_10km = FALSE;

static GOptionEntry entries[] = {
  { "input",
    'i',
    0,
    G_OPTION_ARG_STRING,
    &in_filename,
    "TacOps .dat map filename",
    "FILENAME" },
  { "out",
    'o',
    0,
    G_OPTION_ARG_STRING,
    &out_filename,
    "Output image filename",
    "FILENAME" },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL },
  { "1km",
    0,
    0,
    G_OPTION_ARG_NONE,
    &grid_1km,
    "Overlay 1-kilometer grid",
    NULL },
  { "5km",
    0,
    0,
    G_OPTION_ARG_NONE,
    &grid_5km,
    "Overlay 5-kilometer grid",
    NULL },
  { "10km",
    0,
    0,
    G_OPTION_ARG_NONE,
    &grid_10km,
    "Overlay 10-kilometer grid",
    NULL },
  { "description",
    'd',
    0,
    G_OPTION_ARG_NONE,
    &adddescr,
    "Add description text to generated image",
    NULL },
  { NULL }
};

int
main(int argc, char* argv[])
{
  GError* error = NULL;
  GOptionContext* context;

  in_filename = malloc(256);
  out_filename = malloc(256);

  // set-up options groups
  context =
    g_option_context_new("- renders TacOps .dat map terrain into a PNG file");
  g_option_context_add_main_entries(context, entries, NULL);

  // parse options
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  // decide if all mandatory options are provided
  if (in_filename[0] == '\0' || out_filename[0] == '\0') {
    g_print("%s", g_option_context_get_help(context, TRUE, NULL));
    exit(EXIT_SUCCESS);
  }

  // setup our internal app context and pass it further
  struct ProgramContext progctx;
  progctx.in_filename = in_filename;
  progctx.out_filename = out_filename;
  progctx.flags = 0;
  if (verbose == TRUE)
    progctx.flags |= FLAG_VERBOSE;
  if (adddescr == TRUE)
    progctx.flags |= FLAG_ADD_TEXT_DESCR;
  if (grid_1km == TRUE)
    progctx.flags |= FLAG_ADD_GRID_1KM;
  if (grid_5km == TRUE)
    progctx.flags |= FLAG_ADD_GRID_5KM;
  if (grid_10km == TRUE)
    progctx.flags |= FLAG_ADD_GRID_10KM;

  return program_main(progctx);
}
