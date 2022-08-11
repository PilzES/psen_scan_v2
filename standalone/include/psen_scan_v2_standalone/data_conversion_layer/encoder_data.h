// Copyright (c) 2021-2022 Pilz GmbH & Co. KG
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

#ifndef PSEN_SCAN_V2_STANDALONE_ENCODER_PIN_DATA_H
#define PSEN_SCAN_V2_STANDALONE_ENCODER_PIN_DATA_H

#include <vector>
#include <ostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "psen_scan_v2_standalone/util/format_range.h"

namespace psen_scan_v2_standalone
{
namespace data_conversion_layer
{
namespace monitoring_frame
{
/**
 * @brief Contains all types, etc. needed to describe the encoder information contained
 * in a  data_conversion_layer::monitoring_frame::Message.
 */
namespace encoder
{

struct EncoderData
{
  double encoder_1;
  double encoder_2;
};

inline std::ostream& operator<<(std::ostream& os, const EncoderData& ed)
{
  return os << fmt::format("EncoderData(encoder_1 = {}, encoder_2 = {})",
                           ed.encoder_1,
                           ed.encoder_2);
}

}  // namespace encoder
}  // namespace monitoring_frame
}  // namespace data_conversion_layer
}  // namespace psen_scan_v2_standalone
#endif  // PSEN_SCAN_V2_STANDALONE_ENCODER_PIN_DATA_H
