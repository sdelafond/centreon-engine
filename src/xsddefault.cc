/*
** Copyright 2000-2009           Ethan Galstad
** Copyright 2009                Nagios Core Development Team and Community Contributors
** Copyright 2011-2013,2015,2017 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/xsddefault.hh"

using namespace com::centreon::engine;

static int xsddefault_status_log_fd(-1);

/******************************************************************/
/********************* INIT/CLEANUP FUNCTIONS *********************/
/******************************************************************/

/* initialize status data */
int xsddefault_initialize_status_data() {
  if (verify_config || config->status_file().empty())
    return (OK);

  if (xsddefault_status_log_fd == -1) {
    // delete the old status log (it might not exist).
    unlink(config->status_file().c_str());

    if ((xsddefault_status_log_fd = open(
                                      config->status_file().c_str(),
                                      O_WRONLY | O_CREAT,
                                      S_IRUSR | S_IWUSR | S_IRGRP)) == -1) {
      logger(logging::log_runtime_error, logging::basic)
        << "Error: Unable to open status data file '"
        << config->status_file() << "': " << strerror(errno);
      return (ERROR);
    }
    set_cloexec(xsddefault_status_log_fd);
  }
  return (OK);
}

// cleanup status data before terminating.
int xsddefault_cleanup_status_data(int delete_status_data) {
  if (verify_config)
    return (OK);

  // delete the status log.
  if (delete_status_data && !config->status_file().empty()) {
    if (unlink(config->status_file().c_str()))
      return (ERROR);
  }

  if (xsddefault_status_log_fd != -1) {
    close(xsddefault_status_log_fd);
    xsddefault_status_log_fd = -1;
  }
  return (OK);
}

/******************************************************************/
/****************** STATUS DATA OUTPUT FUNCTIONS ******************/
/******************************************************************/

