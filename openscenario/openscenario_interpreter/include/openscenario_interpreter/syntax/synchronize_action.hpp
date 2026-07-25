// Copyright 2015 TIER IV, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OPENSCENARIO_INTERPRETER__SYNTAX__SYNCHRONIZE_ACTION_HPP_
#define OPENSCENARIO_INTERPRETER__SYNTAX__SYNCHRONIZE_ACTION_HPP_

#include <openscenario_interpreter/scope.hpp>
#include <openscenario_interpreter/simulator_core.hpp>
#include <openscenario_interpreter/syntax/boolean.hpp>
#include <openscenario_interpreter/syntax/double.hpp>
#include <openscenario_interpreter/syntax/entity.hpp>
#include <openscenario_interpreter/syntax/entity_ref.hpp>
#include <openscenario_interpreter/syntax/position.hpp>
#include <optional>
#include <pugixml.hpp>
#include <unordered_map>

namespace openscenario_interpreter
{
inline namespace syntax
{
/* ---- SynchronizeAction (OpenSCENARIO XML 1.2/1.3) ---------------------------
 *
 *  <xsd:complexType name="SynchronizeAction">
 *    <xsd:all>
 *      <xsd:element name="TargetPositionMaster" type="Position"/>
 *      <xsd:element name="TargetPosition" type="Position"/>
 *      <xsd:element name="FinalSpeed" type="FinalSpeed" minOccurs="0"/>
 *    </xsd:all>
 *    <xsd:attribute name="masterEntityRef" type="String" use="required"/>
 *    <xsd:attribute name="targetToleranceMaster" type="Double"/>
 *    <xsd:attribute name="targetTolerance" type="Double"/>
 *  </xsd:complexType>
 *
 *  NOTE: This is a simplified implementation, intentionally limited to the
 *  common case of synchronizing this action's actor with another ("master")
 *  entity so both reach their respective target positions at the same time
 *  (e.g. meeting in the middle of an intersection). It always applies a
 *  linear (constant-acceleration) speed profile via
 *  traffic_simulator::entity::EntityBase::requestSynchronize; the
 *  non-linear (convex/concave), stop/wait and SteadyState behaviours
 *  described by the OpenSCENARIO standard (see e.g. esmini's
 *  SynchronizeAction) are not implemented.
 *
 *  Of <FinalSpeed>, only <AbsoluteSpeed> (without its optional SteadyState
 *  group) is supported; <RelativeSpeedToMaster> is not. The
 *  targetToleranceMaster attribute is parsed for schema-completeness but is
 *  currently unused, since requestSynchronize does not take a separate
 *  tolerance for the master entity's arrival.
 *
 * -------------------------------------------------------------------------- */
struct SynchronizeAction : private Scope,
                           private SimulatorCore::ActionApplication,
                           private SimulatorCore::ConditionEvaluation
{
  const EntityRef master_entity_ref;

  const Position target_position_master;

  const Position target_position;

  const std::optional<Double> final_speed;

  const Double target_tolerance;

  std::unordered_map<Entity, Boolean> accomplishments;

  explicit SynchronizeAction(const pugi::xml_node &, Scope &);

  auto accomplished() -> bool;

  static auto endsImmediately() noexcept -> bool { return false; }

  static auto run() noexcept -> void {}

  auto start() -> void;
};
}  // namespace syntax
}  // namespace openscenario_interpreter

#endif  // OPENSCENARIO_INTERPRETER__SYNTAX__SYNCHRONIZE_ACTION_HPP_
