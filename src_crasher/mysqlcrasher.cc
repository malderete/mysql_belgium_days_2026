// System / STL/ etc
#include <arpa/inet.h>  // inet_addr
#include <cstdlib>      // std::getenv
#include <netinet/in.h> // sockaddr_in
#include <string>       // std::string
#include <sys/socket.h> // AF_INET
#include <unistd.h>     // close

// MySQL "stable" API
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/service_my_plugin_log.h>

// MySQL server internals
#include "my_systime.h"          // my_micro_time
#include "sql/sql_class.h"       // THD
#include "sql/sql_lex.h"         // LEX
#include "sql/table.h"           // Table_ref


static MYSQL_PLUGIN plugin = nullptr;

/* Variables linked to the Plugin's status variables */
static unsigned long status_total_queries = 0;
static unsigned long status_total_special_queries = 0;
static unsigned long status_total_time_us = 0;
/* System variables for the plugin */
static bool plugin_enabled = true;

/**
 * @brief Initialize the plugin at server start or plugin installation
 *
 * This function performs the necessary initialization.
 *
 */
static int mysqldays_plugin_init(MYSQL_PLUGIN p) {
  plugin = p;
  my_plugin_log_message(&plugin, MY_INFORMATION_LEVEL, "Plugin init");

  return 0;
}


/**
 * @brief Deinitialize the audit user tables plugin at server shutdown or
 * plugin uninstallation
 *
 * This function performs the necessary cleanup and deinitialization steps.
 *
 * @return int Always returns 0 indicating successful deinitialization
 *
 * @retval 0 Plugin deinitialization completed successfully
 */
static int mysqldays_plugin_deinit(void *arg [[maybe_unused]]) {
  my_plugin_log_message(&plugin, MY_INFORMATION_LEVEL, "Plugin deinit");
  plugin = nullptr;

  return (0);
}


/**
*  @brief Plugin function handler. This is the CORE of the plugin.
*
*  This functions should be FAST because it is on the HOT-PATH!
*
*  @param [in] thd         Connection context.
*  @param [in] event_class Event class value.
*  @param [in] event       Event data.
*
*  @retval Value indicating, whether the server should abort continuation
*          of the current operation.
*/
static int mysqldays_notify(MYSQL_THD thd, mysql_event_class_t event_class, const void *event) {  
    // Plugin is disabled, it smells like fire around here...
    // This variable is updated by MySQL when somebody does "SET GLOBAL ..."
    if (!plugin_enabled) {
        return 0;
    }

    // Let's crash MySQL
    my_plugin_log_message(
        &plugin, MY_ERROR_LEVEL,
        "Im about to crash mysqld!");
    my_plugin_log_message(
        nullptr, MY_ERROR_LEVEL,
        "Crashing the server LOL!");

    // Short-circuit events we don't care about
    if (event_class != MYSQL_AUDIT_QUERY_CLASS) {
        return 0;
    }

    event_query = const_cast<mysql_event_query *>(static_cast<const mysql_event_query *>(event));
    if (event_query->event_subclass != MYSQL_AUDIT_QUERY_START) {
        // We emit a warning because this should never happen given that the plugin
        // descriptor is supposed to only take accept MYSQL_AUDIT_QUERY_START
        my_plugin_log_message(&plugin, MY_WARNING_LEVEL,"received event of wrong type %d, skipping", event_class);
        return 0;
    }

    auto start_us = my_micro_time();

    // Status varible
    __atomic_fetch_add(&status_total_queries, 1, __ATOMIC_RELAXED);


    // BEGIN SECRET SAUCE!
    const char *username = thd->security_context() != nullptr
                            ? thd->security_context()->user().str
                             : "unknown";

    if (username == nullptr) {
        username = "unknown";
    }

    for (Table_ref *this_table = thd->lex->query_tables; this_table != nullptr; this_table = this_table->next_global) {
        if (!this_table->is_base_table() && !this_table->is_view()) {
            continue;
        }


        if ((this_table->table_name != nullptr) && (this_table->table_name[0] != 0)) {
            if (std::string(this_table->table_name) == "special_table") {
                // Status variable
                __atomic_fetch_add(&status_total_special_queries, 1, __ATOMIC_RELAXED);
            };
        }
    }
    // END SECRET SAUCE!

    // Status variable
    auto elapsed_us = my_micro_time() - start_us;
    __atomic_fetch_add(&status_total_time_us, elapsed_us, __ATOMIC_RELAXED);

    return 0;
}

/*
  Plugin type-specific descriptor
*/
static st_mysql_audit mysqldays_descriptor = {
    MYSQL_AUDIT_INTERFACE_VERSION, /* interface version    */
    nullptr,                       /* release_thd function */
    mysqldays_notify,      /* notify function      */
    {
        0, /* general */
        0, /* connection */
        0, /* parse */
        0, /* authorization */
        0, /* table access */
        0, /* global variables */
        0, /* server startup */
        0, /* server shutdown */
        0, /* command */
        static_cast<unsigned long>(MYSQL_AUDIT_QUERY_START), /* query */
        0, /* stored program */
        0, /* authentication */
        0  /* audit*/
    }};

/*
 Plugin system variables (configuration)
 */
static MYSQL_SYSVAR_BOOL(
    enabled,             /* Name */
    plugin_enabled,      /* Variable */
    PLUGIN_VAR_RQCMDARG, /* Options, runtime modifiable only */
    "Enable or disable mysqldays plugin.", /* Description */
    nullptr,                                       /* Check function */
    nullptr,                                       /* Update function*/
    true                                           /* Default value*/
);

static SYS_VAR *system_variables[] = {MYSQL_SYSVAR(enabled), nullptr};

/*
  Plugin status variables for SHOW STATUS
*/
static SHOW_VAR statvars[] = {
    {"mysqldays_total_queries",
     reinterpret_cast<char *>(&status_total_queries), SHOW_LONG,
     SHOW_SCOPE_GLOBAL},
    {"mysqldays_total_special_queries",
     reinterpret_cast<char *>(&status_total_special_queries), SHOW_LONG,
     SHOW_SCOPE_GLOBAL},
    {"mysqldays_total_time_us",
     reinterpret_cast<char *>(&status_total_time_us), SHOW_LONG,
     SHOW_SCOPE_GLOBAL},
    // Mandatory null entry at the end of the array
    {nullptr, nullptr, SHOW_UNDEF, SHOW_SCOPE_GLOBAL}};

/*
  Plugin library descriptor
*/
mysql_declare_plugin(audit_user_tables){
    MYSQL_AUDIT_PLUGIN,             /* type                            */
    &mysqldays_descriptor,          /* descriptor                      */
    "mysqlcrasher",                    /* name                            */
    "Martin Alderete",              /* author                          */
    "Example plugin for MySQL Belgium days 2026", /* description       */
    PLUGIN_LICENSE_GPL,
    mysqldays_plugin_init,          /* init function (when loaded)     */
    nullptr,                        /* check uninstall function        */
    mysqldays_plugin_deinit,        /* deinit function (when unloaded) */
    0x0001,                         /* version                         */
    statvars,                       /* status variables                */
    system_variables,               /* system variables                */
    nullptr,                        /* reserved */
    0,                              /* flags*/
} mysql_declare_plugin_end;



