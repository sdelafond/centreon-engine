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

#ifndef CCE_SERVICE_HH
#  define CCE_SERVICE_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/monitorable.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

// Forward declaration.
class host;

/**
 *  @class service service.hh "com/centreon/engine/service.hh"
 *  @brief Service as a host's service.
 *
 *  This class represents a service. It is checkable and also a notifier.
 */
class                service : public monitorable {
 public:
                     service();
                     service(service const& other);
                     ~service();
  service&           operator=(service const& other);

  // Configuration.
  std::string const& get_description() const;
  host*              get_host() const;
  bool               get_stalk_on_critical() const;
  bool               get_stalk_on_ok() const;
  bool               get_stalk_on_unknown() const;
  bool               get_stalk_on_warning() const;
  bool               get_volatile() const;

  // State runtime.
  time_t             get_last_time_critical() const;
  void               set_last_time_critical(time_t last_critical);
  time_t             get_last_time_ok() const;
  void               set_last_time_ok(time_t last_ok);
  time_t             get_last_time_unknown() const;
  void               set_last_time_unknown(time_t last_unknown);
  time_t             get_last_time_warning() const;
  void               set_last_time_warning(time_t last_warning);

  // Notification.
  bool               get_notify_on_critical() const;
  void               set_notify_on_critical(bool notify);
  bool               get_notify_on_unknown() const;
  void               set_notify_on_unknown(bool notify);
  bool               get_notify_on_warning() const;
  void               set_notify_on_warning(bool notify);

 private:
  void               _internal_copy(service const& other);

  std::string        _description;
  host*              _host;
  time_t             _last_time_critical;
  time_t             _last_time_ok;
  time_t             _last_time_unknown;
  time_t             _last_time_warning;
  bool               _notify_on_critical;
  bool               _notify_on_unknown;
  bool               _notify_on_warning;
  bool               _stalk_on_critical;
  bool               _stalk_on_ok;
  bool               _stalk_on_unknown;
  bool               _stalk_on_warning;
};

CCE_END()

using com::centreon::engine::service;

typedef std::list<service*> service_set;

#endif // !CCE_SERVICE_HH
