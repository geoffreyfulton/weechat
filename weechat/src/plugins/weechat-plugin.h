/*
 * Copyright (c) 2003-2005 by FlashCode <flashcode@flashtux.org>
 * See README for License detail, AUTHORS for developers list.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* This header is designed to be distributed with WeeChat plugins */

#ifndef __WEECHAT_WEECHAT_PLUGIN_H
#define __WEECHAT_WEECHAT_PLUGIN_H 1

typedef struct t_plugin_dcc_info t_plugin_dcc_info;

struct t_plugin_dcc_info
{
    char *server;                       /* irc server                          */
    char *channel;                      /* irc channel (for DCC chat only)     */
    int type;                           /* DCC type (send or receive)          */
    int status;                         /* DCC status (waiting, sending, ..)   */
    time_t start_time;                  /* the time when DCC started           */
    time_t start_transfer;              /* the time when DCC transfer started  */
    unsigned long addr;                 /* IP address                          */
    int port;                           /* port                                */
    char *nick;                         /* remote nick                         */
    char *filename;                     /* filename (given by sender)          */
    char *local_filename;               /* local filename (with path)          */
    int filename_suffix;                /* suffix (.1 for ex) if renaming file */
    unsigned long size;                 /* file size                           */
    unsigned long pos;                  /* number of bytes received/sent       */
    unsigned long start_resume;         /* start of resume (in bytes)          */
    unsigned long bytes_per_sec;        /* bytes per second                    */
    t_plugin_dcc_info *prev_dcc;        /* link to previous dcc file/chat      */
    t_plugin_dcc_info *next_dcc;        /* link to next dcc file/chat          */
};

typedef struct t_weechat_plugin t_weechat_plugin;

typedef int (t_plugin_handler_func) (t_weechat_plugin *, char *, char *, char *, char *, void *);

/* message handler, called when an IRC messages is received */

typedef struct t_plugin_msg_handler t_plugin_msg_handler;

struct t_plugin_msg_handler
{
    char *irc_command;                  /* name of IRC command (PRIVMSG, ..)   */
    t_plugin_handler_func *msg_handler; /* pointer to message handler          */
    char *msg_handler_args;             /* arguments sent to message handler   */
    void *msg_handler_pointer;          /* pointer sent to message handler     */
    
    int running;                        /* 1 if currently running              */
                                        /* (used to prevent circular call)     */
    t_plugin_msg_handler *prev_handler; /* link to previous handler            */
    t_plugin_msg_handler *next_handler; /* link to next handler                */
};

/* command handler, to add new commands to WeeChat */

typedef struct t_plugin_cmd_handler t_plugin_cmd_handler;

struct t_plugin_cmd_handler
{
    char *command;                      /* name of command (without first '/') */
    char *description;                  /* (for /help) short cmd description   */
    char *arguments;                    /* (for /help) command arguments       */
    char *arguments_description;        /* (for /help) args long description   */
    /* command handler */
    t_plugin_handler_func *cmd_handler; /* pointer to command handler          */
    char *cmd_handler_args;             /* arguments sent to command handler   */
    void *cmd_handler_pointer;          /* pointer sent to command handler     */
    
    int running;                        /* 1 if currently running              */
                                        /* (used to prevent circular call)     */
    t_plugin_cmd_handler *prev_handler; /* link to previous handler            */
    t_plugin_cmd_handler *next_handler; /* link to next handler                */
};

/* plugin, a WeeChat plugin, which is a dynamic library */

struct t_weechat_plugin
{
    /* plugin variables */
    char *filename;                     /* name of plugin on disk              */
    void *handle;                       /* handle of plugin (given by dlopen)  */
    char *name;                         /* plugin name                         */
    char *description;                  /* plugin description                  */
    char *version;                      /* plugin version                      */
    
    /* plugin handlers */
    t_plugin_msg_handler *msg_handlers; /* IRC message handlers                */
    t_plugin_msg_handler *last_msg_handler;
    t_plugin_cmd_handler *cmd_handlers; /* command handlers                    */
    t_plugin_cmd_handler *last_cmd_handler;

    /* links to previous/next plugins */
    t_weechat_plugin *prev_plugin;      /* link to previous plugin             */
    t_weechat_plugin *next_plugin;      /* link to next plugin                 */
    
