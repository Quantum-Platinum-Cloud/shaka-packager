// Copyright 2014 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "packager/media/base/stream_info.h"

#include <inttypes.h>

#include "packager/base/logging.h"
#include "packager/base/strings/stringprintf.h"

namespace shaka {
namespace media {

StreamInfo::StreamInfo(StreamType stream_type,
                       int track_id,
                       uint32_t time_scale,
                       uint64_t duration,
                       Codec codec,
                       const std::string& codec_string,
                       const uint8_t* codec_config,
                       size_t codec_config_size,
                       const std::string& language,
                       bool is_encrypted)
    : stream_type_(stream_type),
      track_id_(track_id),
      time_scale_(time_scale),
      duration_(duration),
      codec_(codec),
      codec_string_(codec_string),
      language_(language),
      is_encrypted_(is_encrypted) {
  if (codec_config_size > 0) {
    codec_config_.assign(codec_config, codec_config + codec_config_size);
  }
}

StreamInfo::~StreamInfo() {}

std::string StreamInfo::ToString() const {
  return base::StringPrintf(
      "type: %s\n codec_string: %s\n time_scale: %d\n duration: "
      "%" PRIu64 " (%.1f seconds)\n is_encrypted: %s\n",
      (stream_type_ == kStreamAudio ? "Audio" : "Video"), codec_string_.c_str(),
      time_scale_, duration_, static_cast<double>(duration_) / time_scale_,
      is_encrypted_ ? "true" : "false");
}

}  // namespace media
}  // namespace shaka
