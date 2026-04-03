#include "TigerTuning.h"

#include <frc/DriverStation.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <units/angle.h>
#include <units/length.h>

#include <fstream>

static std::unique_ptr<TigerTuning> s_instance;

TigerTuning& TigerTuning::GetInstance() {
  if (!s_instance) {
    s_instance.reset(new TigerTuning(kDefaultFilePath));
  }
  return *s_instance;
}

void TigerTuning::Initialize(std::string_view filePath) {
  s_instance.reset(new TigerTuning(filePath));
}

TigerTuning::TigerTuning(std::string_view filePath) {
  std::ifstream file{std::string(filePath)};
  if (file.is_open()) {
    m_cache = wpi::json::parse(file, nullptr, false);
    if (m_cache.is_discarded()) {
      m_cache = wpi::json::object();
    }
  } else {
    m_cache = wpi::json::object();
  }
}

bool TigerTuning::CheckOverride() {
  // If there is an FMS, never accept overrides, only use the JSON onboard.
  if (frc::DriverStation::IsFMSAttached()) {
    return false;
  }

  bool override = frc::SmartDashboard::GetBoolean(kOverrideKey, false);
  if (override && !m_prevOverride) {
    SeedSmartDashboard();
  }
  m_prevOverride = override;
  return override;
}

void TigerTuning::SeedSmartDashboard() {
  for (auto& [key, value] : m_cache.items()) {
    if (value.is_boolean()) {
      frc::SmartDashboard::PutBoolean(key, value.get<bool>());
    } else if (value.is_number()) {
      frc::SmartDashboard::PutNumber(key, value.get<double>());
    } else if (value.is_string()) {
      frc::SmartDashboard::PutString(key, value.get<std::string>());
    } else if (value.is_object()) {
      for (auto& [subKey, subValue] : value.items()) {
        if (subValue.is_number()) {
          frc::SmartDashboard::PutNumber(key + "/" + subKey, subValue.get<double>());
        }
      }
    } else if (value.is_array() && !value.empty()) {
      if (value[0].is_number()) {
        std::vector<double> arr;
        for (auto& v : value) {
          arr.push_back(v.get<double>());
        }
        frc::SmartDashboard::PutNumberArray(key, arr);
      } else if (value[0].is_string()) {
        std::vector<std::string> arr;
        for (auto& v : value) {
          arr.push_back(v.get<std::string>());
        }
        frc::SmartDashboard::PutStringArray(key, arr);
      }
    }
  }
}

double TigerTuning::GetDouble(std::string_view key, double defaultValue) {
  double jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_number()) {
    jsonValue = it->get<double>();
  }
  if (CheckOverride()) {
    return frc::SmartDashboard::GetNumber(key, jsonValue);
  }
  return jsonValue;
}

int TigerTuning::GetInt(std::string_view key, int defaultValue) {
  int jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_number()) {
    jsonValue = it->get<int>();
  }
  if (CheckOverride()) {
    return static_cast<int>(frc::SmartDashboard::GetNumber(key, jsonValue));
  }
  return jsonValue;
}

bool TigerTuning::GetBoolean(std::string_view key, bool defaultValue) {
  bool jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_boolean()) {
    jsonValue = it->get<bool>();
  }
  if (CheckOverride()) {
    return frc::SmartDashboard::GetBoolean(key, jsonValue);
  }
  return jsonValue;
}

std::string TigerTuning::GetString(std::string_view key, std::string_view defaultValue) {
  std::string jsonValue{defaultValue};
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_string()) {
    jsonValue = it->get<std::string>();
  }
  if (CheckOverride()) {
    return frc::SmartDashboard::GetString(key, jsonValue);
  }
  return jsonValue;
}

std::vector<double> TigerTuning::GetDoubleArray(std::string_view key,
                                                std::vector<double> defaultValue) {
  std::vector<double> jsonValue = std::move(defaultValue);
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_array()) {
    jsonValue.clear();
    for (auto& v : *it) {
      if (v.is_number()) {
        jsonValue.push_back(v.get<double>());
      }
    }
  }
  if (CheckOverride()) {
    return frc::SmartDashboard::GetNumberArray(key, std::span(jsonValue));
  }
  return jsonValue;
}

