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

#include <openscenario_interpreter/error.hpp>
#include <openscenario_interpreter/reader/attribute.hpp>
#include <openscenario_interpreter/reader/element.hpp>
#include <openscenario_interpreter/syntax/synchronize_action.hpp>

namespace openscenario_interpreter
{
inline namespace syntax
{
namespace
{
auto readFinalSpeed(const pugi::xml_node & node, Scope & scope) -> std::optional<Double>
{
  if (const auto final_speed_node = node.child("FinalSpeed")) {
    if (const auto absolute_speed_node = final_speed_node.child("AbsoluteSpeed")) {
      return readAttribute<Double>("value", absolute_speed_node, scope);
    } else {
      throw SyntaxError(
        "Given class RelativeSpeedToMaster is valid OpenSCENARIO element of class FinalSpeed, "
        "but is not supported yet");
    }
  } else {
    return std::nullopt;
  }
}
}  // namespace

SynchronizeAction::SynchronizeAction(const pugi::xml_node & node, Scope & scope)
: Scope(scope),
  master_entity_ref(readAttribute<String>("masterEntityRef", node, local())),
  target_position_master(readElement<Position>("TargetPositionMaster", node, local())),
  target_position(readElement<Position>("TargetPosition", node, local())),
  final_speed(readFinalSpeed(node, local())),
  target_tolerance(readAttribute<Double>("targetTolerance", node, local(), Double()))
{
}

auto SynchronizeAction::accomplished() -> bool
{
  return std::all_of(
    std::begin(accomplishments), std::end(accomplishments), [this](auto && accomplishment) {
      auto has_arrived = [this](const auto & actor) {
        auto evaluation = actor.apply([this](const auto & object) {
          return evaluateIsNearbyPosition(
            object, static_cast<NativeLanePosition>(target_position), target_tolerance);
        });
        return not evaluation.size() or evaluation.min();
      };
      return accomplishment.second = accomplishment.second or has_arrived(accomplishment.first);
    });
}

auto SynchronizeAction::start() -> void
{
  accomplishments.clear();

  for (const auto & actor : actors) {
    accomplishments.emplace(actor, false);
    actor.apply([&](const auto & object) {
      applySynchronizeAction(
        object, master_entity_ref, static_cast<NativeLanePosition>(target_position_master),
        static_cast<NativeLanePosition>(target_position), final_speed.value_or(Double(0.0)),
        target_tolerance);
    });
  }
}
}  // namespace syntax
}  // namespace openscenario_interpreter
