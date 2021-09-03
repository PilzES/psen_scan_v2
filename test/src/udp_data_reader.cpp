// Copyright (c) 2021 Pilz GmbH & Co. KG
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

#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include <boost/endian/conversion.hpp>

#include <fmt/format.h>

#include "psen_scan_v2_standalone/util/logging.h"

#include "psen_scan_v2/udp_data_reader.h"

namespace psen_scan_v2_test
{
void UdpDataReader::read(const std::string& filename, const uint16_t port, UdpData& udp_data)
{
  std::ifstream filestr{ filename };
  if (!filestr.is_open())
  {
    PSENSCAN_ERROR("UdpDataReader", "Could not open file {}", filename);
    return;
  }

  std::string line;
  while (std::getline(filestr, line))
  {
    try
    {
      UdpDatum udp_datum;

      std::istringstream is(line);
      if (readAndCheckPortNumber(is, port))
      {
        readTimestamp(is, udp_datum.timestamp_sec_);
        readHexValue(is, "scan_counter:", udp_datum.scan_counter_);
        readHexValue(is, "from_theta:", udp_datum.from_theta_);

        udp_data.push_back(udp_datum);
      }
    }
    catch (const std::ios_base::failure& f)
    {
      PSENSCAN_WARN("UdpDataReader", f.what());
    }
  }
  PSENSCAN_INFO("UdpDataReader", "Read data from {} udp packets.", udp_data.size());
  filestr.close();
}

bool UdpDataReader::readAndCheckPortNumber(std::istringstream& is, const uint16_t expected_port)
{
  std::string key;
  uint16_t udp_port;
  if (!(is >> key && key == "udp_port:" && is >> std::dec >> udp_port))
  {
    throw std::ios_base::failure("Could not read port number from udp data file.");
  }
  return udp_port == expected_port;
}

template <typename T>
void UdpDataReader::readHexValue(std::istringstream& is, const std::string& expected_key, T& value)
{
  std::string key;
  if (!(is >> key && key == expected_key && is >> std::hex >> value))
  {
    throw std::ios_base::failure(fmt::format("Could not read {} from udp data file.", expected_key));
  }
  boost::endian::big_to_native_inplace(value);
}

void UdpDataReader::readTimestamp(std::istringstream& is, double& value)
{
  std::string key;
  if (!(is >> key && key == "time_epoch:" && is >> std::dec >> value))
  {
    throw std::ios_base::failure("Could not read timestamp from udp data file.");
  }
}

}  // namespace psen_scan_v2_test
