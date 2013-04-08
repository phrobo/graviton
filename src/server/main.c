#include <glib.h>

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  loop = g_main_loop_new (NULL, FALSE);

  g_main_loop_run (loop);
  return 0;
}
