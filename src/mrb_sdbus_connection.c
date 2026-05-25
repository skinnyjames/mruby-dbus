#ifndef MRUBY_DBUS_CONNECTION
#define MRUBY_DBUS_CONNECTION

#include "mrb_sdbus_connection.h"

static void mrb_sdbus_conn_type_free(mrb_state* mrb, void* payload)
{
  mrb_sdbus_conn_wrapper* wrapper = (mrb_sdbus_conn_wrapper*) payload;
  // sd_bus_close(wrapper->conn);
  free(payload);
}

static struct mrb_data_type mrb_sdbus_conn_type = { "Connection", mrb_sdbus_conn_type_free };

mrb_sdbus_conn_wrapper* mrb_sdbus_conn_get(mrb_state* mrb, mrb_value self)
{
  mrb_sdbus_conn_wrapper* wrapper = (mrb_sdbus_conn_wrapper*)DATA_PTR(self);
  if (!wrapper) {
    mrb_raise(mrb, E_ARGUMENT_ERROR , "uninitialized conn data") ;
  }
  
  return wrapper;
}

mrb_value mrb_sdbus_conn_system(mrb_state* mrb, mrb_value self)
{
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");

  int rc;
  sd_bus* conn;
  rc = sd_bus_default_system(&conn);
  if (rc < 0)
  {
    mrb_raise(mrb, errklass, strerror(rc));
  }

  mrb_value obj = mrb_funcall(mrb, self, "new", 0, NULL);
  mrb_sdbus_conn_wrapper* wrapper = (mrb_sdbus_conn_wrapper*)malloc(sizeof(mrb_sdbus_conn_wrapper));
  *wrapper = (mrb_sdbus_conn_wrapper){.conn=conn};

  mrb_data_init(obj, wrapper, &mrb_sdbus_conn_type);
  return obj;
}

mrb_value mrb_sdbus_conn_user(mrb_state* mrb, mrb_value self)
{
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");

  int rc;
  sd_bus* conn;
  rc = sd_bus_default_user(&conn);
  if (rc < 0)
  {
    mrb_raise(mrb, errklass, strerror(rc));
  }

  mrb_value obj = mrb_funcall(mrb, self, "new", 0, NULL);
  mrb_sdbus_conn_wrapper* wrapper = (mrb_sdbus_conn_wrapper*)malloc(sizeof(mrb_sdbus_conn_wrapper));
  *wrapper = (mrb_sdbus_conn_wrapper){.conn=conn};

  mrb_data_init(obj, wrapper, &mrb_sdbus_conn_type);
  return obj;
}

mrb_value mrb_sdbus_conn_introspect(mrb_state* mrb, mrb_value self)
{
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");
  mrb_sdbus_conn_wrapper* wrapper = mrb_sdbus_conn_get(mrb, self);
  char* svc_path;
  char* obj_path;

  mrb_get_args(mrb, "zz", &svc_path, &obj_path);
  sd_bus_error err = SD_BUS_ERROR_NULL;
  sd_bus_message* reply = NULL;

  int rc = sd_bus_call_method(
    wrapper->conn, 
    svc_path, 
    obj_path, 
    "org.freedesktop.DBus.Introspectable", 
    "Introspect", 
    &err,
    &reply, 
    ""
  );

  if (rc < 0)
  {
    char err_buf[512];
    snprintf(err_buf, sizeof(err_buf), "Cannot introspect: %s (%s)", 
             err.message ? err.message : "No reason provided", 
             err.name ? err.name : "UnknownError");
             
    sd_bus_error_free(&err);
    mrb_raise(mrb, errklass, err_buf);
  }

  char* xml = NULL;
  sd_bus_message_read(reply, "s", &xml);
  sd_bus_message_unref(reply);
  sd_bus_error_free(&err);

  return mrb_str_new_cstr(mrb, xml);
}


