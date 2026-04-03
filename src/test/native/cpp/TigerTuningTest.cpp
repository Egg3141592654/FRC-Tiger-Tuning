#include <units/angle.h>
#include <units/length.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "TigerTuning.h"
#include "gtest/gtest.h"

namespace {

std::string WriteTempJson(const std::string& content) {
  auto path = std::filesystem::temp_directory_path() / "tigertuning_test.json";
  std::ofstream f(path);
  f << content;
  return path.string();
}

class TigerTuningTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset the singleton to a non-existent file before each test.
    TigerTuning::Initialize("/nonexistent/tigertuning_test.json");
  }
};

// --- Default value tests (file not found) ---

TEST_F(TigerTuningTest, GetDoubleReturnsDefaultWhenFileNotFound) {
  EXPECT_DOUBLE_EQ(1.5, TigerTuning::GetInstance().GetDouble("key", 1.5));
}

TEST_F(TigerTuningTest, GetIntReturnsDefaultWhenFileNotFound) {
  EXPECT_EQ(42, TigerTuning::GetInstance().GetInt("key", 42));
}

TEST_F(TigerTuningTest, GetBooleanReturnsDefaultWhenFileNotFound) {
  EXPECT_TRUE(TigerTuning::GetInstance().GetBoolean("key", true));
  EXPECT_FALSE(TigerTuning::GetInstance().GetBoolean("key", false));
}

TEST_F(TigerTuningTest, GetStringReturnsDefaultWhenFileNotFound) {
  EXPECT_EQ("default", TigerTuning::GetInstance().GetString("key", "default"));
}

TEST_F(TigerTuningTest, GetDoubleArrayReturnsDefaultWhenFileNotFound) {
  std::vector<double> def = {1.0, 2.0};
  EXPECT_EQ(def, TigerTuning::GetInstance().GetDoubleArray("key", def));
}

TEST_F(TigerTuningTest, GetStringArrayReturnsDefaultWhenFileNotFound) {
  std::vector<std::string> def = {"a", "b"};
  EXPECT_EQ(def, TigerTuning::GetInstance().GetStringArray("key", def));
}

// --- Values loaded from JSON ---

TEST_F(TigerTuningTest, LoadsDoubleFromJson) {
  auto path = WriteTempJson(R"({"speed": 3.14})");
  TigerTuning::Initialize(path);
  EXPECT_DOUBLE_EQ(3.14, TigerTuning::GetInstance().GetDouble("speed", 0.0));
}

TEST_F(TigerTuningTest, LoadsIntFromJson) {
  auto path = WriteTempJson(R"({"count": 7})");
  TigerTuning::Initialize(path);
  EXPECT_EQ(7, TigerTuning::GetInstance().GetInt("count", 0));
}

TEST_F(TigerTuningTest, LoadsBooleanFromJson) {
  auto path = WriteTempJson(R"({"enabled": true})");
  TigerTuning::Initialize(path);
  EXPECT_TRUE(TigerTuning::GetInstance().GetBoolean("enabled", false));
}

TEST_F(TigerTuningTest, LoadsStringFromJson) {
  auto path = WriteTempJson(R"({"name": "robot"})");
  TigerTuning::Initialize(path);
  EXPECT_EQ("robot", TigerTuning::GetInstance().GetString("name", ""));
}

TEST_F(TigerTuningTest, LoadsDoubleArrayFromJson) {
  auto path = WriteTempJson(R"({"gains": [1.0, 2.5, 3.0]})");
  TigerTuning::Initialize(path);
  std::vector<double> expected = {1.0, 2.5, 3.0};
  EXPECT_EQ(expected, TigerTuning::GetInstance().GetDoubleArray("gains", {}));
}

TEST_F(TigerTuningTest, LoadsStringArrayFromJson) {
  auto path = WriteTempJson(R"({"labels": ["alpha", "beta"]})");
  TigerTuning::Initialize(path);
  std::vector<std::string> expected = {"alpha", "beta"};
  EXPECT_EQ(expected, TigerTuning::GetInstance().GetStringArray("labels", {}));
}

