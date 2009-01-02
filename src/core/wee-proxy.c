/*
 * Copyright (c) 2003-2008 by FlashCode <flashcode@flashtux.org>
 * See README for License detail, AUTHORS for developers list.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* wee-proxy.c: proxy functions */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "weechat.h"
#include "wee-proxy.h"
#include "wee-config.h"
#include "wee-log.h"
#include "wee-string.h"


char *proxy_option_string[PROXY_NUM_OPTIONS] =
{ "type", "ipv6", "address", "port", "username", "password" };
char *proxy_type_string[PROXY_NUM_TYPES] =
{ "http", "socks4", "socks5" };

struct t_proxy *weechat_proxies = NULL;    /* first proxy                   */
struct t_proxy *last_weechat_proxy = NULL; /* last proxy                    */

struct t_proxy *weechat_temp_proxies = NULL;    /* proxies used when        */
struct t_proxy *last_weechat_temp_proxy = NULL; /* reading config           */


/*
 * proxy_search_option search a proxy option name
 *                     return index of option in array
 *                     "proxy_option_string", or -1 if not found
 */

int
proxy_search_option (const char *option_name)
{
    int i;
    
    if (!option_name)
        return -1;
    
    for (i = 0; i < PROXY_NUM_OPTIONS; i++)
    {
        if (string_strcasecmp (proxy_option_string[i], option_name) == 0)
            return i;
    }
    
    /* proxy option not found */
    return -1;
}

/*
 * proxy_search_type: search type number with string
 *                    return -1 if type is not found
 */

int
proxy_search_type (const char *type)
{
    int i;
    
    if (!type)
        return -1;
    
    for (i = 0; i < PROXY_NUM_TYPES; i++)
    {
        if (string_strcasecmp (proxy_type_string[i], type) == 0)
            return i;
    }
    
    /* type not found */
    return -1;
}

/*
 * proxy_search: search a proxy by name
 */

struct t_proxy *
proxy_search (const char *name)
{
    struct t_proxy *ptr_proxy;
    
    if (!name || !name[0])
        return NULL;
    
    for (ptr_proxy = weechat_proxies; ptr_proxy;
         ptr_proxy = ptr_proxy->next_proxy)
    {
        if (strcmp (ptr_proxy->name, name) == 0)
            return ptr_proxy;
    }
    
    /* proxy not found */
    return NULL;
}

/*
 * proxy_search_with_option_name: search a proxy with name of option
 *                                (like "local_proxy.address")
 */

struct t_proxy *
proxy_search_with_option_name (const char *option_name)
{
    char *proxy_name, *pos_option;
    struct t_proxy *ptr_proxy;
    
    ptr_proxy = NULL;
    
    pos_option = strchr (option_name, '.');
    if (pos_option)
    {
        proxy_name = string_strndup (option_name, pos_option - option_name);
        if (proxy_name)
        {
            for (ptr_proxy = weechat_proxies; ptr_proxy;
                 ptr_proxy = ptr_proxy->next_proxy)
            {
                if (strcmp (ptr_proxy->name, proxy_name) == 0)
                    break;
            }
            free (proxy_name);
        }
    }
    
    return ptr_proxy;
}

/*
 * proxy_set_name: set name for a proxy
 */

void
proxy_set_name (struct t_proxy *proxy, const char *name)
{
    int length;
    char *option_name;
    
    if (!name || !name[0])
        return;
    
    length = strlen (name) + 64;
    option_name = malloc (length);
    if (option_name)
    {
        snprintf (option_name, length, "%s.type", name);
        config_file_option_rename (proxy->type, option_name);
        snprintf (option_name, length, "%s.ipv6", name);
        config_file_option_rename (proxy->ipv6, option_name);
        snprintf (option_name, length, "%s.address", name);
        config_file_option_rename (proxy->address, option_name);
        snprintf (option_name, length, "%s.port", name);
        config_file_option_rename (proxy->port, option_name);
        snprintf (option_name, length, "%s.username", name);
        config_file_option_rename (proxy->username, option_name);
        snprintf (option_name, length, "%s.password", name);
        config_file_option_rename (proxy->password, option_name);
        
        if (proxy->name)
            free (proxy->name);
        proxy->name = strdup (name);
        
        free (option_name);
    }
}

/*
 * proxy_set: set a property for a proxy
 *            return: 1 if ok, 0 if error
 */

