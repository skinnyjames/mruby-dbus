#ifndef MRUBY_DBUS_CONNECTION_H
#define MRUBY_DBUS_CONNECTION_H

#include <basu/sd-bus.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>

typedef struct SDBusConnWrapper {
  sd_bus* conn;
} mrb_sdbus_conn_wrapper;


mrb_sdbus_conn_wrapper* mrb_sdbus_conn_get(mrb_state* mrb, mrb_value self);
void mrb_sdbus_conn_init(mrb_state* mrb);

#endif