const char* mrb_sdbus_serialize_message(mrb_state* mrb, sd_bus_message* message, const char* sig, mrb_value args)
{
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");

  if (!sig || *sig == '\0') return sig;

  char type = sig[0];
  const char* next = sig + 1;

  switch (type)
  {
    case SD_BUS_TYPE_BOOLEAN:
    {
      int cval = (int)mrb_test(args);
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }    
    case SD_BUS_TYPE_UINT16:
    {
      int icval = (int)mrb_integer(args);
      if (icval < 0) mrb_raise(mrb, errklass, "Argument can't be a negative integer");
      uint16_t cval = (uint16_t)icval;
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }
    case SD_BUS_TYPE_UINT32:
    {
      int icval = (int)mrb_integer(args);
      if (icval < 0) mrb_raise(mrb, errklass, "Argument can't be a negative integer");
      uint32_t cval = (uint32_t)icval;
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }
    case SD_BUS_TYPE_UINT64:
    {
      int icval = (int)mrb_integer(args);
      if (icval < 0) mrb_raise(mrb, errklass, "Argument can't be a negative integer");

      uint64_t cval = (uint64_t)icval;
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }
    case SD_BUS_TYPE_INT16:
    {
      int64_t icval = (int64_t)mrb_integer(args);
      int16_t cval = (int16_t)icval;
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }         
    case SD_BUS_TYPE_INT32:
    {
      int64_t icval = (int64_t)mrb_integer(args);
      int32_t cval = (int32_t)icval;
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }        
    case SD_BUS_TYPE_INT64:
    {
      int64_t cval = (int64_t)mrb_integer(args);
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }
    case SD_BUS_TYPE_UNIX_FD:          
    {
      int cval = (int)mrb_integer(args);
      sd_bus_message_append_basic(message, type, &cval);
      break;
    } 
    case SD_BUS_TYPE_DOUBLE:
    {
      double cval = (double)mrb_float(args);
      sd_bus_message_append_basic(message, type, &cval);
      break;
    }     
    case SD_BUS_TYPE_STRING:          
    case SD_BUS_TYPE_OBJECT_PATH:   
    case SD_BUS_TYPE_SIGNATURE:
    {
      char* cval = mrb_str_to_cstr(mrb, args);
      sd_bus_message_append_basic(message, type, cval);
      break;
    }
    case SD_BUS_TYPE_ARRAY:
    {
      // need the contents of the array sig
      char contents[64];
      const char* tmp = next;

      if (next[0] == SD_BUS_TYPE_DICT_ENTRY_BEGIN)
      {
        int depth = 0;
        int i = 0;
        // outer tag is something like a{sai} or a{sa{si}}
        // need to capture everything inside {sa{si}}
        while (true)
        {
          contents[i] = tmp[0];
          i++;

          if (tmp[0] == SD_BUS_TYPE_DICT_ENTRY_BEGIN)
          {
            depth++;
          }
          else if(tmp[0] == SD_BUS_TYPE_DICT_ENTRY_END)
          {
            depth--;
          }

          if (depth <= 0) break;

          tmp++;
        }

        contents[i] = '\0';
        next = tmp + 1;
      }
      else
      {
        contents[0] = tmp[0];
        contents[1] = '\0';
        next = tmp + 1;
      }

      // entering something either like 'i' or '{sa{si}}'
      sd_bus_message_open_container(message, SD_BUS_TYPE_ARRAY, contents);

      char itype = contents[0];

      if (itype == SD_BUS_TYPE_DICT_ENTRY_BEGIN)
      {
        char icontents[64];
        strncpy(icontents, contents + 1, strlen(contents) - 2);
        icontents[strlen(contents) - 2] = '\0';
    
        // args is a hash...
        mrb_value hasharr = mrb_funcall(mrb, args, "to_a", 0, NULL);
        int len = RARRAY_LEN(hasharr);
        for (int i = 0; i < len; i++)
        {
          sd_bus_message_open_container(message, SD_BUS_TYPE_DICT_ENTRY, icontents);

          mrb_value arr = mrb_ary_entry(hasharr, i);
          mrb_value key = mrb_ary_entry(arr, 0);
          mrb_value value = mrb_ary_entry(arr, 1);

          const char* vsig = mrb_sdbus_serialize_message(mrb, message, icontents, key);
          mrb_sdbus_serialize_message(mrb, message, vsig, value);

          sd_bus_message_close_container(message);
        }
      }
      else
      {
        int len = RARRAY_LEN(args);
        for (int i=0; i<len; i++)
        {
          mrb_value value = mrb_ary_entry(args, i);
          mrb_sdbus_serialize_message(mrb, message, contents, value);
        }
      }

      sd_bus_message_close_container(message);
      break;
    }
    case SD_BUS_TYPE_VARIANT:
    {
      // args should be an array
      // first el should be the sig
      // second should be the value...
      char* sig = mrb_str_to_cstr(mrb, mrb_ary_entry(args, 0));
      mrb_value val = mrb_ary_entry(args, 1);

      sd_bus_message_open_container(message, type, sig);
      mrb_sdbus_serialize_message(mrb, message, sig, val);
      sd_bus_message_close_container(message);

      break;
    }
    case SD_BUS_TYPE_STRUCT_BEGIN:
    {
      char contents[64];
      const char* tmp = next;
      int depth = 1;
      int i = 0;

      while (true)
      {
        if (tmp[0] == SD_BUS_TYPE_STRUCT_BEGIN)
        {
          contents[i] = tmp[0];
          depth++;
        }
        else if(tmp[0] == SD_BUS_TYPE_STRUCT_END)
        {
          depth--;
          if (depth > 0)
          {
            contents[i] = tmp[0];
          }
        }
        else
        {
          contents[i] = tmp[0];
        }

        i++;

        if (depth <= 0) break;

        tmp++;
      }

      contents[i-1] = '\0';
      next = tmp + 1;

      sd_bus_message_open_container(message, SD_BUS_TYPE_STRUCT, contents);
      int ii = 0;
      const char* itmp = contents;
      while (itmp && *itmp != '\0')
      { 
        mrb_value value = mrb_ary_entry(args, ii);
        itmp = mrb_sdbus_serialize_message(mrb, message, itmp, value);
        ii++;
      }

      sd_bus_message_close_container(message);
      break;
    }
    default:
    {
      mrb_raise(mrb, errklass, "Can't serialize message");
    }
  }

  return next;
}

