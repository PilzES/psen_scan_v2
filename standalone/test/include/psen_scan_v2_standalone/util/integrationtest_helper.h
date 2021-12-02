// Copyright (c) 2020-2021 Pilz GmbH & Co. KG
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PSEN_SCAN_V2_STANDALONE_TEST_INTEGRATIONTEST_HELPER_H
#define PSEN_SCAN_V2_STANDALONE_TEST_INTEGRATIONTEST_HELPER_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ostream>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "psen_scan_v2_standalone/configuration/scanner_ids.h"
#include "psen_scan_v2_standalone/data_conversion_layer/diagnostics.h"
#include "psen_scan_v2_standalone/data_conversion_layer/io_pin_data.h"
#include "psen_scan_v2_standalone/data_conversion_layer/laserscan_conversions.h"
#include "psen_scan_v2_standalone/data_conversion_layer/monitoring_frame_msg.h"
#include "psen_scan_v2_standalone/data_conversion_layer/monitoring_frame_msg_builder.h"
#include "psen_scan_v2_standalone/util/format_range.h"
#include "psen_scan_v2_standalone/util/tenth_of_degree.h"
#include "psen_scan_v2_standalone/io_state.h"
#include "psen_scan_v2_standalone/laserscan.h"
#include "psen_scan_v2_standalone/scan_range.h"

