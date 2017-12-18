/*
** Copyright 2017 Centreon
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

#include <memory>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/not_found.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/objects/timeperiod.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;


/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 *  Constructor.
 */
contact::contact() {}

contact::contact(configuration::contact const& obj)
  : _name(obj.contact_name()),
    _alias((obj.alias().empty()) ? obj.contact_name() : obj.alias()),
    _email(obj.email()),
    _pager(obj.pager()),
    _address(obj.address()),
    _service_notification_period_name(obj.service_notification_period()),
    _host_notification_period_name(obj.host_notification_period()),
    _service_notified_states(
        ((obj.service_notification_options() & configuration::service::ok)
         ? notifier::ON_RECOVERY : 0)
      | ((obj.service_notification_options() & configuration::service::critical)
         ? notifier::ON_CRITICAL : 0)
      | ((obj.service_notification_options() & configuration::service::warning)
         ? notifier::ON_WARNING : 0)
      | ((obj.service_notification_options() & configuration::service::unknown)
         ? notifier::ON_UNKNOWN : 0)
      | ((obj.service_notification_options() & configuration::service::flapping)
         ? notifier::ON_FLAPPING : 0)
      | ((obj.service_notification_options() & configuration::service::downtime)
         ? notifier::ON_DOWNTIME : 0)),
    _host_notified_states(
        ((obj.host_notification_options() & configuration::host::up)
          ? notifier::ON_RECOVERY : 0)
      | ((obj.host_notification_options() & configuration::host::down)
          ? notifier::ON_DOWN : 0)
      | ((obj.host_notification_options() & configuration::host::unreachable)
          ? notifier::ON_UNREACHABLE : 0)
      | ((obj.host_notification_options() & configuration::host::flapping)
          ? notifier::ON_FLAPPING : 0)
      | ((obj.host_notification_options() & configuration::host::downtime)
          ? notifier::ON_DOWNTIME : 0)),
    _host_notifications_enabled(obj.host_notifications_enabled()),
    _modified_attributes(MODATTR_NONE),
    _modified_host_attributes(MODATTR_NONE),
    _modified_service_attributes(MODATTR_NONE),
    _retain_nonstatus_information(obj.retain_nonstatus_information()),
    _retain_status_information(obj.retain_status_information()),
    _service_notifications_enabled(obj.service_notifications_enabled()),
    _timezone(obj.timezone()) {
    
  // Make sure we have the data we need.
  if (_name.empty())
    throw (engine_error() << "contact: Contact name is empty");

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contact::contact(contact const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contact& contact::operator=(contact const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contact::~contact() {
  logger(dbg_functions, basic)
    << "contact: destructor";
}

/**
 *  Return the contact name
 *
 *  @return a reference to the name
 */
std::string const& contact::get_name() const {
  return _name;
}

/**
 *  Return the contact alias
 *
 *  @return a reference to the alias
 */
std::string const& contact::get_alias() const {
  return _alias;
}

/**
 *  Return the contact email
 *
 *  @return a reference to the email
 */
std::string const& contact::get_email() const {
  return _email;
}

/**
 *  Return the contact pager
 *
 *  @return a reference to the pager
 */
std::string const& contact::get_pager() const {
  return _pager;
}

/**
 *  Checks contact
 *
 * @param[out] w Number of warnings returned by the check
 * @param[out] e Number of errors returned by the check
 *
 * @return true if no errors are returned.
 */
bool contact::check(int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check service notification commands */
  if (get_service_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no service "
      "notification commands defined!";
    errors++;
  }
  else
    for (command_map::iterator
           it(get_service_notification_commands().begin()),
           end(get_service_notification_commands().end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find(buf, '!'));
      std::string command_name(buf.substr(0, index));
      shared_ptr<command> temp_command;
      try {
        temp_command = find_command(command_name);
      }
      catch (not_found const& e) {
        (void)e;
        logger(log_verification_error, basic)
          << "Error: Service notification command '"
          << command_name << "' specified for contact '"
          << get_name() << "' is not defined anywhere!";
	++errors;
      }

      /* save pointer to the command for later */
      it->second = temp_command;
    }

  /* check host notification commands */
  if (get_host_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no host "
      "notification commands defined!";
    errors++;
  }
  else
    for (command_map::iterator
           it(get_host_notification_commands().begin()),
           end(get_host_notification_commands().end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find('!'));
      std::string command_name(buf.substr(0, index));
      shared_ptr<command> cmd;
      try {
        cmd = find_command(command_name);
      }
      catch (not_found const& e) {
        (void)e;
        logger(log_verification_error, basic)
          << "Error: Host notification command '" << command_name
          << "' specified for contact '" << get_name()
          << "' is not defined anywhere!";
	++errors;
      }

      /* save pointer to the command for later */
      it->second = cmd;
    }

  /* check service notification timeperiod */
  if (get_service_notification_period_name().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << get_name() << "' has no service "
      "notification time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod;
    try {
      temp_timeperiod = &find_timeperiod(get_service_notification_period_name());
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Service notification period '"
        << get_service_notification_period_name()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      ++errors;
    }
    set_service_notification_period(temp_timeperiod);
  }

  /* check host notification timeperiod */
  if (get_host_notification_period_name().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << get_name() << "' has no host "
      "notification time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod;
    try {
      temp_timeperiod = &find_timeperiod(get_host_notification_period_name());
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Host notification period '"
        << get_host_notification_period_name()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      ++errors;
    }

    /* save the pointer to the host notification timeperiod for later */
    set_host_notification_period(temp_timeperiod);
  }

  /* check for sane host recovery options */
  if (notify_on_host_recovery()
      && !notify_on_host_down()
      && !notify_on_host_unreachable()) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }

  /* check for sane service recovery options */
  if (notify_on_service_recovery()
      && !notify_on_service_critical()
      && !notify_on_service_warning()) {
    logger(log_verification_error, basic)
      << "Warning: Service recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify critical "
      "and/or warning options as well";
    warnings++;
  }

  /* check for illegal characters in contact name */
  if (contains_illegal_object_chars()) {
    logger(log_verification_error, basic)
      << "Error: The name of contact '" << get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;
  return (errors == 0);
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
command_map& contact::get_host_notification_commands() {
  return _host_notification_commands;
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
command_map const& contact::get_host_notification_commands() const {
  return _host_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
command_map& contact::get_service_notification_commands() {
  return _service_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
command_map const& contact::get_service_notification_commands() const {
  return _service_notification_commands;
}

void contact::set_host_notification_period(timeperiod* tp) {
  _host_notification_period = tp;
}

void contact::set_service_notification_period(timeperiod* tp) {
  _service_notification_period = tp;
}

bool contact::notify_on_service_critical() const {
  return (_service_notified_states & notifier::ON_CRITICAL);
}

bool contact::notify_on_service_recovery() const {
  return (_service_notified_states & notifier::ON_RECOVERY);
}

bool contact::notify_on_service_warning() const {
  return (_service_notified_states & notifier::ON_WARNING);
}

bool contact::notify_on_host_recovery() const {
  return (_host_notified_states & notifier::ON_RECOVERY);
}

bool contact::notify_on_host_down() const {
  return (_host_notified_states & notifier::ON_DOWN);
}

bool contact::notify_on_host_unreachable() const {
  return (_host_notified_states & notifier::ON_UNREACHABLE);
}

bool contact::contains_illegal_object_chars() const {
  if (_name.empty() || !illegal_object_chars)
    return false;
  return (_name.find(illegal_object_chars) != std::string::npos);
}

std::list<shared_ptr<contactgroup> > const& contact::get_contactgroups() const {
  return _contact_groups;
}

std::list<shared_ptr<contactgroup> >& contact::get_contactgroups() {
  return _contact_groups;
}

customvar_set const& contact::get_customvars() const {
  return _vars;
}

void contact::set_customvar(customvar const& var) {
  bool add(false);

  if (_vars.find(var.get_name()) == _vars.end())
    add = true;

  _vars[var.get_name()] = var;

  if (add) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));

    broker_custom_variable(
        NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        this,
        var.get_name().c_str(),
        var.get_value().c_str(),
        &tv);
  }
}

std::string const& contact::get_timezone() const {
  return _timezone;
}

std::string const& contact::get_address(int index) const {
  return _address[index];
}

/**
 *  Update contact status info.
 *
 *  @param aggregated_dump
 */
void contact::update_status(int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_contact_status(
      NEBTYPE_CONTACTSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      NULL);
}

unsigned long contact::get_modified_attributes() const {
  return _modified_attributes;
}

void contact::set_modified_attributes(unsigned long attr) {
  _modified_attributes = attr;
}

unsigned long contact::get_modified_host_attributes() const {
  return _modified_host_attributes;
}

void contact::set_modified_host_attributes(unsigned long attr) {
  _modified_host_attributes = attr;
}

unsigned long contact::get_modified_service_attributes() const {
  return _modified_service_attributes;
}

void contact::set_modified_service_attributes(unsigned long attr) {
  _modified_service_attributes = attr;
}

time_t contact::get_last_host_notification() const {
  return _last_host_notification;
}

void contact::set_last_host_notification(time_t t) {
  _last_host_notification = t;
}

time_t contact::get_last_service_notification() const {
  return _last_service_notification;
}

void contact::set_last_service_notification(time_t t) {
  _last_service_notification = t;
}

timeperiod_struct* contact::get_host_notification_period() const {
  return _host_notification_period;
}

timeperiod_struct* contact::get_service_notification_period() const {
  return _service_notification_period;
}

bool contact::is_host_notifications_enabled() const {
  return _host_notifications_enabled;
}

void contact::set_host_notifications_enabled(bool enabled) {
  _host_notifications_enabled = enabled;
}

bool contact::is_service_notifications_enabled() const {
  return _service_notifications_enabled;
}

void contact::set_service_notifications_enabled(bool enabled) {
  _service_notifications_enabled = enabled;
}

std::string const& contact::get_host_notification_period_name() const {
  return _host_notification_period_name;
}

void contact::set_host_notification_period_name(std::string const& name) {
  _host_notification_period_name = name;
}

std::string const& contact::get_service_notification_period_name() const {
  return _service_notification_period_name;
}

void contact::set_service_notification_period_name(std::string const& name) {
  _service_notification_period_name = name;
}

void contact::add_host_notification_command(std::string const& command_name) {
  // Make sure we have the data we need.
  if (command_name.empty())
    throw (engine_error()
             << "Error: Host notification command is empty");

  _host_notification_commands[command_name] = shared_ptr<command>(0);
}

void contact::add_service_notification_command(std::string const& command_name) {
  // Make sure we have the data we need.
  if (command_name.empty())
    throw (engine_error()
             << "Error: Service notification command is empty");

  _service_notification_commands[command_name] = shared_ptr<command>(0);
}

void contact::update_config(configuration::contact const& obj) {
  configuration::applier::modify_if_different(
    _alias, obj.alias().empty() ? obj.contact_name() : obj.alias());
  configuration::applier::modify_if_different(_email, obj.email());
  configuration::applier::modify_if_different(_pager, obj.pager());
  configuration::applier::modify_if_different(_address, obj.address());
  configuration::applier::modify_if_different(_service_notified_states, obj.service_notification_options());
  configuration::applier::modify_if_different(_host_notified_states, obj.host_notification_options());

  configuration::applier::modify_if_different(
    _host_notification_period_name, obj.host_notification_period());
  configuration::applier::modify_if_different(
    _service_notification_period_name, obj.service_notification_period());
  configuration::applier::modify_if_different(
    _host_notifications_enabled, obj.host_notifications_enabled());
  configuration::applier::modify_if_different(
    _service_notifications_enabled, obj.service_notifications_enabled());
  configuration::applier::modify_if_different(
    _can_submit_commands, obj.can_submit_commands());
  configuration::applier::modify_if_different(
    _retain_status_information, obj.retain_status_information());
  configuration::applier::modify_if_different(
    _retain_nonstatus_information, obj.retain_nonstatus_information());
  configuration::applier::modify_if_different(
    _timezone, obj.timezone());
}

void contact::clear_host_notification_commands() {
  _host_notification_commands.clear();
}

void contact::clear_service_notification_commands() {
  _service_notification_commands.clear();
}

void contact::clear_custom_variables() {
  // Browse all custom vars.
  for (customvar_set::iterator
         it(_vars.begin()),
         end(_vars.end());
       it != end;) {

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_CONTACTCUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      it->second.get_name().c_str(),
      it->second.get_value().c_str(),
      &tv);

    it = _vars.erase(it);
  }
}

bool contact::operator<(contact const& other) {
  if (_name.compare(other._name) < 0)
    return true;
  if (_alias.compare(other._alias) < 0)
    return true;
  return false;
}

bool contact::get_retain_status_information() const {
  return _retain_status_information;
}

bool contact::get_retain_nonstatus_information() const {
  return _retain_nonstatus_information;
}

/* enables host notifications for a contact */
void contact::enable_host_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (is_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_host_attributes |= attr;

  /* enable the host notifications... */
  set_host_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    attr,
    get_modified_host_attributes(),
    MODATTR_NONE,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* disables host notifications for a contact */
void contact::disable_host_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!is_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_host_attributes |= attr;

  /* enable the host notifications... */
  set_host_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    attr,
    get_modified_host_attributes(),
    MODATTR_NONE,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* enables service notifications for a contact */
void contact::enable_service_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (is_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_service_attributes |= attr;

  /* enable the host notifications... */
  set_service_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    MODATTR_NONE,
    get_modified_host_attributes(),
    attr,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* disables service notifications for a contact */
void contact::disable_service_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!is_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_service_attributes |= attr;

  /* enable the host notifications... */
  set_service_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    MODATTR_NONE,
    get_modified_host_attributes(),
    attr,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}
