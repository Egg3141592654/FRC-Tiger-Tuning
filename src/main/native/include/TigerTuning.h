#pragma once

#include <frc/geometry/Pose2d.h>
#include <frc/geometry/Rotation3d.h>
#include <frc/geometry/Translation3d.h>
#include <wpi/json.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

/**
 * Singleton configuration cache backed by a JSON file.
 *
 * Values are read from the JSON file at startup. When the SmartDashboard key
 * kOverrideKey is set to true, getters first check SmartDashboard (seeding it
 * with JSON values on first enable), then fall back to the JSON cache, then to
 * the caller-supplied default.
 *
 * Call Initialize() before the first GetInstance() if you need a non-default
 * file path.
 */
class TigerTuning {
 public:
  static constexpr std::string_view kOverrideKey = "TigerTuning/OverrideEnabled";
  static constexpr std::string_view kDefaultFilePath = "/home/lvuser/deploy/tigertuning.json";

  /** Returns the singleton instance, creating it with the default file path if necessary. */
  static TigerTuning& GetInstance();

  /**
   * Resets the singleton with the given file path.
   *
   * Call before the first GetInstance() if you need a non-default path.
   *
   * @param filePath path to the JSON configuration file
   */
  static void Initialize(std::string_view filePath);

  TigerTuning(const TigerTuning&) = delete;
  TigerTuning& operator=(const TigerTuning&) = delete;

  /**
   * Returns the value stored under @p key as a double.
   *
   * Returns @p defaultValue when the key is absent or not a number. When the
   * SmartDashboard override is active the SmartDashboard entry is returned
   * instead, seeded from the JSON value on first enable.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or @p defaultValue
   */
  double GetDouble(std::string_view key, double defaultValue);

  /**
   * Returns the value stored under @p key as an int.
   *
   * Returns @p defaultValue when the key is absent or not a number. When the
   * SmartDashboard override is active the SmartDashboard entry (read as a
   * double and truncated) is returned instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or @p defaultValue
   */
  int GetInt(std::string_view key, int defaultValue);

  /**
   * Returns the value stored under @p key as a bool.
   *
   * Returns @p defaultValue when the key is absent or not a boolean. When the
   * SmartDashboard override is active the SmartDashboard entry is returned
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or @p defaultValue
   */
  bool GetBoolean(std::string_view key, bool defaultValue);

  /**
   * Returns the value stored under @p key as a std::string.
   *
   * Returns @p defaultValue when the key is absent or not a string. When the
   * SmartDashboard override is active the SmartDashboard entry is returned
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or @p defaultValue
   */
  std::string GetString(std::string_view key, std::string_view defaultValue);

  /**
   * Returns the value stored under @p key as a vector of doubles.
   *
   * Non-numeric elements within the JSON array are silently skipped. Returns
   * @p defaultValue when the key is absent or not an array. When the
   * SmartDashboard override is active the SmartDashboard entry is returned
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured array, or @p defaultValue
   */
  std::vector<double> GetDoubleArray(std::string_view key, std::vector<double> defaultValue = {});

  /**
   * Returns the value stored under @p key as a vector of strings.
   *
   * Non-string elements within the JSON array are silently skipped. Returns
   * @p defaultValue when the key is absent or not an array. When the
   * SmartDashboard override is active the SmartDashboard entry is returned
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured array, or @p defaultValue
   */
  std::vector<std::string> GetStringArray(std::string_view key,
                                          std::vector<std::string> defaultValue = {});

  /**
   * Returns the value stored under @p key as a Pose2d.
   *
   * Expects a JSON object with numeric fields @c x (metres), @c y (metres),
   * and @c rotation (degrees). Any absent field falls back to the
   * corresponding field of @p defaultValue. When the SmartDashboard override
   * is active, sub-keys @c key/x, @c key/y, and @c key/rotation are read
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured pose, or @p defaultValue
   */
  frc::Pose2d GetPose2d(std::string_view key, frc::Pose2d defaultValue = {});

  /**
   * Returns the value stored under @p key as a Translation3d.
   *
   * Expects a JSON object with numeric fields @c x, @c y, and @c z (all
   * metres). Any absent field falls back to the corresponding field of
   * @p defaultValue. When the SmartDashboard override is active, sub-keys
   * @c key/x, @c key/y, and @c key/z are read instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured translation, or @p defaultValue
   */
  frc::Translation3d GetTranslation3d(std::string_view key, frc::Translation3d defaultValue = {});

  /**
   * Returns the value stored under @p key as a Rotation3d.
   *
   * Expects a JSON object with numeric fields @c roll, @c pitch, and @c yaw
   * (all degrees). Any absent field falls back to the corresponding field of
   * @p defaultValue. When the SmartDashboard override is active, sub-keys
   * @c key/roll, @c key/pitch, and @c key/yaw are read instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured rotation, or @p defaultValue
   */
  frc::Rotation3d GetRotation3d(std::string_view key, frc::Rotation3d defaultValue = {});

 private:
  explicit TigerTuning(std::string_view filePath);

  /** Returns true if override is active, seeding SmartDashboard on first transition. */
  bool CheckOverride();
  void SeedSmartDashboard();

  wpi::json m_cache;
  bool m_prevOverride = false;
};