namespace psen_scan_v2_standalone_test
{
using namespace psen_scan_v2_standalone;

static constexpr ScanRange DEFAULT_SCAN_RANGE{ util::TenthOfDegree(1), util::TenthOfDegree(60) };
static constexpr util::TenthOfDegree DEFAULT_SCAN_RESOLUTION{ 2 };
static constexpr int64_t DEFAULT_TIMESTAMP{ 1000000000 };

static double randDouble(double low, double high)
{
  static std::default_random_engine re{};
  using Dist = std::uniform_real_distribution<double>;
  static Dist uid{};
  return uid(re, Dist::param_type{ low, high });
}

static double restrictToOneDigitsAfterComma(const double& value)
{
  return std::round(value * 10.) / 10.;
}

static std::vector<double> generateMeasurements(const unsigned int& num_elements, const double& low, const double& high)
{
  std::vector<double> vec(num_elements);
  // The scanner sends tenth degree values. Therefore, restrict values to one digit after the comma.
  std::generate(vec.begin(), vec.end(), [low, high]() { return restrictToOneDigitsAfterComma(randDouble(low, high)); });
  return vec;
}

static std::vector<double> generateIntensities(const unsigned int& num_elements, const double& low, const double& high)
{
  std::vector<double> vec(num_elements);
  // The scanner sends intensities as int values, therefore, the values are rounded.
  std::generate(vec.begin(), vec.end(), [low, high]() { return std::round(randDouble(low, high)); });
  return vec;
}

static void setPin(PinState& pin_state)
{
  pin_state = PinState(pin_state.id(), pin_state.name(), true);
}

static data_conversion_layer::monitoring_frame::io::PinData createCompleteIOPinData()
{
  using namespace data_conversion_layer::monitoring_frame::io;
  PinData pin_data;
  // First fill everything
  for (std::size_t bit = 0; bit < 8; ++bit)
  {
    for (std::size_t byte = 0; byte < RAW_CHUNK_PHYSICAL_INPUT_SIGNALS_IN_BYTES; ++byte)
    {
      const auto pin_state = createInputPinState(byte, bit, false);
      if (pin_state.name() != "unused")
      {
        pin_data.physical_input_0.push_back(pin_state);
        pin_data.physical_input_1.push_back(pin_state);
        pin_data.physical_input_2.push_back(pin_state);
      }
    }
    for (std::size_t byte = 0; byte < RAW_CHUNK_LOGICAL_INPUT_SIGNALS_IN_BYTES; ++byte)
    {
      const auto pin_state = createLogicalPinState(byte, bit, false);
      if (pin_state.name() != "unused")
      {
        pin_data.logical_input.push_back(pin_state);
      }
    }
    for (std::size_t byte = 0; byte < RAW_CHUNK_OUTPUT_SIGNALS_IN_BYTES; ++byte)
    {
      const auto pin_state = createOutputPinState(byte, bit, false);
      if (pin_state.name() != "unused")
      {
        pin_data.output.push_back(pin_state);
      }
    }
  }
  // Set arbitrary pins
  setPin(pin_data.physical_input_0.at(3));
  setPin(pin_data.physical_input_0.at(11));
  setPin(pin_data.physical_input_2.at(21));
  setPin(pin_data.logical_input.at(8));
  setPin(pin_data.output.at(2));
  return pin_data;
}

static data_conversion_layer::monitoring_frame::Message
createValidMonitoringFrameMsg(const uint32_t scan_counter = 42,
                              const util::TenthOfDegree start_angle = DEFAULT_SCAN_RANGE.getStart(),
                              const util::TenthOfDegree end_angle = DEFAULT_SCAN_RANGE.getEnd(),
                              const uint8_t active_zoneset = 1)
{
  const auto resolution{ util::TenthOfDegree(10) };
  data_conversion_layer::monitoring_frame::MessageBuilder msg_builder;
  msg_builder.fromTheta(start_angle).resolution(resolution).scanCounter(scan_counter).activeZoneset(active_zoneset);

  const unsigned int num_elements = ((end_angle - start_angle) / resolution).value();
  const double lowest_measurement{ 0. };
  const double highest_measurement{ 10. };
  msg_builder.measurements(generateMeasurements(num_elements, lowest_measurement, highest_measurement));

  const double lowest_intensity{ 0. };
  const double highest_intensity{ 16383. };  // only 14 of 16 bits can be used for the actual intensity value

  msg_builder.intensities(generateIntensities(num_elements, lowest_intensity, highest_intensity));

  msg_builder.diagnosticMessages({ { configuration::ScannerId::master,
                                     data_conversion_layer::monitoring_frame::diagnostic::ErrorLocation(1, 7) } });

  msg_builder.iOPinData(createCompleteIOPinData());  // A valid frame contains a complete set of pin data

  return msg_builder.build();
}

static data_conversion_layer::monitoring_frame::Message
createValidMonitoringFrameMsgWithZoneset(const uint8_t active_zoneset)
{
  return createValidMonitoringFrameMsg(42, DEFAULT_SCAN_RANGE.getStart(), DEFAULT_SCAN_RANGE.getEnd(), active_zoneset);
}

std::vector<data_conversion_layer::monitoring_frame::Message>
createValidMonitoringFrameMsgs(const uint32_t scan_counter, const std::size_t num_elements)
{
  std::vector<data_conversion_layer::monitoring_frame::Message> msgs;
  std::generate_n(
      std::back_inserter(msgs), num_elements, [scan_counter]() { return createValidMonitoringFrameMsg(scan_counter); });
  return msgs;
}

std::vector<data_conversion_layer::monitoring_frame::Message>
createMonitoringFrameMsgsForScanRound(const uint32_t scan_counter, const std::size_t num_elements)
{
  std::vector<data_conversion_layer::monitoring_frame::Message> msgs;
  for (std::size_t i = 0; i < num_elements; ++i)
  {
    const util::TenthOfDegree start_angle =
        (DEFAULT_SCAN_RANGE.getEnd() / static_cast<int>(num_elements)) * static_cast<int>(i);
    const util::TenthOfDegree end_angle =
        (DEFAULT_SCAN_RANGE.getEnd() / static_cast<int>(num_elements)) * static_cast<int>(i + 1);
    msgs.push_back(createValidMonitoringFrameMsg(scan_counter, start_angle, end_angle));
  }
  return msgs;
}

static std::vector<data_conversion_layer::monitoring_frame::MessageStamped>
stampMonitoringFrameMsgs(const std::vector<data_conversion_layer::monitoring_frame::Message>& msgs, int64_t stamp)
{
  std::vector<data_conversion_layer::monitoring_frame::MessageStamped> stamped_msgs;
  std::for_each(msgs.begin(), msgs.end(), [&stamped_msgs, stamp](const auto& msg) {
    stamped_msgs.push_back(data_conversion_layer::monitoring_frame::MessageStamped(msg, stamp));
  });
  return stamped_msgs;
}

LaserScan createReferenceScan(const std::vector<data_conversion_layer::monitoring_frame::Message>& msgs,
                              int64_t reference_timestamp)
{
  return data_conversion_layer::LaserScanConverter::toLaserScan(stampMonitoringFrameMsgs(msgs, reference_timestamp));
}

}  // namespace psen_scan_v2_standalone_test

namespace psen_scan_v2_standalone
{
// Avoids too much output with full io pin data
void PrintTo(const LaserScan& scan, std::ostream* os)
{
  *os << fmt::format("LaserScan(timestamp = {} nsec, scanCounter = {}, minScanAngle = {} deg, maxScanAngle = {} deg, "
                     "resolution = {} deg, active_zoneset = {}, measurements = {}, intensities = {}, io_states = ...)",
                     scan.getTimestamp(),
                     scan.getScanCounter(),
                     scan.getMinScanAngle().value() / 10.,
                     scan.getMaxScanAngle().value() / 10.,
                     scan.getScanResolution().value() / 10.,
                     scan.getActiveZoneset(),
                     util::formatRange(scan.getMeasurements()),
                     util::formatRange(scan.getIntensities()));
}
}  // namespace psen_scan_v2_standalone

#endif  // PSEN_SCAN_V2_STANDALONE_TEST_INTEGRATIONTEST_HELPER_H