TEST_F(TigerTuningTest, LoadsMultipleTypesFromJson) {
  auto path = WriteTempJson(R"({"speed": 3.14, "count": 5, "enabled": true, "name": "robot"})");
  TigerTuning::Initialize(path);
  auto& tt = TigerTuning::GetInstance();
  EXPECT_DOUBLE_EQ(3.14, tt.GetDouble("speed", 0.0));
  EXPECT_EQ(5, tt.GetInt("count", 0));
  EXPECT_TRUE(tt.GetBoolean("enabled", false));
  EXPECT_EQ("robot", tt.GetString("name", ""));
}

// --- Missing key returns default ---

TEST_F(TigerTuningTest, MissingKeyReturnsDefault) {
  auto path = WriteTempJson(R"({"other": 99})");
  TigerTuning::Initialize(path);
  auto& tt = TigerTuning::GetInstance();
  EXPECT_DOUBLE_EQ(5.0, tt.GetDouble("missing", 5.0));
  EXPECT_EQ(3, tt.GetInt("missing", 3));
  EXPECT_FALSE(tt.GetBoolean("missing", false));
  EXPECT_EQ("def", tt.GetString("missing", "def"));
}

// --- Malformed JSON falls back to defaults ---

TEST_F(TigerTuningTest, MalformedJsonFallsBackToDefaults) {
  auto path = WriteTempJson("{ this is not valid json }");
  TigerTuning::Initialize(path);
  auto& tt = TigerTuning::GetInstance();
  EXPECT_DOUBLE_EQ(9.0, tt.GetDouble("key", 9.0));
  EXPECT_EQ(1, tt.GetInt("key", 1));
  EXPECT_TRUE(tt.GetBoolean("key", true));
  EXPECT_EQ("fallback", tt.GetString("key", "fallback"));
}

// --- Type mismatches return default ---

TEST_F(TigerTuningTest, StringValueReturnsDefaultForDouble) {
  auto path = WriteTempJson(R"({"val": "notanumber"})");
  TigerTuning::Initialize(path);
  EXPECT_DOUBLE_EQ(9.9, TigerTuning::GetInstance().GetDouble("val", 9.9));
}

TEST_F(TigerTuningTest, StringValueReturnsDefaultForInt) {
  auto path = WriteTempJson(R"({"val": "notanumber"})");
  TigerTuning::Initialize(path);
  EXPECT_EQ(7, TigerTuning::GetInstance().GetInt("val", 7));
}

TEST_F(TigerTuningTest, NumberValueReturnsDefaultForBoolean) {
  auto path = WriteTempJson(R"({"val": 42})");
  TigerTuning::Initialize(path);
  // JSON number is not a boolean; default should be returned
  EXPECT_TRUE(TigerTuning::GetInstance().GetBoolean("val", true));
}

TEST_F(TigerTuningTest, NumberValueReturnsDefaultForString) {
  auto path = WriteTempJson(R"({"val": 42})");
  TigerTuning::Initialize(path);
  EXPECT_EQ("def", TigerTuning::GetInstance().GetString("val", "def"));
}

// --- Empty JSON object ---

TEST_F(TigerTuningTest, EmptyJsonObjectReturnsDefaults) {
  auto path = WriteTempJson("{}");
  TigerTuning::Initialize(path);
  auto& tt = TigerTuning::GetInstance();
  EXPECT_DOUBLE_EQ(1.0, tt.GetDouble("key", 1.0));
  EXPECT_EQ(2, tt.GetInt("key", 2));
  EXPECT_FALSE(tt.GetBoolean("key", false));
  EXPECT_EQ("x", tt.GetString("key", "x"));
}

// --- Array edge cases ---

TEST_F(TigerTuningTest, EmptyDoubleArrayInJsonReturnsEmptyVector) {
  auto path = WriteTempJson(R"({"arr": []})");
  TigerTuning::Initialize(path);
  // Key exists and is array: default is replaced by the (empty) JSON array
  EXPECT_TRUE(TigerTuning::GetInstance().GetDoubleArray("arr", {9.0}).empty());
}

TEST_F(TigerTuningTest, MixedDoubleArraySkipsNonNumbers) {
  auto path = WriteTempJson(R"({"arr": [1.0, "skip", 3.0]})");
  TigerTuning::Initialize(path);
  std::vector<double> expected = {1.0, 3.0};
  EXPECT_EQ(expected, TigerTuning::GetInstance().GetDoubleArray("arr", {}));
}