/* write all status data to file */
int xsddefault_save_status_data() {
  if (xsddefault_status_log_fd == -1)
    return (OK);

  int used_external_command_buffer_slots(0);
  int high_external_command_buffer_slots(0);

  logger(logging::dbg_functions, logging::basic)
    << "save_status_data()";

  // get number of items in the command buffer
  if (config->check_external_commands()) {
    pthread_mutex_lock(&external_command_buffer.buffer_lock);
    used_external_command_buffer_slots = external_command_buffer.items;
    high_external_command_buffer_slots = external_command_buffer.high;
    pthread_mutex_unlock(&external_command_buffer.buffer_lock);
  }

  // generate check statistics
  generate_check_stats();

  std::ostringstream stream;

  time_t current_time;
  time(&current_time);

  // write version info to status file
  stream
    << "#############################################\n"
       "#        CENTREON ENGINE STATUS FILE        #\n"
       "#                                           #\n"
       "# THIS FILE IS AUTOMATICALLY GENERATED BY   #\n"
       "# CENTREON ENGINE. DO NOT MODIFY THIS FILE! #\n"
       "#############################################\n"
       "\n"
       "info {\n"
       "\tcreated=" << static_cast<unsigned long>(current_time) << "\n"
       "\t}\n\n";

  // save program status data
  stream
    << "programstatus {\n"
       "\tmodified_host_attributes=" << modified_host_process_attributes << "\n"
       "\tmodified_service_attributes=" << modified_service_process_attributes << "\n"
       "\tnagios_pid=" << static_cast<unsigned int>(getpid()) << "\n"
       "\tprogram_start=" << static_cast<long long>(program_start) << "\n"
       "\tlast_command_check=" << static_cast<long long>(last_command_check) << "\n"
       "\tlast_log_rotation=" << static_cast<long long>(last_log_rotation) << "\n"
       "\tenable_notifications=" << config->enable_notifications() << "\n"
       "\tactive_service_checks_enabled=" << config->execute_service_checks() << "\n"
       "\tpassive_service_checks_enabled=" << config->accept_passive_service_checks() << "\n"
       "\tactive_host_checks_enabled=" << config->execute_host_checks() << "\n"
       "\tpassive_host_checks_enabled=" << config->accept_passive_host_checks() << "\n"
       "\tenable_event_handlers=" << config->enable_event_handlers() << "\n"
       "\tobsess_over_services=" << config->obsess_over_services() << "\n"
       "\tobsess_over_hosts=" << config->obsess_over_hosts() << "\n"
       "\tcheck_service_freshness=" << config->check_service_freshness() << "\n"
       "\tcheck_host_freshness=" << config->check_host_freshness() << "\n"
       "\tenable_flap_detection=" << config->enable_flap_detection() << "\n"
       "\tprocess_performance_data=" << config->process_performance_data() << "\n"
       "\tglobal_host_event_handler=" << config->global_host_event_handler().c_str() << "\n"
       "\tglobal_service_event_handler=" << config->global_service_event_handler().c_str() << "\n"
       "\tnext_comment_id=" << next_comment_id << "\n"
       "\tnext_downtime_id=" << next_downtime_id << "\n"
       "\tnext_event_id=" << next_event_id << "\n"
       "\tnext_problem_id=" << next_problem_id << "\n"
       "\tnext_notification_id=" << next_notification_id << "\n"
       "\ttotal_external_command_buffer_slots=" << config->external_command_buffer_slots() << "\n"
       "\tused_external_command_buffer_slots=" << used_external_command_buffer_slots << "\n"
       "\thigh_external_command_buffer_slots=" << high_external_command_buffer_slots << "\n"
       "\tactive_scheduled_host_check_stats="
    << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\tactive_ondemand_host_check_stats="
    << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\tpassive_host_check_stats="
    << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\tactive_scheduled_service_check_stats="
    << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
       "\tactive_ondemand_service_check_stats="
    << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
       "\tpassive_service_check_stats="
    << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
       "\tcached_host_check_stats="
    << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\tcached_service_check_stats="
    << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
       "\texternal_command_stats="
    << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[0] << ","
    << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[1] << ","
    << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[2] << "\n"
       "\tparallel_host_check_stats="
    << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\tserial_host_check_stats="
    << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[0] << ","
    << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[1] << ","
    << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[2] << "\n"
       "\t}\n\n";

  /* save host status data */
  for (umap<std::string, com::centreon::shared_ptr< ::host> >::const_iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it) {
    host* hst(it->second.get());
    stream
      << "hoststatus {\n"
      "\thost_name=" << hst->get_host_name() << "\n"
         "\tmodified_attributes=" << hst->get_modified_attributes() << "\n"
         "\tcheck_command=" << hst->get_check_command_args() << "\n"
         "\tcheck_period=" << (hst->get_check_period() ? hst->get_check_period()->name : "") << "\n"
         "\tnotification_period=" << (hst->get_notification_period() ? hst->get_notification_period()->name : "") << "\n"
         "\tcheck_interval=" << hst->get_normal_check_interval() << "\n"
         "\tretry_interval=" << hst->get_retry_check_interval() << "\n"
         "\tevent_handler=" << hst->get_event_handler_args() << "\n"
         "\tget_has_been_checked=" << hst->get_has_been_checked() << "\n"
         "\tshould_be_scheduled=" << hst->get_should_be_scheduled() << "\n"
         "\tcheck_execution_time=" << std::setprecision(3) << std::fixed << hst->get_execution_time() << "\n"
         "\tcheck_latency=" << std::setprecision(3) << std::fixed << hst->get_latency() << "\n"
         "\tcheck_type=" << hst->get_check_type() << "\n"
         "\tcurrent_state=" << hst->get_current_state() << "\n"
         "\tlast_hard_state=" << hst->get_last_hard_state() << "\n"
         "\tlast_event_id=" << hst->get_last_event_id() << "\n"
         "\tcurrent_event_id=" << hst->get_current_event_id() << "\n"
         "\tcurrent_problem_id=" << hst->get_current_problem_id() << "\n"
         "\tlast_problem_id=" << hst->get_last_problem_id() << "\n"
         "\tplugin_output=" << hst->get_output() << "\n"
         "\tlong_plugin_output=" << hst->get_long_output() << "\n"
         "\tperformance_data=" << hst->get_perfdata() << "\n"
         "\tlast_check=" << static_cast<unsigned long>(hst->get_last_check()) << "\n"
         "\tnext_check=" << static_cast<unsigned long>(hst->get_next_check()) << "\n"
         "\tcheck_options=" << hst->get_check_options() << "\n"
         "\tcurrent_attempt=" << hst->get_current_attempt() << "\n"
         "\tmax_attempts=" << hst->get_max_attempts() << "\n"
         "\tstate_type=" << hst->get_current_state_type() << "\n"
         "\tlast_state_change=" << static_cast<unsigned long>(hst->get_last_state_change()) << "\n"
         "\tlast_hard_state_change=" << static_cast<unsigned long>(hst->get_last_hard_state_change()) << "\n"
         "\tlast_time_up=" << static_cast<unsigned long>(hst->get_last_time_up()) << "\n"
         "\tlast_time_down=" << static_cast<unsigned long>(hst->get_last_time_down()) << "\n"
         "\tlast_time_unreachable=" << static_cast<unsigned long>(hst->get_last_time_unreachable()) << "\n"
         "\tlast_notification=" << static_cast<unsigned long>(hst->get_last_notification()) << "\n"
         "\tnext_notification=" << static_cast<unsigned long>(hst->get_next_notification()) << "\n"
         "\tno_more_notifications=" << hst->get_no_more_notifications() << "\n"
         "\tcurrent_notification_number=" << hst->get_current_notification_number() << "\n"
         "\tcurrent_notification_id=" << hst->get_current_notification_id() << "\n"
         "\tnotifications_enabled=" << hst->get_notifications_enabled() << "\n"
         "\tproblem_has_been_acknowledged=" << hst->is_acknowledged() << "\n"
         "\tacknowledgement_type=" << hst->get_acknowledgement_type() << "\n"
         "\tactive_checks_enabled=" << hst->get_active_checks_enabled() << "\n"
         "\tpassive_checks_enabled=" << hst->get_passive_checks_enabled() << "\n"
         "\tevent_handler_enabled=" << hst->get_event_handler_enabled() << "\n"
         "\tflap_detection_enabled=" << hst->get_flap_detection_enabled() << "\n"
         "\tprocess_performance_data=" << hst->get_process_perfdata() << "\n"
         "\tobsess_over_host=" << hst->get_ocp_enabled() << "\n"
         "\tlast_update=" << static_cast<unsigned long>(current_time) << "\n"
         "\tget_flapping=" << hst->get_flapping() << "\n"
         "\tpercent_state_change=" << std::setprecision(2) << std::fixed << hst->get_percent_state_change() << "\n"
         "\tscheduled_downtime_depth=" << hst->get_scheduled_downtime_depth() << "\n";

    // Custom variables.
    for (customvar_set::const_iterator
           it(hst->get_customvars().begin()),
           end(hst->get_customvars().end());
         it != end;
         ++it)
      stream << "\t_" << it->second.get_name()
             << "=" << it->second.get_modified() << ";"
             << it->second.get_value() << "\n";
    stream << "\t}\n\n";
  }

  // save service status data
  for (umap<std::pair<std::string, std::string>, com::centreon::shared_ptr< ::service> >::const_iterator
         it(configuration::applier::state::instance().services().begin()),
         end(configuration::applier::state::instance().services().end());
       it != end;
       ++it) {
    service* svc(it->second.get());
    stream
      << "servicestatus {\n"
         "\thost_name=" << svc->get_host_name() << "\n"
         "\tservice_description=" << svc->get_description() << "\n"
         "\tmodified_attributes=" << svc->get_modified_attributes() << "\n"
         "\tcheck_command=" << svc->get_check_command_args() << "\n"
         "\tcheck_period=" << (svc->get_check_period() ? svc->get_check_period()->name : "") << "\n"
         "\tnotification_period=" << (svc->get_notification_period() ? svc->get_notification_period()->name : "") << "\n"
         "\tcheck_interval=" << svc->get_normal_check_interval() << "\n"
         "\tretry_interval=" << svc->get_retry_check_interval() << "\n"
         "\tevent_handler=" << svc->get_event_handler_args() << "\n"
         "\tget_has_been_checked=" << svc->get_has_been_checked() << "\n"
         "\tshould_be_scheduled=" << svc->get_should_be_scheduled() << "\n"
         "\tcheck_execution_time=" << std::setprecision(3) << std::fixed << svc->get_execution_time() << "\n"
         "\tcheck_latency=" << std::setprecision(3) << std::fixed << svc->get_latency() << "\n"
         "\tcheck_type=" << svc->get_check_type() << "\n"
         "\tcurrent_state=" << svc->get_current_state() << "\n"
         "\tlast_hard_state=" << svc->get_last_hard_state() << "\n"
         "\tlast_event_id=" << svc->get_last_event_id() << "\n"
         "\tcurrent_event_id=" << svc->get_current_event_id() << "\n"
         "\tcurrent_problem_id=" << svc->get_current_problem_id() << "\n"
         "\tlast_problem_id=" << svc->get_last_problem_id() << "\n"
         "\tcurrent_attempt=" << svc->get_current_attempt() << "\n"
         "\tmax_attempts=" << svc->get_max_attempts() << "\n"
         "\tstate_type=" << svc->get_current_state_type() << "\n"
         "\tlast_state_change=" << static_cast<unsigned long>(svc->get_last_state_change()) << "\n"
         "\tlast_hard_state_change=" << static_cast<unsigned long>(svc->get_last_hard_state_change()) << "\n"
         "\tlast_time_ok=" << static_cast<unsigned long>(svc->get_last_time_ok()) << "\n"
         "\tlast_time_warning=" << static_cast<unsigned long>(svc->get_last_time_warning()) << "\n"
         "\tlast_time_unknown=" << static_cast<unsigned long>(svc->get_last_time_unknown()) << "\n"
         "\tlast_time_critical=" << static_cast<unsigned long>(svc->get_last_time_critical()) << "\n"
         "\tplugin_output=" << svc->get_output() << "\n"
         "\tlong_plugin_output=" << svc->get_long_output() << "\n"
         "\tperformance_data=" << svc->get_perfdata() << "\n"
         "\tlast_check=" << static_cast<unsigned long>(svc->get_last_check()) << "\n"
         "\tnext_check=" << static_cast<unsigned long>(svc->get_next_check()) << "\n"
         "\tcheck_options=" << svc->get_check_options() << "\n"
         "\tcurrent_notification_number=" << svc->get_current_notification_number() << "\n"
         "\tcurrent_notification_id=" << svc->get_current_notification_id() << "\n"
         "\tlast_notification=" << static_cast<unsigned long>(svc->get_last_notification()) << "\n"
         "\tnext_notification=" << static_cast<unsigned long>(svc->get_next_notification()) << "\n"
         "\tno_more_notifications=" << svc->get_no_more_notifications() << "\n"
         "\tnotifications_enabled=" << svc->get_notifications_enabled() << "\n"
         "\tactive_checks_enabled=" << svc->get_active_checks_enabled() << "\n"
         "\tpassive_checks_enabled=" << svc->get_passive_checks_enabled() << "\n"
         "\tevent_handler_enabled=" << svc->get_event_handler_enabled() << "\n"
         "\tproblem_has_been_acknowledged=" << svc->is_acknowledged() << "\n"
         "\tacknowledgement_type=" << svc->get_acknowledgement_type() << "\n"
         "\tflap_detection_enabled=" << svc->get_flap_detection_enabled() << "\n"
         "\tprocess_performance_data=" << svc->get_process_perfdata() << "\n"
         "\tobsess_over_service=" << svc->get_ocp_enabled() << "\n"
         "\tlast_update=" << static_cast<unsigned long>(current_time) << "\n"
         "\tget_flapping=" << svc->get_flapping() << "\n"
         "\tpercent_state_change=" << std::setprecision(2) << std::fixed << svc->get_percent_state_change() << "\n"
         "\tscheduled_downtime_depth=" << svc->get_scheduled_downtime_depth() << "\n";

    // Custom variables.
    for (customvar_set::const_iterator
           it(svc->get_customvars().begin()),
           end(svc->get_customvars().end());
         it != end;
         ++it)
      stream << "\t_" << it->second.get_name() << "="
             << it->second.get_modified() << ";"
             << it->second.get_value() << "\n";
    stream << "\t}\n\n";
  }

  // save contact status data
  for (umap<std::string, com::centreon::shared_ptr<contact> >::const_iterator
         it(configuration::applier::state::instance().contacts().begin()),
         end(configuration::applier::state::instance().contacts().end());
       it != end;
       ++it) {
    contact* cntct = it->second.get();
    stream
      << "contactstatus {\n"
         "\tcontact_name=" << cntct->get_name() << "\n"
         "\tmodified_attributes=" << cntct->get_modified_attributes() << "\n"
         "\tmodified_host_attributes=" << cntct->get_modified_host_attributes() << "\n"
         "\tmodified_service_attributes=" << cntct->get_modified_service_attributes() << "\n"
         "\thost_notification_period=" << (cntct->get_host_notification_period() ? cntct->get_host_notification_period()->name : "") << "\n"
         "\tservice_notification_period=" << (cntct->get_service_notification_period() ? cntct->get_service_notification_period()->name : "") << "\n"
         "\tlast_host_notification=" << static_cast<unsigned long>(cntct->get_last_host_notification()) << "\n"
         "\tlast_service_notification=" << static_cast<unsigned long>(cntct->get_last_service_notification()) << "\n"
         "\thost_notifications_enabled=" << cntct->get_host_notifications_enabled() << "\n"
         "\tservice_notifications_enabled=" << cntct->get_service_notifications_enabled() << "\n";
    // custom variables
    for (customvar_set::const_iterator
           it(cntct->get_customvars().begin()),
           end(cntct->get_customvars().end());
         it != end;
         ++it) {
      customvar var(it->second);
      stream << "\t_" << var.get_name() << "=" << var.get_modified() << ";"
             << var.get_value() << "\n";
    }
    stream << "\t}\n\n";
  }

  // save all comments
  for (std::map<unsigned long, comment*>::const_iterator
         it(comment_list.begin()),
         end(comment_list.end());
       it != end;
       ++it) {
    comment* com(it->second);
    if (com->get_comment_type() == comment::HOST_COMMENT)
      stream << "hostcomment {\n";
    else
      stream << "servicecomment {\n";
    stream << "\thost_name=" << com->get_host_name() << "\n";

    if (com->get_comment_type() == comment::SERVICE_COMMENT)
      stream << "\tservice_description=" << com->get_service_description() << "\n";

    stream
      << "\tentry_type=" << com->get_entry_type() << "\n"
         "\tcomment_id=" << com->get_id() << "\n"
         "\tsource=" << com->get_source() << "\n"
         "\tpersistent=" << com->get_persistent() << "\n"
         "\tentry_time=" << static_cast<unsigned long>(com->get_entry_time()) << "\n"
         "\texpires=" << com->get_expires() << "\n"
         "\texpire_time=" << static_cast<unsigned long>(com->get_expire_time()) << "\n"
         "\tauthor=" << com->get_author() << "\n"
         "\tcomment_data=" << com->get_comment_data() << "\n"
         "\t}\n\n";
  }

  // save all downtime
  // FIXME DBR: downtimes need to be reviewed
//  for (scheduled_downtime* dt = scheduled_downtime_list; dt; dt = dt->next) {
//    if (dt->type == HOST_DOWNTIME)
//      stream << "hostdowntime {\n";
//    else
//      stream << "servicedowntime {\n";
//    stream << "\thost_name=" << dt->host_name << "\n";
//    if (dt->type == SERVICE_DOWNTIME)
//      stream << "\tservice_description=" << dt->service_description << "\n";
//    stream
//      << "\tdowntime_id=" << dt->downtime_id << "\n"
//         "\tentry_time=" << static_cast<unsigned long>(dt->entry_time) << "\n"
//         "\tstart_time=" << static_cast<unsigned long>(dt->start_time) << "\n"
//         "\tend_time=" << static_cast<unsigned long>(dt->end_time) << "\n"
//         "\ttriggered_by=" << dt->triggered_by << "\n"
//         "\tfixed=" << dt->fixed << "\n"
//         "\tduration=" << dt->duration << "\n"
//         "\tauthor=" << dt->author << "\n"
//         "\tcomment=" << dt->comment << "\n"
//         "\t}\n\n";
//  }

  // Write data in buffer.
  stream.flush();

  // Prepare status file for overwrite.
  if ((ftruncate(xsddefault_status_log_fd, 0) == -1)
      || (fsync(xsddefault_status_log_fd) == -1)
      || (lseek(xsddefault_status_log_fd, 0, SEEK_SET) == (off_t)-1)) {
    char const* msg(strerror(errno));
    logger(logging::log_runtime_error, logging::basic)
      << "Error: Unable to update status data file '"
      << config->status_file() << "': " << msg;
    return (ERROR);
  }

  // Write status file.
  std::string data(stream.str());
  char const* data_ptr(data.c_str());
  unsigned int size(data.size());
  while (size > 0) {
    ssize_t wb(write(xsddefault_status_log_fd, data_ptr, size));
    if (wb <= 0) {
      char const* msg(strerror(errno));
      logger(logging::log_runtime_error, logging::basic)
        << "Error: Unable to update status data file '"
        << config->status_file() << "': " << msg;
      return (ERROR);
    }
    data_ptr += wb;
    size -= wb;
  }

  return (OK);
}
