/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostgroup::hostgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostgroup::hostgroup(applier::hostgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostgroup::~hostgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostgroup& applier::hostgroup::operator=(
                      applier::hostgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostgroup.
 *
 *  @param[in] obj The new hostgroup to add into the monitoring engine.
 */
void applier::hostgroup::add_object(
                           shared_ptr<configuration::hostgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new hostgroup '" << obj->hostgroup_name() << "'.";

  // Add host group to the global configuration state.
  config->hostgroups().insert(obj);

  // Create host group.
  hostgroup_struct* hg(add_hostgroup(
                         obj->hostgroup_name().c_str(),
                         NULL_IF_EMPTY(obj->alias()),
                         NULL_IF_EMPTY(obj->notes()),
                         NULL_IF_EMPTY(obj->notes_url()),
                         NULL_IF_EMPTY(obj->action_url())));
  if (!hg)
    throw (engine_error() << "Error: Could not register hostgroup '"
           << obj->hostgroup_name() << "'.");

  // Apply resolved hosts on hostgroup.
  for (set_string::const_iterator
         it(obj->resolved_members().begin()),
         end(obj->resolved_members().end());
       it != end;
       ++it)
    if (!add_host_to_hostgroup(hg, it->c_str()))
      throw (engine_error() << "Error: Could not add host member '"
             << *it << "' to host group '" << obj->hostgroup_name()
             << "'.");

  return ;
}

/**
 *  Expand a hostgroup.
 *
 *  @param[in]     obj      Object to expand.
 *  @param[in,out] s        State being applied.
 */
void applier::hostgroup::expand_object(
                           shared_ptr<configuration::hostgroup> obj,
                           configuration::state& s) {
  // Resolve group.
  _resolve_members(obj, s);

  // We do not need to reinsert the object in the set, as
  // _resolve_members() only affects hostgroup::resolved_members()
  // and hostgroup::is_resolved() that are not used for the set
  // ordering.

  return ;
}

/**
 *  Modified hostgroup.
 *
 *  @param[in] obj The new hostgroup to modify into the monitoring
 *                 engine.
 */
void applier::hostgroup::modify_object(
                           shared_ptr<configuration::hostgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying hostgroup '" << obj->hostgroup_name() << "'.";

  // Find old configuration.
  set_hostgroup::iterator
    it_cfg(config->hostgroups_find(obj->key()));
  if (it_cfg == config->hostgroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "host group '" << obj->hostgroup_name() << "'.");

  // Find host group object.
  umap<std::string, shared_ptr<hostgroup_struct> >::iterator
    it_obj(applier::state::instance().hostgroups_find(obj->key()));
  if (it_obj == applier::state::instance().hostgroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "host group object '" << obj->hostgroup_name() << "'.");
  hostgroup_struct* hg(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::hostgroup> old_cfg(*it_cfg);
  config->hostgroups().insert(obj);
  config->hostgroups().erase(it_cfg);

  // Modify properties.
  modify_if_different(
    hg->action_url,
    NULL_IF_EMPTY(obj->action_url()));
  modify_if_different(
    hg->alias,
    NULL_IF_EMPTY(obj->alias()));
  modify_if_different(
    hg->notes,
    NULL_IF_EMPTY(obj->notes()));
  modify_if_different(
    hg->notes_url,
    NULL_IF_EMPTY(obj->notes_url()));

  // Were members modified ?
  if (obj->members() != old_cfg->members()) {
    // Delete all old host group members.
    for (hostsmember* m((*it_obj).second->members); m;) {
      hostsmember* to_delete(m);
      m = m->next;
      deleter::hostsmember(to_delete);
    }
    (*it_obj).second->members = NULL;

    // Create new host group members.
    for (set_string::const_iterator
           it(obj->resolved_members().begin()),
           end(obj->resolved_members().end());
         it != end;
         ++it)
      if (!add_host_to_hostgroup(
             hg,
             it->c_str()))
        throw (engine_error() << "Error: Could not add host member '"
               << *it << "' to host group '" << obj->hostgroup_name()
               << "'.");
  }

  return ;
}

/**
 *  Remove old hostgroup.
 *
 *  @param[in] obj The new hostgroup to remove from the monitoring engine.
 */
void applier::hostgroup::remove_object(
                           shared_ptr<configuration::hostgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing hostgroup '" << obj->hostgroup_name() << "'.";

  // Find host group.
  umap<std::string, shared_ptr<hostgroup_struct> >::iterator
    it(applier::state::instance().hostgroups_find(obj->key()));
  if (it != applier::state::instance().hostgroups().end()) {
    // Remove host group from its list.
    unregister_object<hostgroup_struct>(
      &hostgroup_list,
      it->second.get());

    // Erase host group object (will effectively delete the object).
    applier::state::instance().hostgroups().erase(it);
  }

  // Remove host group from the global configuration set.
  config->hostgroups().erase(obj);

  return ;
}

/**
 *  Resolve a host group.
 *
 *  @param[in] obj
 *  @param[in] s
 */
void applier::hostgroup::resolve_object(
                           shared_ptr<configuration::hostgroup> obj) {
  // XXX
}

/**
 *  Resolve members of a host group.
 *
 *  @param[in,out] obj Hostgroup object.
 *  @param[in]     s   Configuration being applied.
 */
void applier::hostgroup::_resolve_members(
                           shared_ptr<configuration::hostgroup> obj,
                           configuration::state& s) {
  // Only process if hostgroup has not been resolved already.
  if (!obj->is_resolved()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of host group '"
      << obj->hostgroup_name() << "'.";

    // Mark object as resolved.
    obj->set_resolved(true);

    // Add base members.
    for (list_string::const_iterator
           it(obj->members().begin()),
           end(obj->members().end());
         it != end;
         ++it)
      obj->resolved_members().insert(*it);

    // Add hostgroup members.
    for (list_string::const_iterator
           it(obj->hostgroup_members().begin()),
           end(obj->hostgroup_members().end());
         it != end;
         ++it) {
      // Find hostgroup entry.
      set_hostgroup::iterator
        it2(s.hostgroups().begin()),
        end2(s.hostgroups().end());
      while (it2 != end2) {
        if ((*it2)->hostgroup_name() == *it)
          break ;
        ++it2;
      }
      if (it2 == s.hostgroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing hostgroup member '"
               << *it << "' to hostgroup '" << obj->hostgroup_name()
               << "'.");

      // Resolve hostgroup member.
      _resolve_members(*it2, s);

      // Add hostgroup member members to members.
      for (set_string::const_iterator
             it3((*it2)->resolved_members().begin()),
             end3((*it2)->resolved_members().end());
           it3 != end3;
           ++it3)
        obj->resolved_members().insert(*it3);
    }
  }

  return ;
}