TEST_F(TigerTuningTest, StringArrayWithNonStringsSkipsNonStrings) {
  auto path = WriteTempJson(R"({"arr": ["a", 42, "b"]})");
  TigerTuning::Initialize(path);
  std::vector<std::string> expected = {"a", "b"};
  EXPECT_EQ(expected, TigerTuning::GetInstance().GetStringArray("arr", {}));
}

// --- Pose2d ---

TEST_F(TigerTuningTest, GetPose2dReturnsDefaultWhenFileNotFound) {
  frc::Pose2d def{1_m, 2_m, 45_deg};
  auto result = TigerTuning::GetInstance().GetPose2d("pose", def);
  EXPECT_DOUBLE_EQ(def.X().value(), result.X().value());
  EXPECT_DOUBLE_EQ(def.Y().value(), result.Y().value());
  EXPECT_DOUBLE_EQ(def.Rotation().Degrees().value(), result.Rotation().Degrees().value());
}

TEST_F(TigerTuningTest, GetPose2dLoadsFromJson) {
  auto path = WriteTempJson(R"({"pose": {"x": 3.0, "y": 4.0, "rotation": 90.0}})");
  TigerTuning::Initialize(path);
  auto result = TigerTuning::GetInstance().GetPose2d("pose");
  EXPECT_DOUBLE_EQ(3.0, result.X().value());
  EXPECT_DOUBLE_EQ(4.0, result.Y().value());
  EXPECT_DOUBLE_EQ(90.0, result.Rotation().Degrees().value());
}

TEST_F(TigerTuningTest, GetPose2dMissingFieldFallsBackToDefaultField) {
  // Only "x" is present; "y" and "rotation" should come from the default.
  auto path = WriteTempJson(R"({"pose": {"x": 5.0}})");
  TigerTuning::Initialize(path);
  frc::Pose2d def{0_m, 7_m, 30_deg};
  auto result = TigerTuning::GetInstance().GetPose2d("pose", def);
  EXPECT_DOUBLE_EQ(5.0, result.X().value());
  EXPECT_DOUBLE_EQ(7.0, result.Y().value());
  EXPECT_DOUBLE_EQ(30.0, result.Rotation().Degrees().value());
}

TEST_F(TigerTuningTest, GetPose2dReturnsDefaultForNonObjectJson) {
  auto path = WriteTempJson(R"({"pose": 42})");
  TigerTuning::Initialize(path);
  frc::Pose2d def{1_m, 2_m, 45_deg};
  auto result = TigerTuning::GetInstance().GetPose2d("pose", def);
  EXPECT_DOUBLE_EQ(1.0, result.X().value());
  EXPECT_DOUBLE_EQ(2.0, result.Y().value());
  EXPECT_DOUBLE_EQ(45.0, result.Rotation().Degrees().value());
}

// --- Translation3d ---

TEST_F(TigerTuningTest, GetTranslation3dReturnsDefaultWhenFileNotFound) {
  frc::Translation3d def{1_m, 2_m, 3_m};
  auto result = TigerTuning::GetInstance().GetTranslation3d("t3d", def);
  EXPECT_DOUBLE_EQ(1.0, result.X().value());
  EXPECT_DOUBLE_EQ(2.0, result.Y().value());
  EXPECT_DOUBLE_EQ(3.0, result.Z().value());
}

TEST_F(TigerTuningTest, GetTranslation3dLoadsFromJson) {
  auto path = WriteTempJson(R"({"t3d": {"x": 1.5, "y": 2.5, "z": 3.5}})");
  TigerTuning::Initialize(path);
  auto result = TigerTuning::GetInstance().GetTranslation3d("t3d");
  EXPECT_DOUBLE_EQ(1.5, result.X().value());
  EXPECT_DOUBLE_EQ(2.5, result.Y().value());
  EXPECT_DOUBLE_EQ(3.5, result.Z().value());
}