mrb_value mrb_sdbus_deserialize_message(mrb_state* mrb, sd_bus_message* message, mrb_value* hash_or_null)
{
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");
  mrb_value value;

  char type;
  const char *contents;

  if (sd_bus_message_peek_type(message, &type, &contents) > 0)
  {
    switch (type)
    {
      case SD_BUS_TYPE_BOOLEAN:
      {
        bool cval;
        sd_bus_message_read_basic(message, type, &cval);
        value = cval == false ? mrb_false_value() : mrb_true_value();
        break;
      }    
      case SD_BUS_TYPE_UINT16:
      case SD_BUS_TYPE_UINT32:
      case SD_BUS_TYPE_UINT64:
      {
        uint64_t cval;
        sd_bus_message_read_basic(message, type, &cval);
        value = mrb_fixnum_value((mrb_int) cval);
        break;
      }
      case SD_BUS_TYPE_INT16:           
      case SD_BUS_TYPE_INT32:          
      case SD_BUS_TYPE_INT64:
      case SD_BUS_TYPE_UNIX_FD:          
      {
        int64_t cval;
        sd_bus_message_read_basic(message, type, &cval);
        value = mrb_fixnum_value((mrb_int) cval);
        break;
      } 
      case SD_BUS_TYPE_DOUBLE:
      {
        double cval;
        sd_bus_message_read_basic(message, type, &cval);
        value = mrb_float_value(mrb, (mrb_float)cval);
        break;
      }     
      case SD_BUS_TYPE_STRING:          
      case SD_BUS_TYPE_OBJECT_PATH:   
      case SD_BUS_TYPE_SIGNATURE:
      {
        char* cval;
        sd_bus_message_read_basic(message, type, &cval);
        value = mrb_str_new_cstr(mrb, cval);
        break;
      }
      case SD_BUS_TYPE_ARRAY:
      {
        char itype;
        const char* icontents;
        sd_bus_message_enter_container(message, type, contents);
        if (sd_bus_message_peek_type(message, &itype, &icontents) < 0) mrb_raise(mrb, errklass, "Can't peek inside array");

        if (itype == SD_BUS_TYPE_DICT_ENTRY)
        {
          value = mrb_hash_new(mrb);
      
          while (sd_bus_message_at_end(message, 0) == 0) {
            mrb_sdbus_deserialize_message(mrb, message, &value);
          }
        }
        else
        {
          value = mrb_ary_new(mrb);

          while (sd_bus_message_at_end(message, 0) == 0) {
            mrb_value item = mrb_sdbus_deserialize_message(mrb, message, NULL);
            mrb_ary_push(mrb, value, item);
          }
        }
        sd_bus_message_exit_container(message);
        break;
      }
      case SD_BUS_TYPE_VARIANT:
      {        
        sd_bus_message_enter_container(message, type, NULL);
        value = mrb_sdbus_deserialize_message(mrb, message, NULL);
        sd_bus_message_exit_container(message);
        break;
      }
      case SD_BUS_TYPE_STRUCT:
      {
        value = mrb_ary_new(mrb);
        sd_bus_message_enter_container(message, type, contents);
        while (sd_bus_message_at_end(message, 0) == 0) {
          mrb_value item = mrb_sdbus_deserialize_message(mrb, message, NULL);
          mrb_ary_push(mrb, value, item);
        }
        sd_bus_message_exit_container(message);
        break;
      }
      case SD_BUS_TYPE_DICT_ENTRY:
      {
        if (hash_or_null)
        {
          value = *hash_or_null;
        }
        else
        {
          value = mrb_hash_new(mrb);
        }

        sd_bus_message_enter_container(message, type, contents);
        mrb_value key = mrb_sdbus_deserialize_message(mrb, message, NULL);
        mrb_value item = mrb_sdbus_deserialize_message(mrb, message, NULL);

        mrb_hash_set(mrb, value, key, item);
        sd_bus_message_exit_container(message);

        break;
      }
      default:
      {
        mrb_raise(mrb, errklass, "Can't deserialize message");
      }
    }

    return value;
  }

  mrb_raise(mrb, errklass, "Can't peek message");
}

