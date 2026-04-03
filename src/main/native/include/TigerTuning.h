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

  static TigerTuning& GetInstance();
  static void Initialize(std::string_view filePath);

  TigerTuning(const TigerTuning&) = delete;
  TigerTuning& operator=(const TigerTuning&) = delete;

  double GetDouble(std::string_view key, double defaultValue);
  int GetInt(std::string_view key, int defaultValue);
  bool GetBoolean(std::string_view key, bool defaultValue);
  std::string GetString(std::string_view key, std::string_view defaultValue);

  // Complex types
  // Arrays map to SmartDashboard array entries when override is active.
  std::vector<double> GetDoubleArray(std::string_view key, std::vector<double> defaultValue = {});
  std::vector<std::string> GetStringArray(std::string_view key,
                                          std::vector<std::string> defaultValue = {});

  // Geometry types — JSON objects with named numeric fields (distances in
  // metres, angles in degrees).
  //   Pose2d:       {"x": 0.0, "y": 0.0, "rotation": 0.0}
  //   Translation3d: {"x": 0.0, "y": 0.0, "z": 0.0}
  //   Rotation3d:   {"roll": 0.0, "pitch": 0.0, "yaw": 0.0}
  frc::Pose2d GetPose2d(std::string_view key, frc::Pose2d defaultValue = {});
  frc::Translation3d GetTranslation3d(std::string_view key, frc::Translation3d defaultValue = {});
  frc::Rotation3d GetRotation3d(std::string_view key, frc::Rotation3d defaultValue = {});

 private:
  explicit TigerTuning(std::string_view filePath);

  /** Returns true if override is active, seeding SmartDashboard on first transition. */
  bool CheckOverride();
  void SeedSmartDashboard();

  wpi::json m_cache;
  bool m_prevOverride = false;
};
