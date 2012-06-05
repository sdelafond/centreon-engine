/*
** Copyright 2011-2012 Merethis
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

#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include "centreonengine.nsmap"
#include "soapH.h"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/globals.hh"
// #include "com/centreon/engine/logging/dumpers.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/sync.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/sretention.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Restart Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processRestart(soap* s,
                                   centreonengine__processRestartResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    sigrestart = true;
    logger(log_process_info, basic)
      << "Webservice: program restarting...";
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Shutdown Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processShutdown(soap* s,
                                    centreonengine__processShutdownResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    sigshutdown = true;
    logger(log_process_info, basic)
      << "Webservice: program shutting down...";
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Read state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__stateInformationLoad(soap* s,
                                         centreonengine__stateInformationLoadResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    read_initial_state_information();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Save state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__stateInformationSave(soap* s,
                                         centreonengine__stateInformationSaveResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    save_state_information(false);

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get whether or not the host is acknowledged.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAcknowledgementIsSet(soap* s,
                                                ns1__hostIDType* host_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = (host->acknowledgement_type != ACKNOWLEDGEMENT_NONE);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAcknowledgementType(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->acknowledgement_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the address of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAddress(soap* s,
                                   ns1__hostIDType* host_id,
                                   std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->address != NULL) {
      val = host->address;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the address of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  address Host's address.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetAddress(soap* s,
                                   ns1__hostIDType* host_id,
                                   std::string address,
                                   centreonengine__hostSetAddressResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << address << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->address;
    host->address = my_strdup(address.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if active checks are enabled on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckActiveEnabled(soap* s,
                                              ns1__hostIDType* host_id,
                                              bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->checks_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host check command.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckCommand(soap* s,
                                        ns1__hostIDType* host_id,
                                        std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->host_check_command != NULL) {
      val = host->host_check_command;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckCurrentAttempt(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->current_attempt;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckIntervalNormal(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(host->check_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckIntervalRetry(soap* s,
                                              ns1__hostIDType* host_id,
                                              unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(host->retry_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckLast(soap* s,
                                     ns1__hostIDType* host_id,
                                     unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_check;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckMaxAttempts(soap* s,
                                            ns1__hostIDType* host_id,
                                            unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->max_attempts;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the next host_id check is scheduled to run.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckNext(soap* s,
                                     ns1__hostIDType* host_id,
                                     unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->next_check;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host check options.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckOptions(soap* s,
                                        ns1__hostIDType* host_id,
                                        unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->check_options;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckPassiveEnabled(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->accept_passive_host_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host check period.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckPeriod(soap* s,
                                       ns1__hostIDType* host_id,
                                       centreonengine__hostGetCheckPeriodResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->check_period != NULL) {
      res.val = soap_new_ns1__timeperiodIDType(s, 1);
      res.val->timeperiod = host->check_period;
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the host should be scheduled.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckShouldBeScheduled(soap* s,
                                                  ns1__hostIDType* host_id,
                                                  bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->should_be_scheduled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the type of the host check.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckType(soap* s,
                                     ns1__hostIDType* host_id,
                                     unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->check_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckActiveEnabled(soap* s,
                                              ns1__hostIDType* host_id,
                                              bool enable,
                                              centreonengine__hostSetCheckActiveEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->checks_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the host check command.
 *
 *  @param[in]  s        Unused.
 *  @param[in]  host_id  Host to set data.
 *  @param[in]  command  New check command.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckCommand(soap* s,
                                        ns1__hostIDType* host_id,
                                        std::string command,
                                        centreonengine__hostSetCheckCommandResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name
        << ", " << command << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->host_check_command;
    host->host_check_command = my_strdup(command.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Check interval time.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckIntervalNormal(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned int interval,
                                               centreonengine__hostSetCheckIntervalNormalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << interval << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->check_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the retry check interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Check interval time.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckIntervalRetry(soap* s,
                                              ns1__hostIDType* host_id,
                                              unsigned int interval,
                                              centreonengine__hostSetCheckIntervalRetryResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << interval << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->retry_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  attempts  Max attempts.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckMaxAttempts(soap* s,
                                            ns1__hostIDType* host_id,
                                            unsigned int attempts,
                                            centreonengine__hostSetCheckMaxAttemptsResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << attempts << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (attempts == 0) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' bad attempts value.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->max_attempts = attempts;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host_id passive checks.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckPassiveEnabled(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool enable,
                                               centreonengine__hostSetCheckPassiveEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->accept_passive_host_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the host has been checked for circular path.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCircularPathChecked(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->circular_path_checked;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the host has circular path.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCircularPathHas(soap* s,
                                           ns1__hostIDType* host_id,
                                           bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->contains_circular_path;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host downtime depth.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetDowntimeDepth(soap* s,
                                         ns1__hostIDType* host_id,
                                         unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->scheduled_downtime_depth;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host has a pending flexible downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetDowntimeFlexPending(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->pending_flex_downtime;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host event handler.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetEventHandler(soap* s,
                                        ns1__hostIDType* host_id,
                                        std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->event_handler != NULL) {
      val = host->event_handler;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the host event handler is enabled.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetEventHandlerEnabled(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->event_handler_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the host event handler.
 *
 *  @param[in]  s             Unused.
 *  @param[in]  host_id       Host to set data.
 *  @param[in]  event_handler The event handler.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetEventHandler(soap* s,
                                        ns1__hostIDType* host_id,
                                        std::string event_handler,
                                        centreonengine__hostSetEventHandlerResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << event_handler << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->event_handler;
    host->event_handler = my_strdup(event_handler.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host_id event handler.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetEventHandlerEnabled(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool enable,
                                               centreonengine__hostSetEventHandlerEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->event_handler_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if failure prediction is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFailurePredictionEnabled(soap* s,
                                                    ns1__hostIDType* host_id,
                                                    bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->failure_prediction_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get host_id failure prediction options.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFailurePredictionOptions(soap* s,
                                                    ns1__hostIDType* host_id,
                                                    std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->failure_prediction_options != NULL) {
      val = host->failure_prediction_options;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFailurePredictionEnabled(soap* s,
                                                    ns1__hostIDType* host_id,
                                                    bool enable,
                                                    centreonengine__hostSetFailurePredictionEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->failure_prediction_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the flap detection comment ID of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionCommentID(soap* s,
                                                  ns1__hostIDType* host_id,
                                                  unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->flapping_comment_id;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionEnabled(soap* s,
                                                ns1__hostIDType* host_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->flap_detection_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host is flapping.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionIsFlapping(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->is_flapping;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnDown(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->flap_detection_on_down;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnUnreachable(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->flap_detection_on_unreachable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnUp(soap* s,
                                             ns1__hostIDType* host_id,
                                             bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->flap_detection_on_up;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the flap detection state history was updated.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionStateHistoryLastUpdate(soap* s,
                                                               ns1__hostIDType* host_id,
                                                               unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_state_history_update;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host high flap threshold.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionThresholdHigh(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->high_flap_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host low flap threshold.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionThresholdLow(soap* s,
                                                     ns1__hostIDType* host_id,
                                                     double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->low_flap_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionEnabled(soap* s,
                                                ns1__hostIDType* host_id,
                                                bool enable,
                                                centreonengine__hostSetFlapDetectionEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->flap_detection_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnDown(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool enable,
                                               centreonengine__hostSetFlapDetectionOnDownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->flap_detection_on_down = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnUnreachable(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetFlapDetectionOnUnreachableResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->flap_detection_on_unreachable = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnUp(soap* s,
                                             ns1__hostIDType* host_id,
                                             bool enable,
                                             centreonengine__hostSetFlapDetectionOnUpResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->flap_detection_on_up = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the host.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionThresholdHigh(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      double threshold,
                                                      centreonengine__hostSetFlapDetectionThresholdHighResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << threshold << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->high_flap_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the host.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionThresholdLow(soap* s,
                                                     ns1__hostIDType* host_id,
                                                     double threshold,
                                                     centreonengine__hostSetFlapDetectionThresholdLowResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << threshold << ")";

  host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->low_flap_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if freshness checks are enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessCheckEnabled(soap* s,
                                                 ns1__hostIDType* host_id,
                                                 bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->check_freshness;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessIsActive(soap* s,
                                             ns1__hostIDType* host_id,
                                             bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->is_being_freshened;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessThreshold(soap* s,
                                              ns1__hostIDType* host_id,
                                              int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->freshness_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFreshnessCheckEnabled(soap* s,
                                                 ns1__hostIDType* host_id,
                                                 bool enable,
                                                 centreonengine__hostSetFreshnessCheckEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->check_freshness = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the host freshness threshold.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFreshnessThreshold(soap* s,
                                              ns1__hostIDType* host_id,
                                              int threshold,
                                              centreonengine__hostSetFreshnessThresholdResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name << ", " << threshold << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->freshness_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the modified attributes on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetModifiedAttributes(soap* s,
                                              ns1__hostIDType* host_id,
                                              unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->modified_attributes;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host alias.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNameAlias(soap* s,
                                     ns1__hostIDType* host_id,
                                     std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->alias != NULL) {
      val = host->alias;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host display name.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNameDisplay(soap* s,
                                       ns1__hostIDType* host_id,
                                       std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->display_name != NULL) {
      val = host->display_name;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the host alias.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  alias   Host's alias.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNameAlias(soap* s,
                                     ns1__hostIDType* host_id,
                                     std::string alias,
                                     centreonengine__hostSetNameAliasResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << alias << ")";

  host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->alias;
    host->alias = my_strdup(alias.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the display name of the host.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  host_id      Host to set data.
 *  @param[in]  displayname  Host's display name.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNameDisplay(soap* s,
                                       ns1__hostIDType* host_id,
                                       std::string displayname,
                                       centreonengine__hostSetNameDisplayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name
        << ", " << displayname << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->display_name;
    host->display_name = my_strdup(displayname.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the ID of the current host_id notification.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsCurrentID(soap* s,
                                                  ns1__hostIDType* host_id,
                                                  unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->current_notification_id;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current notification number of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsCurrentNumber(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->current_notification_number;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsEnabled(soap* s,
                                                ns1__hostIDType* host_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notifications_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsFirstDelay(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(host->first_notification_delay);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the notification interval of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsInterval(soap* s,
                                                 ns1__hostIDType* host_id,
                                                 unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(host->notification_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsLast(soap* s,
                                             ns1__hostIDType* host_id,
                                             unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_host_notification;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsNext(soap* s,
                                             ns1__hostIDType* host_id,
                                             unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->next_host_notification;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnDown(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notify_on_down;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is on downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnDowntime(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notify_on_downtime;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is flappy.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnFlapping(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notify_on_flapping;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host recovers.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnRecovery(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notify_on_recovery;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnUnreachable(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->notify_on_unreachable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the notification period of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsPeriod(soap* s,
                                               ns1__hostIDType* host_id,
                                               centreonengine__hostGetNotificationsPeriodResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->notification_period != NULL) {
      res.val = soap_new_ns1__timeperiodIDType(s, 1);
      res.val->timeperiod = host->notification_period;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsEnabled(soap* s,
                                                ns1__hostIDType* host_id,
                                                bool enable,
                                                centreonengine__hostSetNotificationsEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notifications_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the time after which the first host_id notification will be sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  delay   Delay of the first notification.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsFirstDelay(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   unsigned int delay,
                                                   centreonengine__hostSetNotificationsFirstDelayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->first_notification_delay = delay;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the notification interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Notification interval.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsInterval(soap* s,
                                                 ns1__hostIDType* host_id,
                                                 unsigned int interval,
                                                 centreonengine__hostSetNotificationsIntervalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << interval << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notification_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnDown(soap* s,
                                               ns1__hostIDType* host_id,
                                               bool enable,
                                               centreonengine__hostSetNotificationsOnDownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notify_on_down = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is in downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnDowntime(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool enable,
                                                   centreonengine__hostSetNotificationsOnDowntimeResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notify_on_downtime = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is flappy.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnFlapping(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool enable,
                                                   centreonengine__hostSetNotificationsOnFlappingResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notify_on_flapping = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id recovers.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnRecovery(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool enable,
                                                   centreonengine__hostSetNotificationsOnRecoveryResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notify_on_recovery = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnUnreachable(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetNotificationsOnUnreachableResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->notify_on_unreachable = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  >Check whether or not host_id is being obsessed over.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetObsessOver(soap* s,
                                      ns1__hostIDType* host_id,
                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->obsess_over_host;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host_id obsession.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetObsessOver(soap* s,
                                      ns1__hostIDType* host_id,
                                      bool enable,
                                      centreonengine__hostSetObsessOverResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->obsess_over_host = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if perfdata processing is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPerfdataProcessingEnabled(soap* s,
                                                     ns1__hostIDType* host_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->process_performance_data;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetPerfdataProcessingEnabled(soap* s,
                                                     ns1__hostIDType* host_id,
                                                     bool enable,
                                                     centreonengine__hostSetPerfdataProcessingEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->process_performance_data = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginExecutionTime(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(host->execution_time);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host check if currently executing.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginIsExecuting(soap* s,
                                             ns1__hostIDType* host_id,
                                             bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->is_executing;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host latency.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginLatency(soap* s,
                                         ns1__hostIDType* host_id,
                                         double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->latency;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginOutput(soap* s,
                                        ns1__hostIDType* host_id,
                                        std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->plugin_output != NULL) {
      val = host->plugin_output;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginPerfdata(soap* s,
                                          ns1__hostIDType* host_id,
                                          std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (host->perf_data != NULL) {
      val = host->perf_data;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host status information are retained.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetRetainStatusInformation(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->retain_status_information;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host non status information are retained.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetRetainNonStatusInformation(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->retain_nonstatus_information;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetRetainStatusInformation(soap* s,
                                                   ns1__hostIDType* host_id,
                                                   bool enable,
                                                   centreonengine__hostSetRetainStatusInformationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->retain_status_information = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetRetainNonStatusInformation(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetRetainNonStatusInformationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->retain_nonstatus_information = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the number of services on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetServicesCount(soap* s,
                                         ns1__hostIDType* host_id,
                                         unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->total_services;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the total check interval on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetServicesTotalCheckInterval(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->total_service_check_interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on down is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnDown(soap* s,
                                       ns1__hostIDType* host_id,
                                       bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->stalk_on_down;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on unreachable is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnUnreachable(soap* s,
                                              ns1__hostIDType* host_id,
                                              bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->stalk_on_unreachable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on up is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnUp(soap* s,
                                     ns1__hostIDType* host_id,
                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->stalk_on_up;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnDown(soap* s,
                                       ns1__hostIDType* host_id,
                                       bool enable,
                                       centreonengine__hostSetStalkOnDownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->stalk_on_down = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnUnreachable(soap* s,
                                              ns1__hostIDType* host_id,
                                              bool enable,
                                              centreonengine__hostSetStalkOnUnreachableResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->stalk_on_unreachable = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on up.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnUp(soap* s,
                                     ns1__hostIDType* host_id,
                                     bool enable,
                                     centreonengine__hostSetStalkOnUpResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->stalk_on_up = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateCurrent(soap* s,
                                        ns1__hostIDType* host_id,
                                        unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->current_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the initial state of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateInitial(soap* s,
                                        ns1__hostIDType* host_id,
                                        unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->initial_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last host_id state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLast(soap* s,
                                     ns1__hostIDType* host_id,
                                     unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastChange(soap* s,
                                           ns1__hostIDType* host_id,
                                           unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_state_change;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the host was in a down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastDown(soap* s,
                                         ns1__hostIDType* host_id,
                                         unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_time_down;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the host last hard state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastHard(soap* s,
                                         ns1__hostIDType* host_id,
                                         unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_hard_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastHardChange(soap* s,
                                               ns1__hostIDType* host_id,
                                               unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_hard_state_change;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastUnreachable(soap* s,
                                                ns1__hostIDType* host_id,
                                                unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_time_unreachable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastUp(soap* s,
                                       ns1__hostIDType* host_id,
                                       unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->last_time_up;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the percent state change of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStatePercentChange(soap* s,
                                              ns1__hostIDType* host_id,
                                              ULONG64& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<time_t>(host->percent_state_change);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] val     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateType(soap* s,
                                     ns1__hostIDType* host_id,
                                     unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = host->state_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable all notifications beyond a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to get data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsBeyondEnabled(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetNotificationsBeyondEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (enable == true) {
      enable_and_propagate_notifications(host, 0, false, true, true);
    }
    else {
      disable_and_propagate_notifications(host, 0, false, true, true);
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable notifications of a host and its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnSelfAndChildrenEnabled(soap* s,
                                                                 ns1__hostIDType* host_id,
                                                                 bool enable,
                                                                 centreonengine__hostSetNotificationsOnSelfAndChildrenEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (enable == true) {
      enable_and_propagate_notifications(host, 0, true, true, false);
    }
    else {
      disable_and_propagate_notifications(host, 0, true, true, false);
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set notification period of host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  timeperiod_id     Period information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsPeriod(soap* s,
                                               ns1__hostIDType* host_id,
                                               ns1__timeperiodIDType* timeperiod_id,
                                               centreonengine__hostSetNotificationsPeriodResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name
        << ", " << timeperiod_id->timeperiod << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->notification_period;
    host->notification_period = my_strdup(timeperiod_id->timeperiod.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get whether or not the service is acknowledged.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetAcknowledgementIsSet(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = (service->acknowledgement_type != ACKNOWLEDGEMENT_NONE);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetAcknowledgementType(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->acknowledgement_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if active checks are enabled on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckActiveEnabled(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->checks_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service check command.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckCommand(soap* s,
                                           ns1__serviceIDType* service_id,
                                           std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->service_check_command != NULL) {
      val = service->service_check_command;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckCurrentAttempt(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->current_attempt;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckIntervalNormal(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(service->check_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckIntervalRetry(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(service->retry_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckLast(soap* s,
                                        ns1__serviceIDType* service_id,
                                        unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_check;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckMaxAttempts(soap* s,
                                               ns1__serviceIDType* service_id,
                                               unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->max_attempts;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the next service check is scheduled to run.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckNext(soap* s,
                                        ns1__serviceIDType* service_id,
                                        unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->next_check;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service check options.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckOptions(soap* s,
                                           ns1__serviceIDType* service_id,
                                           unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->check_options;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckPassiveEnabled(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->accept_passive_service_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckPeriod(soap* s,
                                          ns1__serviceIDType* service_id,
                                          centreonengine__serviceGetCheckPeriodResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->check_period != NULL) {
      res.val = soap_new_ns1__timeperiodIDType(s, 1);
      res.val->timeperiod = service->check_period;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the service should be scheduled.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckShouldBeScheduled(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->should_be_scheduled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the type of the service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckType(soap* s,
                                        ns1__serviceIDType* service_id,
                                        unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->check_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckActiveEnabled(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 bool enable,
                                                 centreonengine__serviceSetCheckActiveEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->checks_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the service check command.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  command           New check command.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckCommand(soap* s,
                                           ns1__serviceIDType* service_id,
                                           std::string command,
                                           centreonengine__serviceSetCheckCommandResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << command << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] service->service_check_command;
    service->service_check_command = my_strdup(command.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Check interval time.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckIntervalNormal(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned int interval,
                                                  centreonengine__serviceSetCheckIntervalNormalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << interval << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->check_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the retry check interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Check interval time.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckIntervalRetry(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 unsigned int interval,
                                                 centreonengine__serviceSetCheckIntervalRetryResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << interval << ")";

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->retry_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  attempts          Max attempts.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckMaxAttempts(soap* s,
                                               ns1__serviceIDType* service_id,
                                               unsigned int attempts,
                                               centreonengine__serviceSetCheckMaxAttemptsResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << attempts << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (attempts == 0) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service + "' bad attempts value.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->max_attempts = attempts;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host passive checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckPassiveEnabled(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  bool enable,
                                                  centreonengine__serviceSetCheckPassiveEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->accept_passive_service_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the value of a service custom variable.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[in]  variable          Custom variable.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCustomVariable(soap* s,
                                             ns1__serviceIDType* service_id,
                                             std::string variable,
                                             std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << variable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for(customvariablesmember* tmp = service->custom_variables;
        tmp != NULL;
        tmp = tmp->next){
      if (!strcasecmp(tmp->variable_name, variable.c_str())) {
        if (tmp->variable_value != NULL) {
          val = tmp->variable_value;
        }
        sync::instance().worker_finish();
        return (SOAP_OK);
      }
    }

    std::string* error = soap_new_std__string(s, 1);
    *error = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name
      + "' and variable `" + variable + "' not found.";

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
}

/**
 *  Get the service downtime depth.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetDowntimeDepth(soap* s,
                                            ns1__serviceIDType* service_id,
                                            unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->scheduled_downtime_depth;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if service has a pending flexible downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetDowntimeFlexPending(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->pending_flex_downtime;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetEventHandler(soap* s,
                                           ns1__serviceIDType* service_id,
                                           std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->event_handler != NULL) {
      val = service->event_handler;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the service event handler is enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetEventHandlerEnabled(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->event_handler_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  event_handler     The event handler.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetEventHandler(soap* s,
                                           ns1__serviceIDType* service_id,
                                           std::string event_handler,
                                           centreonengine__serviceSetEventHandlerResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << event_handler << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] service->event_handler;
    service->event_handler = my_strdup(event_handler.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetEventHandlerEnabled(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  bool enable,
                                                  centreonengine__serviceSetEventHandlerEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->event_handler_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if failure prediction is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFailurePredictionEnabled(soap* s,
                                                       ns1__serviceIDType* service_id,
                                                       bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->failure_prediction_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get service failure prediction options.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFailurePredictionOptions(soap* s,
                                                       ns1__serviceIDType* service_id,
                                                       std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->failure_prediction_options != NULL) {
      val = service->failure_prediction_options;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFailurePredictionEnabled(soap* s,
                                                       ns1__serviceIDType* service_id,
                                                       bool enable,
                                                       centreonengine__serviceSetFailurePredictionEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->failure_prediction_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the flap detection comment ID of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionCommentID(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flapping_comment_id;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionEnabled(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flap_detection_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if service is flapping.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionIsFlapping(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->is_flapping;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnCritical(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flap_detection_on_critical;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnOk(soap* s,
                                                ns1__serviceIDType* service_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flap_detection_on_ok;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnUnknown(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flap_detection_on_unknown;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnWarning(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->flap_detection_on_warning;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service high flap threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionThresholdHigh(soap* s,
                                                         ns1__serviceIDType* service_id,
                                                         double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->high_flap_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service low flap threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionThresholdLow(soap* s,
                                                        ns1__serviceIDType* service_id,
                                                        double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->low_flap_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionEnabled(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   bool enable,
                                                   centreonengine__serviceSetFlapDetectionEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->flap_detection_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnCritical(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetFlapDetectionOnCriticalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->flap_detection_on_critical = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnOk(soap* s,
                                                ns1__serviceIDType* service_id,
                                                bool enable,
                                                centreonengine__serviceSetFlapDetectionOnOkResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->flap_detection_on_ok = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnUnknown(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool enable,
                                                     centreonengine__serviceSetFlapDetectionOnUnknownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->flap_detection_on_unknown = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnWarning(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool enable,
                                                     centreonengine__serviceSetFlapDetectionOnWarningResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->flap_detection_on_warning = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionThresholdHigh(soap* s,
                                                         ns1__serviceIDType* service_id,
                                                         double threshold,
                                                         centreonengine__serviceSetFlapDetectionThresholdHighResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << threshold << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->high_flap_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionThresholdLow(soap* s,
                                                        ns1__serviceIDType* service_id,
                                                        double threshold,
                                                        centreonengine__serviceSetFlapDetectionThresholdLowResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << threshold << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->low_flap_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if freshness checks are enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessCheckEnabled(soap* s,
                                                    ns1__serviceIDType* service_id,
                                                    bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->check_freshness;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessIsActive(soap* s,
                                                ns1__serviceIDType* service_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->is_being_freshened;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessThreshold(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->freshness_threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFreshnessCheckEnabled(soap* s,
                                                    ns1__serviceIDType* service_id,
                                                    bool enable,
                                                    centreonengine__serviceSetFreshnessCheckEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->check_freshness = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the service freshness threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFreshnessThreshold(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 int threshold,
                                                 centreonengine__serviceSetFreshnessThresholdResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << threshold << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->freshness_threshold = threshold;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the modified attributes on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetModifiedAttributes(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->modified_attributes;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service display name.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNameDisplay(soap* s,
                                          ns1__serviceIDType* service_id,
                                          std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->display_name != NULL) {
      val = service->display_name;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the display name of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  name              Service's display name.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNameDisplay(soap* s,
                                          ns1__serviceIDType* service_id,
                                          std::string name,
                                          centreonengine__serviceSetNameDisplayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << name << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] service->display_name;
    service->display_name = my_strdup(name.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the ID of the current service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsCurrentID(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->current_notification_id;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current notification number of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsCurrentNumber(soap* s,
                                                         ns1__serviceIDType* service_id,
                                                         unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->current_notification_number;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsEnabled(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notifications_enabled;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsFirstDelay(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(service->first_notification_delay);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the notification interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsInterval(soap* s,
                                                    ns1__serviceIDType* service_id,
                                                    unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(service->notification_interval);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsLast(soap* s,
                                                ns1__serviceIDType* service_id,
                                                unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_notification;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsNext(soap* s,
                                                ns1__serviceIDType* service_id,
                                                unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->next_notification;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnCritical(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_critical;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is on downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnDowntime(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_downtime;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is flappy.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnFlapping(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_flapping;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service recovers.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnRecovery(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_recovery;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnUnknown(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_unknown;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnWarning(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->notify_on_warning;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the notification period of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsPeriod(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  centreonengine__serviceGetNotificationsPeriodResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->notification_period != NULL) {
      res.val = soap_new_ns1__timeperiodIDType(s, 1);
      res.val->timeperiod = service->notification_period;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsEnabled(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   bool enable,
                                                   centreonengine__serviceSetNotificationsEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notifications_enabled = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the time after which the first service notification will be sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Delay of the first notification.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsFirstDelay(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      unsigned int delay,
                                                      centreonengine__serviceSetNotificationsFirstDelayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->first_notification_delay = delay;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the notification interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Notification interval.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsInterval(soap* s,
                                                    ns1__serviceIDType* service_id,
                                                    unsigned int interval,
                                                    centreonengine__serviceSetNotificationsIntervalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << " { " << service_id->host->name << " }, " << interval << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notification_interval = interval;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnCritical(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetNotificationsOnCriticalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_critical = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is in downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnDowntime(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetNotificationsOnDowntimeResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_downtime = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is flappy.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnFlapping(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetNotificationsOnFlappingResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_flapping = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service recovers.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnRecovery(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetNotificationsOnRecoveryResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_recovery = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnUnknown(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool enable,
                                                     centreonengine__serviceSetNotificationsOnUnknownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_unknown = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnWarning(soap* s,
                                                     ns1__serviceIDType* service_id,
                                                     bool enable,
                                                     centreonengine__serviceSetNotificationsOnWarningResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->notify_on_warning = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check whether or not service is being obsessed over.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetObsessOver(soap* s,
                                         ns1__serviceIDType* service_id,
                                         bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->obsess_over_service;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable service obsession.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetObsessOver(soap* s,
                                         ns1__serviceIDType* service_id,
                                         bool enable,
                                         centreonengine__serviceSetObsessOverResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->obsess_over_service = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if perfdata processing is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPerfdataProcessingEnabled(soap* s,
                                                        ns1__serviceIDType* service_id,
                                                        bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->process_performance_data;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetPerfdataProcessingEnabled(soap* s,
                                                        ns1__serviceIDType* service_id,
                                                        bool enable,
                                                        centreonengine__serviceSetPerfdataProcessingEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->process_performance_data = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginExecutionTime(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned int& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<unsigned int>(service->execution_time);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if service check if currently executing.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginIsExecuting(soap* s,
                                                ns1__serviceIDType* service_id,
                                                bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->is_executing;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service latency.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginLatency(soap* s,
                                            ns1__serviceIDType* service_id,
                                            double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->latency;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginOutput(soap* s,
                                           ns1__serviceIDType* service_id,
                                           std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->plugin_output != NULL) {
      val = service->plugin_output;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginPerfdata(soap* s,
                                             ns1__serviceIDType* service_id,
                                             std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (service->perf_data != NULL) {
      val = service->perf_data;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if service status information are retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetRetainStatusInformation(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->retain_status_information;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if service non status information are retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetRetainNonStatusInformation(soap* s,
                                                         ns1__serviceIDType* service_id,
                                                         bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->retain_nonstatus_information;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetRetainStatusInformation(soap* s,
                                                      ns1__serviceIDType* service_id,
                                                      bool enable,
                                                      centreonengine__serviceSetRetainStatusInformationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->retain_status_information = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetRetainNonStatusInformation(soap* s,
                                                         ns1__serviceIDType* service_id,
                                                         bool enable,
                                                         centreonengine__serviceSetRetainNonStatusInformationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->retain_nonstatus_information = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on critical is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnCritical(soap* s,
                                              ns1__serviceIDType* service_id,
                                              bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->stalk_on_critical;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on ok is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnOk(soap* s,
                                        ns1__serviceIDType* service_id,
                                        bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->stalk_on_ok;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on unknown is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnUnknown(soap* s,
                                             ns1__serviceIDType* service_id,
                                             bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->stalk_on_unknown;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if stalking on warning is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnWarning(soap* s,
                                             ns1__serviceIDType* service_id,
                                             bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->stalk_on_warning;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnCritical(soap* s,
                                              ns1__serviceIDType* service_id,
                                              bool enable,
                                              centreonengine__serviceSetStalkOnCriticalResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->stalk_on_critical = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on ok.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnOk(soap* s,
                                        ns1__serviceIDType* service_id,
                                        bool enable,
                                        centreonengine__serviceSetStalkOnOkResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->stalk_on_ok = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnUnknown(soap* s,
                                             ns1__serviceIDType* service_id,
                                             bool enable,
                                             centreonengine__serviceSetStalkOnUnknownResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->stalk_on_unknown = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnWarning(soap* s,
                                             ns1__serviceIDType* service_id,
                                             bool enable,
                                             centreonengine__serviceSetStalkOnWarningResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << service_id->service
 << " { " << service_id->host->name << " }, " << enable << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->stalk_on_warning = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateCurrent(soap* s,
                                           ns1__serviceIDType* service_id,
                                           unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->current_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the initial state of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateInitial(soap* s,
                                           ns1__serviceIDType* service_id,
                                           unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->initial_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last service state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLast(soap* s,
                                        ns1__serviceIDType* service_id,
                                        unsigned short& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastChange(soap* s,
                                              ns1__serviceIDType* service_id,
                                              unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_state_change;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastCritical(soap* s,
                                                ns1__serviceIDType* service_id,
                                                unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_time_critical;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the service last hard state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastHard(soap* s,
                                            ns1__serviceIDType* service_id,
                                            unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_hard_state;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastHardChange(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_hard_state_change;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastOk(soap* s,
                                          ns1__serviceIDType* service_id,
                                          unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_time_ok;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastUnknown(soap* s,
                                               ns1__serviceIDType* service_id,
                                               unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_time_unknown;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastWarning(soap* s,
                                               ns1__serviceIDType* service_id,
                                               unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->last_time_warning;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the percent state change of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStatePercentChange(soap* s,
                                                 ns1__serviceIDType* service_id,
                                                 ULONG64& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = static_cast<time_t>(service->percent_state_change);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateType(soap* s,
                                        ns1__serviceIDType* service_id,
                                        unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = service->state_type;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a host.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  host_id              Host to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnHostAdd(soap* s,
                                             ns1__hostIDType* host_id,
                                             ns1__acknowledgementType* acknowledgement_type,
                                             centreonengine__acknowledgementOnHostAddResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << acknowledgement_type->author
      << ", " << acknowledgement_type->comment
      << ", " << acknowledgement_type->notify
      << ", " << acknowledgement_type->persistent
      << ", " << acknowledgement_type->sticky << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

    char* author = my_strdup(acknowledgement_type->author.c_str());
    char* comment = my_strdup(acknowledgement_type->comment.c_str());

    acknowledge_host_problem(host,
                             author,
                             comment,
                             type,
                             acknowledgement_type->notify,
                             acknowledgement_type->persistent);

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnHostRemove(soap* s,
                                                ns1__hostIDType* host_id,
                                                centreonengine__acknowledgementOnHostRemoveResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    remove_host_acknowledgement(host);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a service.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  service_id           Service to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnServiceAdd(soap* s,
                                                ns1__serviceIDType* service_id,
                                                ns1__acknowledgementType* acknowledgement_type,
                                                centreonengine__acknowledgementOnServiceAddResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << acknowledgement_type->author
      << ", " << acknowledgement_type->comment
      << ", " << acknowledgement_type->notify
      << ", " << acknowledgement_type->persistent
      << ", " << acknowledgement_type->sticky << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

    char* author = my_strdup(acknowledgement_type->author.c_str());
    char* comment = my_strdup(acknowledgement_type->comment.c_str());

    acknowledge_service_problem(service,
                                author,
                                comment,
                                type,
                                acknowledgement_type->notify,
                                acknowledgement_type->persistent);
    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnServiceRemove(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   centreonengine__acknowledgementOnServiceRemoveResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    remove_service_acknowledgement(service);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Process a host check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostProcessResult(soap* s,
                                           ns1__hostIDType* host_id,
                                           ns1__checkResultType* result_type,
                                           centreonengine__checkHostProcessResultResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << result_type->retval << ", ... })";

    logger(dbg_checks, most)
      << "Webservice: call " << __func__ << ": output=" << result_type->output;

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (process_passive_host_check(time(NULL),
                                   host->name,
                                   result_type->retval,
                                   result_type->output.c_str()) == ERROR) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' check process result failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostSchedule(soap* s,
                                      ns1__hostIDType* host_id,
                                      long delay,
                                      centreonengine__checkHostScheduleResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_host_check(host, delay, CHECK_OPTION_NONE);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleForced(soap* s,
                                            ns1__hostIDType* host_id,
                                            long delay,
                                            centreonengine__checkHostScheduleForcedResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_host_check(host, delay, CHECK_OPTION_FORCE_EXECUTION);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleServices(soap* s,
                                              ns1__hostIDType* host_id,
                                              long delay,
                                              centreonengine__checkHostScheduleServicesResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_NONE);
      }
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule's delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleServicesForced(soap* s,
                                                    ns1__hostIDType* host_id,
                                                    long delay,
                                                    centreonengine__checkHostScheduleServicesForcedResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_FORCE_EXECUTION);
      }
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Process a service check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceProcessResult(soap* s,
                                              ns1__serviceIDType* service_id,
                                              ns1__checkResultType* result_type,
                                              centreonengine__checkServiceProcessResultResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << result_type->retval << ", ... })";

    logger(dbg_checks, most)
      << "Webservice: call " << __func__ << ": output=" << result_type->output;

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (process_passive_service_check(time(NULL),
                                      service_id->host->name.c_str(),
                                      service->description,
                                      result_type->retval,
                                      result_type->output.c_str()) == ERROR) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' check process result failed "
        + "for host `" + service_id->host->name + "'.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceSchedule(soap* s,
                                         ns1__serviceIDType* service_id,
                                         long delay,
                                         centreonengine__checkServiceScheduleResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_service_check(service, delay, CHECK_OPTION_NONE);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceScheduleForced(soap* s,
                                               ns1__serviceIDType* service_id,
                                               long delay,
                                               centreonengine__checkServiceScheduleForcedResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_service_check(service, delay, CHECK_OPTION_FORCE_EXECUTION);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the author of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetAuthor(soap* s,
                                      ns1__downtimeIDType* downtime_id,
                                      std::string& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (downtime->author != NULL) {
      val = downtime->author;
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the duration of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetDuration(soap* s,
                                        ns1__downtimeIDType* downtime_id,
                                        double& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->duration;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the end time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetEnd(soap* s,
                                   ns1__downtimeIDType* downtime_id,
                                   unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->end_time;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if a downtime is fixed or not.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetFixed(soap* s,
                                     ns1__downtimeIDType* downtime_id,
                                     bool& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->fixed;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the start time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetStart(soap* s,
                                     ns1__downtimeIDType* downtime_id,
                                     unsigned long& val) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->start_time;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delete a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeDelete(soap* s,
                                   ns1__downtimeIDType* downtime_id,
                                   centreonengine__downtimeDeleteResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    if (unschedule_downtime(HOST_DOWNTIME, downtime_id->downtime) == ERROR
        && unschedule_downtime(SERVICE_DOWNTIME, downtime_id->downtime) == ERROR) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a downtime on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddToHost(soap* s,
                                      ns1__hostIDType* host_id,
                                      ns1__downtimeType* downtime_type,
                                      centreonengine__downtimeAddToHostResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    char* author = my_strdup(downtime_type->author.c_str());
    char* comment = my_strdup(downtime_type->comment.c_str());

    unsigned long downtime_id;
    if (schedule_downtime(HOST_DOWNTIME,
                          host->name,
                          NULL,
                          time(NULL),
                          author,
                          comment,
                          downtime_type->starttime,
                          downtime_type->endtime,
                          downtime_type->fixed,
                          downtime_type->triggerid->downtime,
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime to host failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule downtime for a host and all of its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddAndPropagateToHost(soap* s,
                                                  ns1__hostIDType* host_id,
                                                  ns1__downtimeType* downtime_type,
                                                  centreonengine__downtimeAddAndPropagateToHostResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    time_t entry_time = time(NULL);

    char* author = my_strdup(downtime_type->author.c_str());
    char* comment = my_strdup(downtime_type->comment.c_str());

    unsigned long downtime_id;
    if (schedule_downtime(HOST_DOWNTIME,
                          host->name,
                          NULL,
                          entry_time,
                          author,
                          comment,
                          downtime_type->starttime,
                          downtime_type->endtime,
                          downtime_type->fixed,
                          downtime_type->triggerid->downtime,
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {

      delete[] author;
      delete[] comment;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime faild.";
      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    schedule_and_propagate_downtime(host,
                                    entry_time,
                                    author,
                                    comment,
                                    downtime_type->starttime,
                                    downtime_type->endtime,
                                    downtime_type->fixed,
                                    0,
                                    static_cast<unsigned long>(downtime_type->duration));

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule downtime for a host and all of its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddAndPropagateTriggeredToHost(soap* s,
                                                           ns1__hostIDType* host_id,
                                                           ns1__downtimeType* downtime_type,
                                                           centreonengine__downtimeAddAndPropagateTriggeredToHostResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    time_t entry_time = time(NULL);

    char* author = my_strdup(downtime_type->author.c_str());
    char* comment = my_strdup(downtime_type->comment.c_str());

    unsigned long downtime_id;
    if (schedule_downtime(HOST_DOWNTIME,
                          host->name,
                          NULL,
                          entry_time,
                          author,
                          comment,
                          downtime_type->starttime,
                          downtime_type->endtime,
                          downtime_type->fixed,
                          downtime_type->triggerid->downtime,
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime faild.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    schedule_and_propagate_downtime(host,
                                    entry_time,
                                    author,
                                    comment,
                                    downtime_type->starttime,
                                    downtime_type->endtime,
                                    downtime_type->fixed,
                                    res.downtimeid->downtime,
                                    static_cast<unsigned long>(downtime_type->duration));

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a downtime on all services of a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddToHostServices(soap* s,
                                              ns1__hostIDType* host_id,
                                              ns1__downtimeType* downtime_type,
                                              centreonengine__downtimeAddToHostServicesResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }


    time_t entry_time = time(NULL);

    char* author = my_strdup(downtime_type->author.c_str());
    char* comment = my_strdup(downtime_type->comment.c_str());
    bool is_error = false;

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr !=NULL) {
        if (schedule_downtime(SERVICE_DOWNTIME,
                              host->name,
                              tmp->service_ptr->description,
                              entry_time,
                              author,
                              comment,
                              downtime_type->starttime,
                              downtime_type->endtime,
                              downtime_type->fixed,
                              downtime_type->triggerid->downtime,
                              static_cast<unsigned long>(downtime_type->duration),
                              NULL) == ERROR) {
          is_error = true;
        }
      }
    }

    delete[] author;
    delete[] comment;

    if (is_error == 0) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' one or more service cannot schedule downtime.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a downtime on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddToService(soap* s,
                                         ns1__serviceIDType* service_id,
                                         ns1__downtimeType* downtime_type,
                                         centreonengine__downtimeAddToServiceResponse& res) {
  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    char* author = my_strdup(downtime_type->author.c_str());
    char* comment = my_strdup(downtime_type->comment.c_str());

    unsigned long downtime_id;
    if (schedule_downtime(SERVICE_DOWNTIME,
                          service->host_name,
                          service->description,
                          time(NULL),
                          author,
                          comment,
                          downtime_type->starttime,
                          downtime_type->endtime,
                          downtime_type->fixed,
                          downtime_type->triggerid->downtime,
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service + "' add downtime to service failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delay a host notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationHostDelay(soap* s,
                                          ns1__hostIDType* host_id,
                                          long delay,
                                          centreonengine__notificationHostDelayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->next_host_notification = delay;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Send a notification on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationHostSend(soap* s,
                                         ns1__hostIDType* host_id,
                                         ns1__notificationType* notification_type,
                                         centreonengine__notificationHostSendResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << notification_type->author
      << ", " << notification_type->broadcast
      << ", " << notification_type->comment
      << ", " << notification_type->forced
      << ", " << notification_type->increment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
      | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
      | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);

    char* author = my_strdup(notification_type->author.c_str());
    char* comment = my_strdup(notification_type->comment.c_str());

    if (host_notification(host,
                          NOTIFICATION_CUSTOM,
                          author,
                          comment,
                          options) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' send notification failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delay a service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationServiceDelay(soap* s,
                                             ns1__serviceIDType* service_id,
                                             long delay,
                                             centreonengine__notificationServiceDelayResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->next_notification = delay;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Send a notification on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationServiceSend(soap* s,
                                            ns1__serviceIDType* service_id,
                                            ns1__notificationType* notification_type,
                                            centreonengine__notificationServiceSendResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << notification_type->author
      << ", " << notification_type->broadcast
      << ", " << notification_type->comment
      << ", " << notification_type->forced
      << ", " << notification_type->increment << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
      | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
      | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);


    char* author = my_strdup(notification_type->author.c_str());
    char* comment = my_strdup(notification_type->comment.c_str());

    if (service_notification(service,
                             NOTIFICATION_CUSTOM,
                             author,
                             comment,
                             options) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service + "' send notification failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] author;
    delete[] comment;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

// /**
//  *  Dump all object list.
//  *
//  *  @param[in]  s                 Unused.
//  *  @param[out] res               Unused.
//  *
//  *  @return SOAP_OK on success.
//  */
// int centreonengine__dumpObjectList(soap* s,
//                                    centreonengine__dumpObjectListResponse& res) {
//   (void)res;

