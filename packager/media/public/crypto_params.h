// Copyright 2017 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef PACKAGER_MEDIA_PUBLIC_CRYPTO_PARAMS_H_
#define PACKAGER_MEDIA_PUBLIC_CRYPTO_PARAMS_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace shaka {

/// Encryption / decryption key providers.
enum class KeyProvider {
  kNone = 0,
  kWidevine = 1,
  kPlayready = 2,
  kRawKey = 3,
};

/// Signer credential for Widevine license server.
struct WidevineSigner {
  /// Name of the signer / content provider.
  std::string signer_name;

  enum class SigningKeyType {
    kNone,
    kAes,
    kRsa,
  };
  /// Specifies the signing key type, which determines whether AES or RSA key
  /// are used to authenticate the signer. A type of 'kNone' is invalid.
  SigningKeyType signing_key_type = SigningKeyType::kNone;
  struct {
    /// AES signing key.
    std::vector<uint8_t> key;
    /// AES signing IV.
    std::vector<uint8_t> iv;
  } aes;
  struct {
    /// RSA signing private key.
    std::string key;
  } rsa;
};

/// Widevine encryption parameters.
struct WidevineEncryptionParams {
  /// Widevine license / key server URL.
  std::string key_server_url;
  /// Generates and includes an additional v1 PSSH box for the common system ID.
  /// See: https://goo.gl/s8RIhr.
  // TODO(kqyang): Move to EncryptionParams and support common PSSH generation
  // in all key providers.
  bool include_common_pssh = false;
  /// Content identifier.
  std::vector<uint8_t> content_id;
  /// The name of a stored policy, which specifies DRM content rights.
  std::string policy;
  /// Signer credential for Widevine license / key server.
  WidevineSigner signer;
  /// Group identifier, if present licenses will belong to this group.
  std::vector<uint8_t> group_id;
};

/// Playready encryption parameters.
/// Two different modes of playready key acquisition is supported:
///   (1) Fetch from a key server. `key_server_url` and `program_identifier` are
///       required. The presence of other parameters may be necessary depends
///       on server configuration.
///   (2) Provide the raw key directly. Both `key_id` and `key` are required.
///       We are planning to merge this mode with `RawKeyEncryptionParams`.
struct PlayreadyEncryptionParams {
  /// Playready license / key server URL.
  std::string key_server_url;
  /// Playready program identifier.
  std::string program_identifier;
  /// Absolute path to the Certificate Authority file for the server cert in PEM
  /// format.
  std::string ca_file;
  /// Absolute path to client certificate file.
  std::string client_cert_file;
  /// Absolute path to the private key file.
  std::string client_cert_private_key_file;
  /// Password to the private key file.
  std::string client_cert_private_key_password;

  // TODO(kqyang): move raw playready key generation to RawKey.
  /// Provides a raw Playready KeyId.
  std::vector<uint8_t> key_id;
  /// Provides a raw Playready Key.
  std::vector<uint8_t> key;
};

/// Raw key encryption parameters, i.e. with key parameters provided.
struct RawKeyEncryptionParams {
  /// An optional initialization vector. If not provided, a random `iv` will be
  /// generated. Note that this parameter should only be used during testing.
  std::vector<uint8_t> iv;
  /// Inject a custom `pssh` or multiple concatenated `psshs`. If not provided,
  /// a common system pssh will be generated.
  std::vector<uint8_t> pssh;

  using StreamLabel = std::string;
  struct KeyPair {
    std::vector<uint8_t> key_id;
    std::vector<uint8_t> key;
  };
  /// Defines the KeyPair for the streams. An empty `StreamLabel` indicates the
  /// default `KeyPair`, which applies to all the `StreamLabels` not present in
  /// `key_map`.
  std::map<StreamLabel, KeyPair> key_map;
};

/// Encryption parameters.
struct EncryptionParams {
  /// Specifies the key provider, which determines which key provider is used
  /// and which encryption params is valid. 'kNone' means not to encrypt the
  /// streams.
  KeyProvider key_provider = KeyProvider::kNone;
  // Only one of the three fields is valid.
  WidevineEncryptionParams widevine;
  PlayreadyEncryptionParams playready;
  RawKeyEncryptionParams raw_key;

  /// Clear lead duration in seconds.
  double clear_lead_in_seconds = 0;
  /// The protection scheme: "cenc", "cens", "cbc1", "cbcs".
  static constexpr uint32_t kProtectionSchemeCenc = 0x63656E63;
  static constexpr uint32_t kProtectionSchemeCbc1 = 0x63626331;
  static constexpr uint32_t kProtectionSchemeCens = 0x63656E73;
  static constexpr uint32_t kProtectionSchemeCbcs = 0x63626373;
  uint32_t protection_scheme = kProtectionSchemeCenc;
  /// Crypto period duration in seconds. A positive value means key rotation is
  /// enabled, the key provider must support key rotation in this case.
  static constexpr double kNoKeyRotation = 0;
  double crypto_period_duration_in_seconds = kNoKeyRotation;
  /// Enable/disable subsample encryption for VP9.
  bool vp9_subsample_encryption = true;

  /// Encrypted stream information that is used to determine stream label.
  struct EncryptedStreamAttributes {
    enum StreamType {
      kUnknown,
      kVideo,
      kAudio,
    };

    StreamType stream_type = kUnknown;
    union OneOf {
      OneOf() {}

      struct {
        int width = 0;
        int height = 0;
        float frame_rate = 0;
        int bit_depth = 0;
      } video;

      struct {
        int number_of_channels = 0;
      } audio;
    } oneof;
  };
  /// Stream label function assigns a stream label to the stream to be
  /// encrypted. Stream label is used to associate KeyPair with streams. Streams
  /// with the same stream label always uses the same keyPair; Streams with
  /// different stream label could use the same or different KeyPairs.
  /// A default stream label function will be generated if not set.
  std::function<std::string(const EncryptedStreamAttributes& stream_attributes)>
      stream_label_func;
};

/// Widevine decryption parameters.
struct WidevineDecryptionParams {
  /// Widevine license / key server URL.
  std::string key_server_url;
  /// Signer credential for Widevine license / key server.
  WidevineSigner signer;
};

/// Raw key decryption parameters, i.e. with key parameters provided.
struct RawKeyDecryptionParams {
  using StreamLabel = std::string;
  struct KeyPair {
    std::vector<uint8_t> key_id;
    std::vector<uint8_t> key;
  };
  /// Defines the KeyPair for the streams. An empty `StreamLabel` indicates the
  /// default `KeyPair`, which applies to all the `StreamLabels` not present in
  /// `key_map`.
  std::map<StreamLabel, KeyPair> key_map;
};

/// Decryption parameters.
struct DecryptionParams {
  /// Specifies the key provider, which determines which key provider is used
  /// and which encryption params is valid. 'kNone' means not to decrypt the
  /// streams.
  KeyProvider key_provider = KeyProvider::kNone;
  // Only one of the two fields is valid.
  WidevineDecryptionParams widevine;
  RawKeyDecryptionParams raw_key;
};

}  // namespace shaka

#endif  // PACKAGER_MEDIA_PUBLIC_CRYPTO_PARAMS_H_