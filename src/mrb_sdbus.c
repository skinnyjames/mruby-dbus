#ifndef MRUBY_DBUS
#define MRUBY_DBUS

#include "mrb_sdbus.h"
#include "mrb_sdbus_connection.h"


void mrb_mruby_sdbus_gem_init(mrb_state* mrb)
{
  mrb_sdbus_conn_init(mrb);
}

void mrb_mruby_sdbus_gem_final(mrb_state* mrb)
{
}
#endif