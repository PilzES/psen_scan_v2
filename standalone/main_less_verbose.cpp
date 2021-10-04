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
#include <string>
#include <thread>

#include <psen_scan_v2_standalone/core.h>

using namespace psen_scan_v2_standalone;

const std::string SCANNER_IP{ "192.168.0.10" };
const util::TenthOfDegree ANGLE_START{ 1 };
const util::TenthOfDegree ANGLE_END{ 2749 };
constexpr bool FRAGMENTED_ENABLED{ true };

void laserScanCallback(const LaserScan& scan)
{
  if (scan.getMinScanAngle() == 500)
  {
    PSENSCAN_INFO("laserScanCallback()", "timestamp: {}", scan.getTimestamp());
  }
}

int main(int argc, char** argv)
{
  setLogLevel(CONSOLE_BRIDGE_LOG_INFO);

  ScannerConfigurationBuilder config_builder;
  config_builder.scannerIp(SCANNER_IP)
      .scanRange(ScanRange{ ANGLE_START, ANGLE_END })
      .enableFragmentedScans(FRAGMENTED_ENABLED);

  ScannerV2 scanner(config_builder.build(), laserScanCallback);

  scanner.start();
  std::this_thread::sleep_for(std::chrono::minutes(20));
  scanner.stop();

  return 0;
}
