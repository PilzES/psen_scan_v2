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

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <psen_scan_v2_standalone/core.h>

using namespace std::chrono_literals;
using namespace psen_scan_v2_standalone;

const std::string SCANNER_IP{ "192.168.0.10" };
const util::TenthOfDegree ANGLE_START{ 1 };
const util::TenthOfDegree ANGLE_END{ 2749 };

bool startScanner(const ScannerConfiguration& config)
{
  auto scanner{ std::make_unique<ScannerV2>(config, [](const LaserScan& scan) {}) };

  const auto start_future{ scanner->start() };
  const auto start_status{ start_future.wait_for(5s) };
  if (start_status == std::future_status::timeout)
  {
    PSENSCAN_ERROR("startScanner()", "Scanner did not start properly");
    return false;
  }
  return true;
}

bool stopScanner(const ScannerConfiguration& config)
{
  auto scanner{ std::make_unique<ScannerV2>(config, [](const LaserScan& scan) {}) };

  const auto stop_future{ scanner->stop() };
  const auto stop_status{ stop_future.wait_for(5s) };
  if (stop_status == std::future_status::timeout)
  {
    PSENSCAN_ERROR("main", "Scanner did not stop properly");
    return false;
  }
  return true;
}

int main(int argc, char** argv)
{
  setLogLevel(CONSOLE_BRIDGE_LOG_INFO);

  ScannerConfigurationBuilder config_builder;
  config_builder.scannerIp(SCANNER_IP).scanRange(ScanRange{ ANGLE_START, ANGLE_END });
  const auto config = config_builder.build();

  if (!startScanner(config))
  {
    return 1;
  }
  std::this_thread::sleep_for(20min);
  return !stopScanner(config);
}
