package com.tigertuning;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import edu.wpi.first.math.geometry.Pose2d;
import edu.wpi.first.math.geometry.Rotation2d;
import edu.wpi.first.math.geometry.Rotation3d;
import edu.wpi.first.math.geometry.Translation3d;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Singleton configuration cache backed by a JSON file. Direct Java port of {@code TigerTuning}
 * (native/cpp); both share the same JSON schema and SmartDashboard keys.
 *
 * <p>Values are read from the JSON file at startup. When the SmartDashboard key {@link
 * #kOverrideKey} is {@code true} (and no FMS is attached), getters first check SmartDashboard
 * (seeding it with JSON values on first enable), then fall back to the JSON cache, then to the
 * caller-supplied default.
 *
 * <p>Call {@link #initialize(String)} before the first {@link #getInstance()} if you need a
 * non-default file path.
 *
 * <p>JSON schema for geometry types (distances in metres, angles in degrees):
 *
 * <ul>
 *   <li>Pose2d: {@code {"x": 0.0, "y": 0.0, "rotation": 0.0}}
 *   <li>Translation3d: {@code {"x": 0.0, "y": 0.0, "z": 0.0}}
 *   <li>Rotation3d: {@code {"roll": 0.0, "pitch": 0.0, "yaw": 0.0}}
 * </ul>
 */
public class TigerTuning {
  public static final String kOverrideKey = "TigerTuning/OverrideEnabled";
  public static final String kDefaultFilePath = "/home/lvuser/deploy/tigertuning.json";

  private static final ObjectMapper kMapper = new ObjectMapper();

  private static TigerTuning s_instance;

  private final JsonNode m_cache;
  private boolean m_prevOverride = false;

  /** Returns the singleton, creating it with the default file path if necessary. */
  public static TigerTuning getInstance() {
    if (s_instance == null) {
      s_instance = new TigerTuning(kDefaultFilePath);
    }
    return s_instance;
  }

  /**
   * Resets the singleton with the given file path. Call before the first {@link #getInstance()}.
   */
  public static void initialize(String filePath) {
    s_instance = new TigerTuning(filePath);
  }

  private TigerTuning(String filePath) {
    JsonNode parsed = kMapper.createObjectNode();
    try {
      JsonNode node = kMapper.readTree(new File(filePath));
      if (node != null && node.isObject()) {
        parsed = node;
      }
    } catch (IOException ignored) {
    }
    m_cache = parsed;
  }

  /** Returns true if the SmartDashboard override is active, seeding it on the first transition. */
  private boolean checkOverride() {
    if (DriverStation.isFMSAttached()) {
      return false;
    }
    boolean override = SmartDashboard.getBoolean(kOverrideKey, false);
    if (override && !m_prevOverride) {
      seedSmartDashboard();
    }
    m_prevOverride = override;
    return override;
  }

  private void seedSmartDashboard() {
    for (Map.Entry<String, JsonNode> entry : m_cache.properties()) {
      String key = entry.getKey();
      JsonNode value = entry.getValue();
      if (value.isBoolean()) {
        SmartDashboard.putBoolean(key, value.asBoolean());
      } else if (value.isNumber()) {
        SmartDashboard.putNumber(key, value.asDouble());
      } else if (value.isTextual()) {
        SmartDashboard.putString(key, value.asText());
      } else if (value.isObject()) {
        for (Map.Entry<String, JsonNode> sub : value.properties()) {
          if (sub.getValue().isNumber()) {
            SmartDashboard.putNumber(key + "/" + sub.getKey(), sub.getValue().asDouble());
          }
        }
      } else if (value.isArray() && value.size() > 0) {
        JsonNode first = value.get(0);
        if (first.isNumber()) {
          double[] arr = new double[value.size()];
          for (int i = 0; i < value.size(); i++) {
            arr[i] = value.get(i).asDouble();
          }
          SmartDashboard.putNumberArray(key, arr);
        } else if (first.isTextual()) {
          String[] arr = new String[value.size()];
          for (int i = 0; i < value.size(); i++) {
            arr[i] = value.get(i).asText();
          }
          SmartDashboard.putStringArray(key, arr);
        }
      }
    }
  }

  /**
   * Returns the value stored under {@code key} as a {@code double}.
   *
   * <p>Returns {@code defaultValue} when the key is absent or not a number. When the SmartDashboard
   * override is active the SmartDashboard entry is returned instead, seeded from the JSON value on
   * first enable.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or {@code defaultValue}
   */
  public double getDouble(String key, double defaultValue) {
    JsonNode node = m_cache.get(key);
    double jsonValue = (node != null && node.isNumber()) ? node.asDouble() : defaultValue;
    if (checkOverride()) {
      return SmartDashboard.getNumber(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as an {@code int}.
   *
   * <p>Returns {@code defaultValue} when the key is absent or not a number. When the SmartDashboard
   * override is active the SmartDashboard entry (read as a {@code double} and truncated) is returned
   * instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or {@code defaultValue}
   */
  public int getInt(String key, int defaultValue) {
    JsonNode node = m_cache.get(key);
    int jsonValue = (node != null && node.isNumber()) ? node.asInt() : defaultValue;
    if (checkOverride()) {
      return (int) SmartDashboard.getNumber(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@code boolean}.
   *
   * <p>Returns {@code defaultValue} when the key is absent or not a boolean. When the SmartDashboard
   * override is active the SmartDashboard entry is returned instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or {@code defaultValue}
   */
  public boolean getBoolean(String key, boolean defaultValue) {
    JsonNode node = m_cache.get(key);
    boolean jsonValue = (node != null && node.isBoolean()) ? node.asBoolean() : defaultValue;
    if (checkOverride()) {
      return SmartDashboard.getBoolean(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@code String}.
   *
   * <p>Returns {@code defaultValue} when the key is absent or not a string. When the SmartDashboard
   * override is active the SmartDashboard entry is returned instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured value, or {@code defaultValue}
   */
  public String getString(String key, String defaultValue) {
    JsonNode node = m_cache.get(key);
    String jsonValue = (node != null && node.isTextual()) ? node.asText() : defaultValue;
    if (checkOverride()) {
      return SmartDashboard.getString(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@code double[]}.
   *
   * <p>Non-numeric elements within the JSON array are silently skipped. Returns {@code
   * defaultValue} when the key is absent or not an array. When the SmartDashboard override is
   * active the SmartDashboard entry is returned instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured array, or {@code defaultValue}
   */
  public double[] getDoubleArray(String key, double[] defaultValue) {
    JsonNode node = m_cache.get(key);
    double[] jsonValue = defaultValue;
    if (node != null && node.isArray()) {
      List<Double> list = new ArrayList<>();
      for (JsonNode v : node) {
        if (v.isNumber()) {
          list.add(v.asDouble());
        }
      }
      jsonValue = list.stream().mapToDouble(Double::doubleValue).toArray();
    }
    if (checkOverride()) {
      return SmartDashboard.getNumberArray(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@code String[]}.
   *
   * <p>Non-string elements within the JSON array are silently skipped. Returns {@code defaultValue}
   * when the key is absent or not an array. When the SmartDashboard override is active the
   * SmartDashboard entry is returned instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured array, or {@code defaultValue}
   */
  public String[] getStringArray(String key, String[] defaultValue) {
    JsonNode node = m_cache.get(key);
    String[] jsonValue = defaultValue;
    if (node != null && node.isArray()) {
      List<String> list = new ArrayList<>();
      for (JsonNode v : node) {
        if (v.isTextual()) {
          list.add(v.asText());
        }
      }
      jsonValue = list.toArray(new String[0]);
    }
    if (checkOverride()) {
      return SmartDashboard.getStringArray(key, jsonValue);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@link Pose2d}.
   *
   * <p>Expects a JSON object with numeric fields {@code x} (metres), {@code y} (metres), and
   * {@code rotation} (degrees). Any absent field falls back to the corresponding field of {@code
   * defaultValue}. When the SmartDashboard override is active, sub-keys {@code key/x},
   * {@code key/y}, and {@code key/rotation} are read instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured pose, or {@code defaultValue}
   */
  public Pose2d getPose2d(String key, Pose2d defaultValue) {
    JsonNode node = m_cache.get(key);
    Pose2d jsonValue = defaultValue;
    if (node != null && node.isObject()) {
      double x = objectField(node, "x", defaultValue.getX());
      double y = objectField(node, "y", defaultValue.getY());
      double rot = objectField(node, "rotation", defaultValue.getRotation().getDegrees());
      jsonValue = new Pose2d(x, y, Rotation2d.fromDegrees(rot));
    }
    if (checkOverride()) {
      double x = SmartDashboard.getNumber(key + "/x", jsonValue.getX());
      double y = SmartDashboard.getNumber(key + "/y", jsonValue.getY());
      double rot =
          SmartDashboard.getNumber(key + "/rotation", jsonValue.getRotation().getDegrees());
      return new Pose2d(x, y, Rotation2d.fromDegrees(rot));
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@link Pose2d}, defaulting to the origin.
   *
   * @param key configuration key
   * @return the configured pose, or an identity {@link Pose2d} if the key is absent
   * @see #getPose2d(String, Pose2d)
   */
  public Pose2d getPose2d(String key) {
    return getPose2d(key, new Pose2d());
  }

  /**
   * Returns the value stored under {@code key} as a {@link Translation3d}.
   *
   * <p>Expects a JSON object with numeric fields {@code x}, {@code y}, and {@code z} (all metres).
   * Any absent field falls back to the corresponding field of {@code defaultValue}. When the
   * SmartDashboard override is active, sub-keys {@code key/x}, {@code key/y}, and {@code key/z}
   * are read instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured translation, or {@code defaultValue}
   */
  public Translation3d getTranslation3d(String key, Translation3d defaultValue) {
    JsonNode node = m_cache.get(key);
    Translation3d jsonValue = defaultValue;
    if (node != null && node.isObject()) {
      double x = objectField(node, "x", defaultValue.getX());
      double y = objectField(node, "y", defaultValue.getY());
      double z = objectField(node, "z", defaultValue.getZ());
      jsonValue = new Translation3d(x, y, z);
    }
    if (checkOverride()) {
      double x = SmartDashboard.getNumber(key + "/x", jsonValue.getX());
      double y = SmartDashboard.getNumber(key + "/y", jsonValue.getY());
      double z = SmartDashboard.getNumber(key + "/z", jsonValue.getZ());
      return new Translation3d(x, y, z);
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@link Translation3d}, defaulting to the
   * origin.
   *
   * @param key configuration key
   * @return the configured translation, or a zero {@link Translation3d} if the key is absent
   * @see #getTranslation3d(String, Translation3d)
   */
  public Translation3d getTranslation3d(String key) {
    return getTranslation3d(key, new Translation3d());
  }

  /**
   * Returns the value stored under {@code key} as a {@link Rotation3d}.
   *
   * <p>Expects a JSON object with numeric fields {@code roll}, {@code pitch}, and {@code yaw} (all
   * degrees). Any absent field falls back to the corresponding field of {@code defaultValue}. When
   * the SmartDashboard override is active, sub-keys {@code key/roll}, {@code key/pitch}, and
   * {@code key/yaw} are read instead.
   *
   * @param key configuration key
   * @param defaultValue fallback when the key is absent or the wrong type
   * @return the configured rotation, or {@code defaultValue}
   */
  public Rotation3d getRotation3d(String key, Rotation3d defaultValue) {
    JsonNode node = m_cache.get(key);
    Rotation3d jsonValue = defaultValue;
    if (node != null && node.isObject()) {
      double roll = objectField(node, "roll", Math.toDegrees(defaultValue.getX()));
      double pitch = objectField(node, "pitch", Math.toDegrees(defaultValue.getY()));
      double yaw = objectField(node, "yaw", Math.toDegrees(defaultValue.getZ()));
      jsonValue = new Rotation3d(Math.toRadians(roll), Math.toRadians(pitch), Math.toRadians(yaw));
    }
    if (checkOverride()) {
      double roll = SmartDashboard.getNumber(key + "/roll", Math.toDegrees(jsonValue.getX()));
      double pitch = SmartDashboard.getNumber(key + "/pitch", Math.toDegrees(jsonValue.getY()));
      double yaw = SmartDashboard.getNumber(key + "/yaw", Math.toDegrees(jsonValue.getZ()));
      return new Rotation3d(Math.toRadians(roll), Math.toRadians(pitch), Math.toRadians(yaw));
    }
    return jsonValue;
  }

  /**
   * Returns the value stored under {@code key} as a {@link Rotation3d}, defaulting to no rotation.
   *
   * @param key configuration key
   * @return the configured rotation, or a zero {@link Rotation3d} if the key is absent
   * @see #getRotation3d(String, Rotation3d)
   */
  public Rotation3d getRotation3d(String key) {
    return getRotation3d(key, new Rotation3d());
  }

  /** Returns the numeric field value from a JSON object node, or {@code fallback} if absent. */
  private static double objectField(JsonNode object, String field, double fallback) {
    JsonNode node = object.get(field);
    return (node != null && node.isNumber()) ? node.asDouble() : fallback;
  }
}