std::vector<std::string> TigerTuning::GetStringArray(std::string_view key,
                                                     std::vector<std::string> defaultValue) {
  std::vector<std::string> jsonValue = std::move(defaultValue);
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_array()) {
    jsonValue.clear();
    for (auto& v : *it) {
      if (v.is_string()) {
        jsonValue.push_back(v.get<std::string>());
      }
    }
  }
  if (CheckOverride()) {
    return frc::SmartDashboard::GetStringArray(key, std::span(jsonValue));
  }
  return jsonValue;
}

frc::Pose2d TigerTuning::GetPose2d(std::string_view key, frc::Pose2d defaultValue) {
  frc::Pose2d jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_object()) {
    double x = it->value("x", defaultValue.X().value());
    double y = it->value("y", defaultValue.Y().value());
    double rot = it->value("rotation", defaultValue.Rotation().Degrees().value());
    jsonValue = frc::Pose2d{units::meter_t{x}, units::meter_t{y}, units::degree_t{rot}};
  }
  if (CheckOverride()) {
    std::string k{key};
    double x = frc::SmartDashboard::GetNumber(k + "/x", jsonValue.X().value());
    double y = frc::SmartDashboard::GetNumber(k + "/y", jsonValue.Y().value());
    double rot =
        frc::SmartDashboard::GetNumber(k + "/rotation", jsonValue.Rotation().Degrees().value());
    return frc::Pose2d{units::meter_t{x}, units::meter_t{y}, units::degree_t{rot}};
  }
  return jsonValue;
}

frc::Translation3d TigerTuning::GetTranslation3d(std::string_view key,
                                                 frc::Translation3d defaultValue) {
  frc::Translation3d jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_object()) {
    double x = it->value("x", defaultValue.X().value());
    double y = it->value("y", defaultValue.Y().value());
    double z = it->value("z", defaultValue.Z().value());
    jsonValue = frc::Translation3d{units::meter_t{x}, units::meter_t{y}, units::meter_t{z}};
  }
  if (CheckOverride()) {
    std::string k{key};
    double x = frc::SmartDashboard::GetNumber(k + "/x", jsonValue.X().value());
    double y = frc::SmartDashboard::GetNumber(k + "/y", jsonValue.Y().value());
    double z = frc::SmartDashboard::GetNumber(k + "/z", jsonValue.Z().value());
    return frc::Translation3d{units::meter_t{x}, units::meter_t{y}, units::meter_t{z}};
  }
  return jsonValue;
}

frc::Rotation3d TigerTuning::GetRotation3d(std::string_view key, frc::Rotation3d defaultValue) {
  frc::Rotation3d jsonValue = defaultValue;
  auto it = m_cache.find(std::string(key));
  if (it != m_cache.end() && it->is_object()) {
    double roll = it->value("roll", units::degree_t{defaultValue.X()}.value());
    double pitch = it->value("pitch", units::degree_t{defaultValue.Y()}.value());
    double yaw = it->value("yaw", units::degree_t{defaultValue.Z()}.value());
    jsonValue =
        frc::Rotation3d{units::degree_t{roll}, units::degree_t{pitch}, units::degree_t{yaw}};
  }
  if (CheckOverride()) {
    std::string k{key};
    double roll =
        frc::SmartDashboard::GetNumber(k + "/roll", units::degree_t{jsonValue.X()}.value());
    double pitch =
        frc::SmartDashboard::GetNumber(k + "/pitch", units::degree_t{jsonValue.Y()}.value());
    double yaw = frc::SmartDashboard::GetNumber(k + "/yaw", units::degree_t{jsonValue.Z()}.value());
    return frc::Rotation3d{units::degree_t{roll}, units::degree_t{pitch}, units::degree_t{yaw}};
  }
  return jsonValue;
}