int
proxy_set (struct t_proxy *proxy, const char *property, const char *value)
{
    if (!proxy || !property || !value)
        return 0;
    
    if (string_strcasecmp (property, "name") == 0)
    {
        proxy_set_name (proxy, value);
        return 1;
    }
    else if (string_strcasecmp (property, "type") == 0)
    {
        config_file_option_set (proxy->type, value, 1);
        return 1;
    }
    else if (string_strcasecmp (property, "ipv6") == 0)
    {
        config_file_option_set (proxy->ipv6, value, 1);
        return 1;
    }
    else if (string_strcasecmp (property, "address") == 0)
    {
        config_file_option_set (proxy->address, value, 1);
        return 1;
    }
    else if (string_strcasecmp (property, "port") == 0)
    {
        config_file_option_set (proxy->port, value, 1);
        return 1;
    }
    else if (string_strcasecmp (property, "username") == 0)
    {
        config_file_option_set (proxy->username, value, 1);
        return 1;
    }
    else if (string_strcasecmp (property, "password") == 0)
    {
        config_file_option_set (proxy->password, value, 1);
        return 1;
    }
    
    return 0;
}

/*
 * proxy_create_option: create an option for a proxy
 */

struct t_config_option *
proxy_create_option (const char *proxy_name, int index_option,
                     const char *value)
{
    struct t_config_option *ptr_option;
    int length;
    char *option_name;
    
    ptr_option = NULL;
    
    length = strlen (proxy_name) + 1 +
        strlen (proxy_option_string[index_option]) + 1;
    option_name = malloc (length);
    if (option_name)
    {
        snprintf (option_name, length, "%s.%s",
                  proxy_name, proxy_option_string[index_option]);
        
        switch (index_option)
        {
            case PROXY_OPTION_TYPE:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "integer",
                    N_("proxy type (http (default), socks4, socks5)"),
                    "http|socks4|socks5", 0, 0, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_OPTION_IPV6:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "boolean",
                    N_("connect to proxy using ipv6"),
                    NULL, 0, 0, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_OPTION_ADDRESS:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "string",
                    N_("proxy server address (IP or hostname)"),
                    NULL, 0, 0, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_OPTION_PORT:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "integer",
                    N_("port for connecting to proxy server"),
                    NULL, 0, 65535, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_OPTION_USERNAME:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "string",
                    N_("username for proxy server"),
                    NULL, 0, 0, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_OPTION_PASSWORD:
                ptr_option = config_file_new_option (
                    weechat_config_file, weechat_config_section_proxy,
                    option_name, "string",
                    N_("password for proxy server"),
                    NULL, 0, 0, value, NULL, 0,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                break;
            case PROXY_NUM_OPTIONS:
                break;
        }
        free (option_name);
    }
    
    return ptr_option;
}

/*
 * proxy_create_option_temp: create option for a temporary proxy (when reading
 *                           config file)
 */

void
proxy_create_option_temp (struct t_proxy *temp_proxy, int index_option,
                          const char *value)
{
    struct t_config_option *new_option;
    
    new_option = proxy_create_option (temp_proxy->name,
                                      index_option,
                                      value);
    if (new_option)
    {
        switch (index_option)
        {
            case PROXY_OPTION_TYPE:
                temp_proxy->type = new_option;
                break;
            case PROXY_OPTION_IPV6:
                temp_proxy->ipv6 = new_option;
                break;
            case PROXY_OPTION_ADDRESS:
                temp_proxy->address = new_option;
                break;
            case PROXY_OPTION_PORT:
                temp_proxy->port = new_option;
                break;
            case PROXY_OPTION_USERNAME:
                temp_proxy->username = new_option;
                break;
            case PROXY_OPTION_PASSWORD:
                temp_proxy->password = new_option;
                break;
        }
    }
}

/*
 * proxy_alloc: allocate and initialize new proxy structure
 */

struct t_proxy *
proxy_alloc (const char *name)
{
    struct t_proxy *new_proxy;
    
    new_proxy = malloc (sizeof (*new_proxy));
    if (new_proxy)
    {
        new_proxy->name = strdup (name);
        new_proxy->type = NULL;
        new_proxy->ipv6 = NULL;
        new_proxy->address = NULL;
        new_proxy->port = NULL;
        new_proxy->username = NULL;
        new_proxy->password = NULL;
        new_proxy->prev_proxy = NULL;
        new_proxy->next_proxy = NULL;
    }
    
    return new_proxy;
}

/*
 * proxy_new_with_options: create a new proxy with options
 */

struct t_proxy *
proxy_new_with_options (const char *name,
                        struct t_config_option *type,
                        struct t_config_option *ipv6,
                        struct t_config_option *address,
                        struct t_config_option *port,
                        struct t_config_option *username,
                        struct t_config_option *password)
{
    struct t_proxy *new_proxy;
    
    /* create proxy */
    new_proxy = proxy_alloc (name);
    if (new_proxy)
    {
        new_proxy->type = type;
        new_proxy->ipv6 = ipv6;
        new_proxy->address = address;
        new_proxy->port = port;
        new_proxy->username = username;
        new_proxy->password = password;
        
        /* add proxy to proxies list */
        new_proxy->prev_proxy = last_weechat_proxy;
        if (weechat_proxies)
            last_weechat_proxy->next_proxy = new_proxy;
        else
            weechat_proxies = new_proxy;
        last_weechat_proxy = new_proxy;
        new_proxy->next_proxy = NULL;
    }
    
    return new_proxy;
}

/*
 * proxy_new: create a new proxy
 */

struct t_proxy *
proxy_new (const char *name, const char *type, const char *ipv6,
           const char *address, const char *port, const char *username,
           const char *password)
{
    struct t_config_option *option_type, *option_ipv6, *option_address;
    struct t_config_option *option_port, *option_username, *option_password;
    struct t_proxy *new_proxy;
    
    if (!name || !name[0])
        return NULL;
    
    /* it's not possible to create 2 proxies with same name */
    if (proxy_search (name))
        return NULL;
    
    /* look for type */
    if (proxy_search_type (type) < 0)
        return NULL;
    
    option_type = proxy_create_option (name, PROXY_OPTION_TYPE,
                                       type);
    option_ipv6 = proxy_create_option (name, PROXY_OPTION_IPV6,
                                       ipv6);
    option_address = proxy_create_option (name, PROXY_OPTION_ADDRESS,
                                          (address) ? address : "");
    option_port = proxy_create_option (name, PROXY_OPTION_PORT,
                                       port);
    option_username = proxy_create_option (name, PROXY_OPTION_USERNAME,
                                           (username) ? username : "");
    option_password = proxy_create_option (name, PROXY_OPTION_PASSWORD,
                                           (password) ? password : "");
    
    new_proxy = proxy_new_with_options (name, option_type, option_ipv6,
                                        option_address, option_port,
                                        option_username, option_password);
    if (!new_proxy)
    {
        if (option_type)
            config_file_option_free (option_type);
        if (option_ipv6)
            config_file_option_free (option_ipv6);
        if (option_address)
            config_file_option_free (option_address);
        if (option_port)
            config_file_option_free (option_port);
        if (option_username)
            config_file_option_free (option_username);
        if (option_password)
            config_file_option_free (option_password);
    }
    
    return new_proxy;
}

/*
 * proxy_use_temp_proxies: use temp proxies (created by reading config file)
 */

void
proxy_use_temp_proxies ()
{
    struct t_proxy *ptr_temp_proxy, *next_temp_proxy;
    
    for (ptr_temp_proxy = weechat_temp_proxies; ptr_temp_proxy;
         ptr_temp_proxy = ptr_temp_proxy->next_proxy)
    {
        if (!ptr_temp_proxy->type)
            ptr_temp_proxy->type = proxy_create_option (ptr_temp_proxy->name,
                                                        PROXY_OPTION_TYPE,
                                                        "http");
        if (!ptr_temp_proxy->ipv6)
            ptr_temp_proxy->ipv6 = proxy_create_option (ptr_temp_proxy->name,
                                                        PROXY_OPTION_IPV6,
                                                        "off");
        if (!ptr_temp_proxy->address)
            ptr_temp_proxy->address = proxy_create_option (ptr_temp_proxy->name,
                                                           PROXY_OPTION_ADDRESS,
                                                           "127.0.0.1");
        if (!ptr_temp_proxy->port)
            ptr_temp_proxy->port = proxy_create_option (ptr_temp_proxy->name,
                                                        PROXY_OPTION_PORT,
                                                        "3128");
        if (!ptr_temp_proxy->username)
            ptr_temp_proxy->username = proxy_create_option (ptr_temp_proxy->name,
                                                            PROXY_OPTION_USERNAME,
                                                            "");
        if (!ptr_temp_proxy->password)
            ptr_temp_proxy->password = proxy_create_option (ptr_temp_proxy->name,
                                                            PROXY_OPTION_PASSWORD,
                                                            "");
        
        if (ptr_temp_proxy->type && ptr_temp_proxy->ipv6
            && ptr_temp_proxy->address && ptr_temp_proxy->port
            && ptr_temp_proxy->username && ptr_temp_proxy->password)
        {
            proxy_new_with_options (ptr_temp_proxy->name,
                                    ptr_temp_proxy->type,
                                    ptr_temp_proxy->ipv6,
                                    ptr_temp_proxy->address,
                                    ptr_temp_proxy->port,
                                    ptr_temp_proxy->username,
                                    ptr_temp_proxy->password);
        }
        else
        {
            if (ptr_temp_proxy->type)
            {
                config_file_option_free (ptr_temp_proxy->type);
                ptr_temp_proxy->type = NULL;
            }
            if (ptr_temp_proxy->ipv6)
            {
                config_file_option_free (ptr_temp_proxy->ipv6);
                ptr_temp_proxy->ipv6 = NULL;
            }
            if (ptr_temp_proxy->address)
            {
                config_file_option_free (ptr_temp_proxy->address);
                ptr_temp_proxy->address = NULL;
            }
            if (ptr_temp_proxy->port)
            {
                config_file_option_free (ptr_temp_proxy->port);
                ptr_temp_proxy->port = NULL;
            }
            if (ptr_temp_proxy->username)
            {
                config_file_option_free (ptr_temp_proxy->username);
                ptr_temp_proxy->username = NULL;
            }
            if (ptr_temp_proxy->password)
            {
                config_file_option_free (ptr_temp_proxy->password);
                ptr_temp_proxy->password = NULL;
            }
        }
    }
    
    /* free all temp proxies */
    while (weechat_temp_proxies)
    {
        next_temp_proxy = weechat_temp_proxies->next_proxy;
        
        if (weechat_temp_proxies->name)
            free (weechat_temp_proxies->name);
        free (weechat_temp_proxies);
        
        weechat_temp_proxies = next_temp_proxy;
    }
    last_weechat_temp_proxy = NULL;
}

/*
 * proxy_free: delete a proxy
 */

void
proxy_free (struct t_proxy *proxy)
{
    if (!proxy)
        return;
    
    /* remove proxy from proxies list */
    if (proxy->prev_proxy)
        (proxy->prev_proxy)->next_proxy = proxy->next_proxy;
    if (proxy->next_proxy)
        (proxy->next_proxy)->prev_proxy = proxy->prev_proxy;
    if (weechat_proxies == proxy)
        weechat_proxies = proxy->next_proxy;
    if (last_weechat_proxy == proxy)
        last_weechat_proxy = proxy->prev_proxy;
    
    /* free data */
    if (proxy->name)
        free (proxy->name);
    if (proxy->type)
        config_file_option_free (proxy->type);
    if (proxy->ipv6)
        config_file_option_free (proxy->ipv6);
    if (proxy->address)
        config_file_option_free (proxy->address);
    if (proxy->port)
        config_file_option_free (proxy->port);
    if (proxy->username)
        config_file_option_free (proxy->username);
    if (proxy->password)
        config_file_option_free (proxy->password);
    
    free (proxy);
}

/*
 * proxy_free_all: delete all proxies
 */

void
proxy_free_all ()
{
    while (weechat_proxies)
    {
        proxy_free (weechat_proxies);
    }
}

/*
 * proxy_print_log: print proxy infos in log (usually for crash dump)
 */

void
proxy_print_log ()
{
    struct t_proxy *ptr_proxy;
    
    for (ptr_proxy = weechat_proxies; ptr_proxy;
         ptr_proxy = ptr_proxy->next_proxy)
    {
        log_printf ("");
        log_printf ("[proxy (addr:0x%lx)]", ptr_proxy);
        log_printf ("  name . . . . . . . . . : '%s'",  ptr_proxy->name);
        log_printf ("  type . . . . . . . . . : %d (%s)",
                    CONFIG_INTEGER(ptr_proxy->type),
                    proxy_type_string[CONFIG_INTEGER(ptr_proxy->type)]);
        log_printf ("  ipv6 . . . . . . . . . : %d",    CONFIG_INTEGER(ptr_proxy->ipv6));
        log_printf ("  address. . . . . . . . : '%s'",  CONFIG_STRING(ptr_proxy->address));
        log_printf ("  port . . . . . . . . . : %d",    CONFIG_INTEGER(ptr_proxy->port));
        log_printf ("  username . . . . . . . : '%s'",  CONFIG_STRING(ptr_proxy->username));
        log_printf ("  password . . . . . . . : '%s'",  CONFIG_STRING(ptr_proxy->password));
        log_printf ("  prev_proxy . . . . . . : 0x%lx", ptr_proxy->prev_proxy);
        log_printf ("  next_proxy . . . . . . : 0x%lx", ptr_proxy->next_proxy);
    }
}