mrb_value mrb_sdbus_conn_get_prop(mrb_state* mrb, mrb_value self)
{
  char* service;
  char* object;
  char* interface;
  char* prop_name;
  char* type;
  int rc;
  sd_bus_error err = SD_BUS_ERROR_NULL;
  sd_bus_message* reply = NULL;

  mrb_get_args(mrb, "zzzzz", &service, &object, &interface, &prop_name, &type);
  
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");

  mrb_sdbus_conn_wrapper* wrapper = mrb_sdbus_conn_get(mrb, self);

  rc = sd_bus_get_property(wrapper->conn, service, object, interface, prop_name, &err, &reply, type);
  if (rc < 0)
  {
    char err_buf[512];
    snprintf(err_buf, sizeof(err_buf), "Cannot get property: %s (%s)", 
             err.message ? err.message : "No reason provided", 
             err.name ? err.name : "UnknownError");
             
    sd_bus_error_free(&err);
    if (reply) sd_bus_message_unref(reply);
    mrb_raise(mrb, errklass, err_buf);
  }

  mrb_value ret = mrb_sdbus_deserialize_message(mrb, reply, NULL);
  sd_bus_message_unref(reply);
  return ret;
}

mrb_value mrb_sdbus_conn_call(mrb_state* mrb, mrb_value self)
{
  char* service;
  char* object;
  char* interface;
  char* method_name;
  const char* typestr;
  mrb_value args;
  int rc;
  sd_bus_error err = SD_BUS_ERROR_NULL;
  sd_bus_message* message = NULL;
  sd_bus_message* reply = NULL;

  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");
  mrb_sdbus_conn_wrapper* wrapper = mrb_sdbus_conn_get(mrb, self);

  mrb_get_args(mrb, "zzzzzo", &service, &object, &interface, &method_name, &typestr, &args);
  rc = sd_bus_message_new_method_call(wrapper->conn, &message, service, object, interface, method_name);
  if (rc < 0) mrb_raise(mrb, errklass, "Cannot construct message");

  // handle typestr
  if (!mrb_nil_p(args))
  {
    // we have args to serialize
    if (mrb_type(args) == MRB_TT_ARRAY)
    {
      int len = RARRAY_LEN(args);
      for (int i=0; i<len; i++)
      {
        mrb_value arg = mrb_ary_entry(args, i);
        typestr = mrb_sdbus_serialize_message(mrb, message, typestr, arg);
      }
    }
    else
    {
      mrb_sdbus_serialize_message(mrb, message, typestr, args);
    }
  }

  

  rc = sd_bus_call(wrapper->conn, message, 0, &err, &reply);
  sd_bus_message_unref(message);

  if (rc < 0)
  {
    char err_buf[512];
    snprintf(err_buf, sizeof(err_buf), "D-Bus Error: %s (%s)", 
             err.message ? err.message : "No reason provided", 
             err.name ? err.name : "UnknownError");
    sd_bus_error_free(&err);
    if (reply) sd_bus_message_unref(reply);
    mrb_raise(mrb, errklass, err_buf);
  }

  if (sd_bus_message_is_empty(reply) > 0)
  {
    return mrb_nil_value();
  }
  printf("has reply %s\n", method_name);
  mrb_value reply_value = mrb_sdbus_deserialize_message(mrb, reply, NULL);
  sd_bus_message_unref(reply);
  return reply_value;
}