TEST_F(TigerTuningTest, GetTranslation3dMissingFieldFallsBackToDefaultField) {
  auto path = WriteTempJson(R"({"t3d": {"x": 9.0}})");
  TigerTuning::Initialize(path);
  frc::Translation3d def{0_m, 5_m, 6_m};
  auto result = TigerTuning::GetInstance().GetTranslation3d("t3d", def);
  EXPECT_DOUBLE_EQ(9.0, result.X().value());
  EXPECT_DOUBLE_EQ(5.0, result.Y().value());
  EXPECT_DOUBLE_EQ(6.0, result.Z().value());
}

TEST_F(TigerTuningTest, GetTranslation3dReturnsDefaultForNonObjectJson) {
  auto path = WriteTempJson(R"({"t3d": [1.0, 2.0, 3.0]})");
  TigerTuning::Initialize(path);
  frc::Translation3d def{1_m, 2_m, 3_m};
  auto result = TigerTuning::GetInstance().GetTranslation3d("t3d", def);
  EXPECT_DOUBLE_EQ(1.0, result.X().value());
  EXPECT_DOUBLE_EQ(2.0, result.Y().value());
  EXPECT_DOUBLE_EQ(3.0, result.Z().value());
}

// --- Rotation3d ---

TEST_F(TigerTuningTest, GetRotation3dReturnsDefaultWhenFileNotFound) {
  frc::Rotation3d def{10_deg, 20_deg, 30_deg};
  auto result = TigerTuning::GetInstance().GetRotation3d("rot", def);
  EXPECT_NEAR(10.0, units::degree_t{result.X()}.value(), 1e-9);
  EXPECT_NEAR(20.0, units::degree_t{result.Y()}.value(), 1e-9);
  EXPECT_NEAR(30.0, units::degree_t{result.Z()}.value(), 1e-9);
}

TEST_F(TigerTuningTest, GetRotation3dLoadsFromJson) {
  auto path = WriteTempJson(R"({"rot": {"roll": 10.0, "pitch": 20.0, "yaw": 30.0}})");
  TigerTuning::Initialize(path);
  auto result = TigerTuning::GetInstance().GetRotation3d("rot");
  EXPECT_NEAR(10.0, units::degree_t{result.X()}.value(), 1e-9);
  EXPECT_NEAR(20.0, units::degree_t{result.Y()}.value(), 1e-9);
  EXPECT_NEAR(30.0, units::degree_t{result.Z()}.value(), 1e-9);
}

TEST_F(TigerTuningTest, GetRotation3dMissingFieldFallsBackToDefaultField) {
  auto path = WriteTempJson(R"({"rot": {"yaw": 90.0}})");
  TigerTuning::Initialize(path);
  frc::Rotation3d def{5_deg, 10_deg, 0_deg};
  auto result = TigerTuning::GetInstance().GetRotation3d("rot", def);
  EXPECT_NEAR(5.0, units::degree_t{result.X()}.value(), 1e-9);
  EXPECT_NEAR(10.0, units::degree_t{result.Y()}.value(), 1e-9);
  EXPECT_NEAR(90.0, units::degree_t{result.Z()}.value(), 1e-9);
}

TEST_F(TigerTuningTest, GetRotation3dReturnsDefaultForNonObjectJson) {
  auto path = WriteTempJson(R"({"rot": 45})");
  TigerTuning::Initialize(path);
  frc::Rotation3d def{0_deg, 0_deg, 45_deg};
  auto result = TigerTuning::GetInstance().GetRotation3d("rot", def);
  EXPECT_NEAR(45.0, units::degree_t{result.Z()}.value(), 1e-9);
}

// --- Initialize resets state ---

TEST_F(TigerTuningTest, ReinitializeWithNewFileReflectsNewValues) {
  auto path1 = WriteTempJson(R"({"val": 1.0})");
  TigerTuning::Initialize(path1);
  EXPECT_DOUBLE_EQ(1.0, TigerTuning::GetInstance().GetDouble("val", 0.0));

  auto path2 = (std::filesystem::temp_directory_path() / "tigertuning_test2.json").string();
  {
    std::ofstream f(path2);
    f << R"({"val": 2.0})";
  }
  TigerTuning::Initialize(path2);
  EXPECT_DOUBLE_EQ(2.0, TigerTuning::GetInstance().GetDouble("val", 0.0));
}

}  // namespace
