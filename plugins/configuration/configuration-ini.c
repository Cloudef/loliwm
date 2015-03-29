#include <orbment/plugin.h>
#include <chck/xdg/xdg.h>
#include <chck/string/string.h>
#include "ciniparser/ciniparser.h"
#include "config.h"

static bool (*add_configuration_backend)(const struct function *get);
static bool (*get)(void *value_out, const char *key, char type);

static dictionary *dict; 

static bool
ini_get(void *value_out, const char *key, const char type)
{
   if (!dict)
      return false;

   if (!ciniparser_find_entry(dict, key))
      return false;

   if (!value_out)
      return true;

   switch(type) {
      case 'i':
         *(int*)value_out = ciniparser_getint(dict, key, 0);
         break;
   }

   return true;
}

static char *
get_config_path(void)
{
   const char *config_dir = xdg_get_path("XDG_CONFIG_HOME", ".config");
   static const char *suffix = "orbment/orbment.ini";
   struct chck_string path = {0};

   if (chck_cstr_is_empty(config_dir))
      return NULL;

   if (!chck_string_set_format(&path, "%s/%s", config_dir, suffix))
      return NULL;

   return path.data;
}

bool
plugin_init(plugin_h self)
{
   plugin_h configuration;
   if (!(configuration = import_plugin(self, "configuration"))) {

      return false;
   }

   if (!(add_configuration_backend = import_method(self, configuration, "add_configuration_backend", "b(fun)|1")))
      return false;
   if (!(get = import_method(self, configuration, "get", "b(v,c[],c)|1")))
      return false;
   if (!add_configuration_backend(FUN(ini_get, "b(v,c[],c)|1")))
      return false;

   dict = ciniparser_load(get_config_path());

   return true;
}

const struct plugin_info*
plugin_register(void)
{
   static const char *requires[] = {
      "configuration",
      NULL,
   };

   static const char *provides[] = {
      "configuration-backend",
      NULL,
   };

   static const char *groups[] = {
      "configuration",
      NULL,
   };

   static const struct plugin_info info = {
      .name = "configuration-ini",
      .description = "Configuration backend for the INI format.",
      .version = VERSION,
      .requires = requires,
      .provides = provides,
      .groups = groups,
   };

   return &info;
}