typedef struct sdbus_signal_payload
{
  mrb_state* mrb;
  mrb_value block;
} mrb_sdbus_signal_payload;

static int mrb_sdbus_signal_cb(sd_bus_message* message, void* payload, sd_bus_error *ret_error)
{
  mrb_sdbus_signal_payload* sig = (mrb_sdbus_signal_payload*)payload;
  mrb_value ret = mrb_sdbus_deserialize_message(sig->mrb, message, NULL);

  mrb_p(sig->mrb, ret);
  mrb_funcall(sig->mrb, sig->block, "call", 1, ret);
  return 0;
}

mrb_value mrb_sdbus_on_signal(mrb_state* mrb, mrb_value self)
{  
  char* service;
  char* object;
  char* interface;
  char* method_name;
  mrb_value proc;
  mrb_get_args(mrb, "zzzzo", &service, &object, &interface, &method_name, &proc);
  struct RClass* mod = mrb_module_get(mrb, "SDBus");
  struct RClass* errklass = mrb_class_get_under(mrb, mod, "Error");
  mrb_sdbus_conn_wrapper* wrapper = mrb_sdbus_conn_get(mrb, self);

  sd_bus_slot* slot = NULL;
  // todo free this shit...
  mrb_sdbus_signal_payload* payload = malloc(sizeof(mrb_sdbus_signal_payload));
  payload->mrb = mrb;
  payload->block = proc;

  int rc = sd_bus_match_signal(wrapper->conn, &slot, service, object, interface, method_name, mrb_sdbus_signal_cb, (void*)payload);
  if (rc < 0)
  {
    printf("RC: %d\n", rc);
    mrb_raise(mrb, errklass, "BAd");
  }

  return mrb_nil_value();
}

mrb_value mrb_sdbus_process(mrb_state* mrb, mrb_value self)
{
  mrb_sdbus_conn_wrapper* wrapper = mrb_sdbus_conn_get(mrb, self);
  while (sd_bus_process(wrapper->conn, NULL) > 0) {}
  return mrb_nil_value();
}

void mrb_sdbus_conn_init(mrb_state* mrb)
{
  struct RClass* mod = mrb_define_module(mrb, "SDBus");
  struct RClass* klass = mrb_define_class_under(mrb, mod, "Connection", mrb->object_class);
  MRB_SET_INSTANCE_TT(klass, MRB_TT_DATA);

  mrb_define_class_method(mrb, klass, "user_bus", mrb_sdbus_conn_user, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass, "system_bus", mrb_sdbus_conn_system, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "introspect", mrb_sdbus_conn_introspect, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "get", mrb_sdbus_conn_get_prop, MRB_ARGS_REQ(5));
  mrb_define_method(mrb, klass, "call", mrb_sdbus_conn_call, MRB_ARGS_REQ(6));
  mrb_define_method(mrb, klass, "on", mrb_sdbus_on_signal, MRB_ARGS_REQ(5));
  mrb_define_method(mrb, klass, "next", mrb_sdbus_process, MRB_ARGS_NONE());

}
#endif