    /* plugin functions (interface) */
    
    /* IMPORTANT NOTE for WeeChat developers: always add new interface functions
       at the END of functions, for keeping backward compatibility with
       existing plugins */
    
    int (*ascii_strcasecmp) (t_weechat_plugin *, char *, char *);
    char **(*explode_string) (t_weechat_plugin *, char *, char *, int, int *);
    void (*free_exploded_string) (t_weechat_plugin *, char **);
    int (*mkdir_home) (t_weechat_plugin *, char *);
    void (*exec_on_files) (t_weechat_plugin *, char *,
                           int (*)(t_weechat_plugin *, char *));

    t_plugin_msg_handler *(*msg_handler_add) (t_weechat_plugin *, char *,
                                              t_plugin_handler_func *,
                                              char *, void *);
    void (*msg_handler_remove) (t_weechat_plugin *, t_plugin_msg_handler *);
    void (*msg_handler_remove_all) (t_weechat_plugin *);
    t_plugin_cmd_handler *(*cmd_handler_add) (t_weechat_plugin *, char *,
                                              char *, char *, char *,
                                              t_plugin_handler_func *,
                                              char *, void *);
    void (*cmd_handler_remove) (t_weechat_plugin *, t_plugin_cmd_handler *);
    void (*cmd_handler_remove_all) (t_weechat_plugin *);
    
    void (*printf) (t_weechat_plugin *, char *, char *, char *, ...);
    void (*printf_server) (t_weechat_plugin *, char *, ...);
    void (*infobar_printf) (t_weechat_plugin *, int, char *, ...);
    
    void (*exec_command) (t_weechat_plugin *, char *, char *, char *);
    char *(*get_info) (t_weechat_plugin *, char *, char *, char *);
    t_plugin_dcc_info *(*get_dcc_info) (t_weechat_plugin *);
    void (*free_dcc_info) (t_weechat_plugin *, t_plugin_dcc_info *);
    
    char *(*get_config) (t_weechat_plugin *, char *);
    
    /* WeeChat developers: ALWAYS add new functions at the end */
};

/* general useful functions */
extern int weechat_ascii_strcasecmp (t_weechat_plugin *,char *, char *);
extern char **weechat_explode_string (t_weechat_plugin *, char *, char *, int, int *);
extern void weechat_free_exploded_string (t_weechat_plugin *, char **);
extern int weechat_plugin_mkdir_home (t_weechat_plugin *, char *);
extern void weechat_plugin_exec_on_files (t_weechat_plugin *, char *,
                                          int (*)(t_weechat_plugin *, char *));

/* handler functions */
extern t_plugin_msg_handler *weechat_plugin_msg_handler_add (t_weechat_plugin *, char *,
                                                             t_plugin_handler_func *,
                                                             char *, void *);
extern void weechat_plugin_msg_handler_remove (t_weechat_plugin *, t_plugin_msg_handler *);
extern void weechat_plugin_msg_handler_remove_all (t_weechat_plugin *);
extern t_plugin_cmd_handler *weechat_plugin_cmd_handler_add (t_weechat_plugin *, char *,
                                                             char *, char *, char *,
                                                             t_plugin_handler_func *,
                                                             char *, void *);
extern void weechat_plugin_cmd_handler_remove (t_weechat_plugin *, t_plugin_cmd_handler *);
extern void weechat_plugin_cmd_handler_remove_all (t_weechat_plugin *);

/* display functions */
extern void weechat_plugin_printf (t_weechat_plugin *, char *, char *, char *, ...);
extern void weechat_plugin_printf_server (t_weechat_plugin *, char *, ...);
extern void weechat_plugin_infobar_printf (t_weechat_plugin *, int, char *, ...);

/* IRC functions */
extern void weechat_plugin_exec_command (t_weechat_plugin *, char *, char *, char *);
extern char *weechat_plugin_get_info (t_weechat_plugin *, char *, char *, char *);
extern t_plugin_dcc_info *weechat_plugin_get_dcc_info (t_weechat_plugin *);
extern void weechat_plugin_free_dcc_info (t_weechat_plugin *, t_plugin_dcc_info *);

/* other functions */
extern char *weechat_plugin_get_config (t_weechat_plugin *, char *);

#endif /* weechat-plugin.h */