//   try {
//     sync::instance().wait_thread_safeness();
//     logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
//     com::centreon::engine::logging::dump_object_list();
//     sync::instance().worker_finish();
//   }
//   catch (std::exception const& e) {
//     logger(dbg_commands, most)
//       << "Webservice: " << __func__ << " failed: " << e.what() << ".";
//     sync::instance().worker_finish();
//     return (soap_receiver_fault(s, "invalid argument", e.what()));
//   }
//   catch (...) {
//     logger(dbg_commands, most)
//       << "Webservice: " << __func__ << " failed. catch all.";
//     sync::instance().worker_finish();
//     return (soap_receiver_fault(s, "Runtime error.", "catch all"));

//   }
//   return (SOAP_OK);
// }

/**
 *  Add host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host              Host to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addHost(soap* s,
			    ns1__hostType* host,
			    centreonengine__addHostResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host->name << ")";

    create_host(*host);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add host dependency.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  hostdependency    Host dependency to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addHostDependency(soap* s,
                                      ns1__hostDependencyType* hostdependency,
                                      centreonengine__addHostDependencyResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "()";

    create_host_dependency(*hostdependency);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add host escalation.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  hostescalation    Host escalation to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addHostEscalation(soap* s,
                                      ns1__hostEscalationType* hostescalation,
                                      centreonengine__addHostEscalationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "()";

    create_host_escalation(*hostescalation);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service           Service to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addService(soap* s,
			       ns1__serviceType* service,
			       centreonengine__addServiceResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({" << service->hostName
      << ", " << service->serviceDescription << "})";


    create_service(*service);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add service dependency.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  servicedependency Service dependency to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addServiceDependency(soap* s,
                                         ns1__serviceDependencyType* servicedependency,
                                         centreonengine__addServiceDependencyResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({"
      << servicedependency->dependentServiceDescription << ", "
      << servicedependency->serviceDescription << "})";

    create_service_dependency(*servicedependency);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add service escalation.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  serviceescalation Service escalation to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addServiceEscalation(soap* s,
                                         ns1__serviceEscalationType* serviceescalation,
                                         centreonengine__addServiceEscalationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({"
      << serviceescalation->serviceDescription << "})";

    create_service_escalation(*serviceescalation);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
           << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Add timeperiod.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  tperiod           Timeperiod to add.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__addTimeperiod(soap* s,
                                  ns1__timeperiodType* tperiod,
                                  centreonengine__addTimeperiodResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << tperiod->name << ")";

    create_timeperiod(*tperiod);

    sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed: " << e.what() << ".";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "invalid argument", e.what()));
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Remove host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to remove.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__removeHost(soap* s,
			       ns1__hostIDType* host_id,
			       centreonengine__removeHostResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name << ")";

    if (!remove_host_by_id(host_id->name.c_str())) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
                     << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));

  }
  return (SOAP_OK);
}

/**
 *  Remove service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to remove.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__removeService(soap* s,
				  ns1__serviceIDType* service_id,
				  centreonengine__removeServiceResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } })";

    if (!remove_service_by_id(service_id->host->name.c_str(), service_id->service.c_str())) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
                     << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));

  }
  return (SOAP_OK);
}

/**
 *  Remove service escalation.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  serviceescalation_id Service escalation to remove.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__removeServiceEscalation(soap* s,
					    ns1__serviceEscalationIDType* escalation_id,
					    centreonengine__removeServiceEscalationResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({" << escalation_id->name
      << ", " << escalation_id->description << "})";

    if (!remove_service_escalation_by_id(escalation_id->name.c_str(),
                                         escalation_id->description.c_str())) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service escalation `" + escalation_id->name + " "
	+ escalation_id->description + "' not found.";

      logger(dbg_commands, most)
                     << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));

  }
  return (SOAP_OK);
}

/**
 *  Remove service dependency.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  servicedependency_id Service dependency to remove.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__removeServiceDependency(soap* s,
					    ns1__serviceDependencyIDType* dependency_id,
					    centreonengine__removeServiceDependencyResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({" << dependency_id->hostName
      << ", " << dependency_id->serviceDescription << ", "
      << dependency_id->dependentHostName << ", "
      << dependency_id->dependentServiceDescription << "})";

    if (!remove_service_dependency_by_id(dependency_id->hostName.c_str(),
					dependency_id->serviceDescription.c_str(),
					dependency_id->dependentHostName.c_str(),
					dependency_id->dependentServiceDescription.c_str())) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service dependency `" + dependency_id->hostName + " "
	+ dependency_id->serviceDescription + " " + dependency_id->dependentHostName
	+ dependency_id->dependentServiceDescription + "' not found.";

      logger(dbg_commands, most)
                     << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));

  }
  return (SOAP_OK);
}

/**
 *  Remove timeperiod.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  timeperiod_id     Timeperiod to remove.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__removeTimeperiod(soap* s,
				     ns1__timeperiodIDType* timeperiod_id,
				     centreonengine__removeHostResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << timeperiod_id->timeperiod.c_str() << ")";

    if (!remove_timeperiod_by_id(timeperiod_id->timeperiod.c_str())) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + timeperiod_id->timeperiod + "' not found.";

      logger(dbg_commands, most)
                     << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));

  }
  return (SOAP_OK);
}

/**
 *  Check if event handlers are enabled globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getEventHandlersEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_event_handlers;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if failure prediction is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getFailurePredictionEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_failure_prediction;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getFlapDetectionEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_flap_detection;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if hosts active checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsChecksActiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = execute_host_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if hosts passive checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsChecksPassiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = accept_passive_host_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the global host event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsEventHandler(soap* s,
                                         centreonengine__getHostsEventHandlerResponse& res) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    res.command = soap_new_ns1__commandIDType(s, 1);
    res.command->command = global_host_event_handler;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the hosts freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsFreshnessChecksEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = check_host_freshness;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsObsessOverEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = obsess_over_hosts;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getNotificationsEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_notifications;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if performance data processing is enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getPerfdataProcessingEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = process_performance_data;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services active checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesChecksActiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = execute_service_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services passive checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesChecksPassiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = accept_passive_service_checks;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the global service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesEventHandler(soap* s,
                                            centreonengine__getServicesEventHandlerResponse& res) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    res.command = soap_new_ns1__commandIDType(s, 1);
    res.command->command = global_service_event_handler;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesFreshnessChecksEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = check_service_freshness;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesObsessOverEnabled(soap* s, bool& val) {
  (void)s;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = obsess_over_services;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable event handlers globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setEventHandlersEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setEventHandlersEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    enable_event_handlers = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setFailurePredictionEnabled(soap* s,
                                                bool enable,
                                                centreonengine__setFailurePredictionEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_failure_prediction = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setFlapDetectionEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setFlapDetectionEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_flap_detection = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsChecksActiveEnabled(soap* s,
                                                bool enable,
                                                centreonengine__setHostsChecksActiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    execute_host_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable passive host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsChecksPassiveEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setHostsChecksPassiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    accept_passive_host_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Change the global host event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsEventHandler(soap* s,
                                         ns1__commandIDType* command_id,
                                         centreonengine__setHostsEventHandlerResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command_id->command << ")";

    command* command = find_command(command_id->command.c_str());
    if (command == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Command `" + command_id->command + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] global_host_event_handler;
    global_host_event_handler = my_strdup(command_id->command.c_str());
    global_host_event_handler_ptr = command;

    modified_host_process_attributes |= MODATTR_EVENT_HANDLER_COMMAND;

    broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
                                 NEBFLAG_NONE,NEBATTR_NONE,
                                 CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER,
                                 MODATTR_EVENT_HANDLER_COMMAND,
                                 modified_host_process_attributes,
                                 MODATTR_NONE,
                                 modified_service_process_attributes,
                                 NULL);

    update_program_status(false);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsFreshnessChecksEnabled(soap* s,
                                                   bool enable,
                                                   centreonengine__setHostsFreshnessChecksEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    check_host_freshness = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over host checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsObsessOverEnabled(soap* s,
                                              bool enable,
                                              centreonengine__setHostsObsessOverEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    obsess_over_hosts = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setNotificationsEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setNotificationsEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_notifications = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable performance data processing globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setPerfdataProcessingEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setPerfdataProcessingEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    process_performance_data = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesChecksActiveEnabled(soap* s,
                                                   bool enable,
                                                   centreonengine__setServicesChecksActiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    execute_service_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable passive service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesChecksPassiveEnabled(soap* s,
                                                    bool enable,
                                                    centreonengine__setServicesChecksPassiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    accept_passive_service_checks = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Change the global service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesEventHandler(soap* s,
                                            ns1__commandIDType* command_id,
                                            centreonengine__setServicesEventHandlerResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command_id->command << ")";

    command* command = find_command(command_id->command.c_str());
    if (command == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Command `" + command_id->command + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] global_service_event_handler;
    global_service_event_handler = my_strdup(command_id->command.c_str());
    global_service_event_handler_ptr = command;

    modified_service_process_attributes |= MODATTR_EVENT_HANDLER_COMMAND;

    broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
                                 NEBFLAG_NONE,
                                 NEBATTR_NONE,
                                 CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER,
                                 MODATTR_NONE,
                                 modified_host_process_attributes,
                                 MODATTR_EVENT_HANDLER_COMMAND,
                                 modified_service_process_attributes,
                                 NULL);

    update_program_status(false);
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable service freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesFreshnessChecksEnabled(soap* s,
                                                      bool enable,
                                                      centreonengine__setServicesFreshnessChecksEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    check_service_freshness = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over service checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesObsessOverEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setServicesObsessOverEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    obsess_over_services = enable;
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the host check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckPeriod(soap* s,
                                       ns1__hostIDType* host_id,
                                       ns1__timeperiodIDType* timeperiod_id,
                                       centreonengine__hostSetCheckPeriodResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name
        << ", " << timeperiod_id->timeperiod << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] host->check_period;
    host->check_period = my_strdup(timeperiod_id->timeperiod.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetServicesCheckActiveEnabled(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetServicesCheckActiveEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        tmp->service_ptr->checks_enabled = enable;
      }
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetServicesNotificationsEnabled(soap* s,
                                                        ns1__hostIDType* host_id,
                                                        bool enable,
                                                        centreonengine__hostSetServicesNotificationsEnabledResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        tmp->service_ptr->notifications_enabled = enable;
      }
    }
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set the service check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckPeriod(soap* s,
                                          ns1__serviceIDType* service_id,
                                          ns1__timeperiodIDType* timeperiod_id,
                                          centreonengine__serviceSetCheckPeriodResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, "
      << timeperiod_id->timeperiod << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] service->check_period;
    service->check_period = my_strdup(timeperiod_id->timeperiod.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Set notification period of service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsPeriod(soap* s,
                                                  ns1__serviceIDType* service_id,
                                                  ns1__timeperiodIDType* timeperiod_id,
                                                  centreonengine__serviceSetNotificationsPeriodResponse& res) {
  (void)res;

  try {
    sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, "
      << timeperiod_id->timeperiod << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] service->notification_period;
    service->notification_period = my_strdup(timeperiod_id->timeperiod.c_str());
    sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}