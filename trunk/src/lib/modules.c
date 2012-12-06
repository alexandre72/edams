/*
 * modules.c
 * This file is part of EDAMS
 *
 * Copyright (C) 2012 - Alexandre Dussart
 *
 * EDAMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EDAMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDAMS. If not, see <http://www.gnu.org/licenses/>.
 */


#include "modules.h"

#include "edams.h"
#include "libedams.h"
#include "path.h"
#include "rooms.h"

static Eina_List *modules = NULL;



static Eina_Bool
list_cb(Eina_Module *m, void *data)
{
    const char *file;
    void (*print_infos)(void);
    char *(*_module_name_get)(void);
    char *(*_module_desc_get)(void);
    char *(*_module_id_get)(void);
    char *(*_module_theme_get)(void);
    Evas_Object *(*_module_settings_get)(App_Info *app);
    Module_Info *module = NULL;

    file = eina_module_file_get(m);


    if (!eina_module_load(m))
    {
		ERR(_("Can't load module %s: %s"), file, eina_error_msg_get(eina_error_get()));
		return EINA_FALSE;
    }

    module = calloc(1, sizeof(Module_Info));
    print_infos = eina_module_symbol_get(m, "_print_module_infos");
   if (!print_infos )
   {
		ERR(_("Can't find _print_module_infos() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
   }
    print_infos();

    _module_id_get = eina_module_symbol_get(m, "_module_id_get");
    if (!_module_id_get)
    {
		ERR(_("Can't find _module_id_get() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
    }


   _module_name_get = eina_module_symbol_get(m, "_module_name_get");
   if (!_module_name_get)
   {
		ERR(_("Can't find _module_name_get() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
   }

   _module_desc_get = eina_module_symbol_get(m, "_module_desc_get");
   if (!_module_desc_get)
   {
		ERR(_("Can't find _module_desc_get() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
   }

    _module_theme_get = eina_module_symbol_get(m, "_module_theme_get");
   if (!_module_theme_get)
   {
		ERR(_("Can't find _module_theme_get() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
   }

   _module_settings_get = eina_module_symbol_get(m, "_module_settings_get");
   if (!_module_settings_get)
   {
		ERR(_("Can't find _module_settings_get() in module %s: %s!"),file, eina_error_msg_get(eina_error_get()));
		eina_module_unload(m);
		return EINA_FALSE;
   }

	Evas_Object *p = (Evas_Object *)data;
   	//FIX: should get icon from module!.
	App_Info *app = (App_Info *)evas_object_data_get(p, "App_Info");

	module->id = strdup(_module_id_get());
	module->name = strdup(_module_name_get());
	module->icon = strdup(_module_theme_get());
	module->description = strdup(_module_desc_get());
	module->settings = _module_settings_get(app);
	modules = eina_list_append(modules, module);

   return EINA_TRUE;
}


Module_Info *
module_free(Module_Info *module)
{
    if(module)
    {
		//INF("\tFreeing %s", module->name);

	    if(module->name)
	    FREE(module->name);

	    if(module->description)
	    FREE(module->description);

	    if(module->icon)
	    FREE(module->icon);

	    FREE(module);

	    return NULL;
    }
    return NULL;
}


Eina_List *
modules_list_free(Eina_List *modules)
{
	if(modules)
	{
    	Module_Info *mod;

        //Point to first node of list.
        for(modules = eina_list_last(modules); modules; modules = eina_list_prev(modules));

        EINA_LIST_FREE(modules, mod)
            mod = module_free(mod);

        eina_list_free(modules);
	    return NULL;
    }

    return NULL;
}


Eina_List *
modules_list_get(Evas_Object *parent)
{
   	Eina_Array *_modules = NULL;

   _modules = eina_module_list_get(_modules, edams_modules_path_get(), EINA_TRUE, &list_cb, parent);

  	if(!_modules)
		ERR(_("Can't get modules list!"));

  	//TODO: should be in shutdown edams to unload all loaded module.
   //eina_module_list_free(_modules);
   // if(!file);
   //eina_module_unload(m);

	return modules;
}
