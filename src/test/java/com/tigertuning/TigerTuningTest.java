package com.tigertuning;

import static org.junit.jupiter.api.Assertions.*;

import edu.wpi.first.hal.HAL;
import edu.wpi.first.math.geometry.Pose2d;
import edu.wpi.first.math.geometry.Rotation2d;
import edu.wpi.first.math.geometry.Rotation3d;
import edu.wpi.first.math.geometry.Translation3d;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

class TigerTuningTest {
  @TempDir static Path tempDir;

  @BeforeAll
  static void initHal() {
    HAL.initialize(500, 0);
  }

  private static String path(String filename, String json) throws IOException {
    Path file = tempDir.resolve(filename);
    Files.writeString(file, json);
    return file.toString();
  }

  // ── getDouble ────────────────────────────────────────────────────────────

  @Test
  void getDouble_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(path("d1.json", "{\"speed\": 1.5}"));
    assertEquals(1.5, TigerTuning.getInstance().getDouble("speed", 0.0));
  }

  @Test
  void getDouble_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("d2.json", "{}"));
    assertEquals(3.14, TigerTuning.getInstance().getDouble("missing", 3.14));
  }

  @Test
  void getDouble_wrongType_returnsDefault() throws IOException {
    TigerTuning.initialize(path("d3.json", "{\"key\": \"notanumber\"}"));
    assertEquals(5.0, TigerTuning.getInstance().getDouble("key", 5.0));
  }

  // ── getInt ───────────────────────────────────────────────────────────────

  @Test
  void getInt_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(path("i1.json", "{\"count\": 42}"));
    assertEquals(42, TigerTuning.getInstance().getInt("count", 0));
  }

  @Test
  void getInt_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("i2.json", "{}"));
    assertEquals(7, TigerTuning.getInstance().getInt("missing", 7));
  }

  @Test
  void getInt_wrongType_returnsDefault() throws IOException {
    TigerTuning.initialize(path("i3.json", "{\"key\": true}"));
    assertEquals(9, TigerTuning.getInstance().getInt("key", 9));
  }

  // ── getBoolean ───────────────────────────────────────────────────────────

  @Test
  void getBoolean_keyPresent_returnsTrue() throws IOException {
    TigerTuning.initialize(path("b1.json", "{\"enabled\": true}"));
    assertTrue(TigerTuning.getInstance().getBoolean("enabled", false));
  }

  @Test
  void getBoolean_keyPresent_returnsFalse() throws IOException {
    TigerTuning.initialize(path("b2.json", "{\"enabled\": false}"));
    assertFalse(TigerTuning.getInstance().getBoolean("enabled", true));
  }

  @Test
  void getBoolean_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("b3.json", "{}"));
    assertTrue(TigerTuning.getInstance().getBoolean("missing", true));
  }

  @Test
  void getBoolean_wrongType_returnsDefault() throws IOException {
    TigerTuning.initialize(path("b4.json", "{\"key\": 1}"));
    assertFalse(TigerTuning.getInstance().getBoolean("key", false));
  }

  // ── getString ────────────────────────────────────────────────────────────

  @Test
  void getString_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(path("s1.json", "{\"name\": \"tiger\"}"));
    assertEquals("tiger", TigerTuning.getInstance().getString("name", ""));
  }

  @Test
  void getString_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("s2.json", "{}"));
    assertEquals("default", TigerTuning.getInstance().getString("missing", "default"));
  }

  @Test
  void getString_wrongType_returnsDefault() throws IOException {
    TigerTuning.initialize(path("s3.json", "{\"key\": 42}"));
    assertEquals("x", TigerTuning.getInstance().getString("key", "x"));
  }

  // ── getDoubleArray ───────────────────────────────────────────────────────

  @Test
  void getDoubleArray_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(path("da1.json", "{\"gains\": [1.0, 2.0, 3.0]}"));
    assertArrayEquals(
        new double[] {1.0, 2.0, 3.0},
        TigerTuning.getInstance().getDoubleArray("gains", new double[0]));
  }

  @Test
  void getDoubleArray_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("da2.json", "{}"));
    double[] def = {9.0, 8.0};
    assertArrayEquals(def, TigerTuning.getInstance().getDoubleArray("missing", def));
  }

  @Test
  void getDoubleArray_nonNumberElementsFiltered() throws IOException {
    TigerTuning.initialize(path("da3.json", "{\"vals\": [1.0, \"skip\", 3.0]}"));
    assertArrayEquals(
        new double[] {1.0, 3.0},
        TigerTuning.getInstance().getDoubleArray("vals", new double[0]));
  }

  @Test
  void getDoubleArray_wrongType_returnsDefault() throws IOException {
    TigerTuning.initialize(path("da4.json", "{\"key\": \"notarray\"}"));
    double[] def = {1.0};
    assertArrayEquals(def, TigerTuning.getInstance().getDoubleArray("key", def));
  }

  // ── getStringArray ───────────────────────────────────────────────────────

  @Test
  void getStringArray_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(path("sa1.json", "{\"names\": [\"a\", \"b\", \"c\"]}"));
    assertArrayEquals(
        new String[] {"a", "b", "c"},
        TigerTuning.getInstance().getStringArray("names", new String[0]));
  }

  @Test
  void getStringArray_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("sa2.json", "{}"));
    String[] def = {"x", "y"};
    assertArrayEquals(def, TigerTuning.getInstance().getStringArray("missing", def));
  }

  @Test
  void getStringArray_nonStringElementsFiltered() throws IOException {
    TigerTuning.initialize(path("sa3.json", "{\"vals\": [\"a\", 99, \"c\"]}"));
    assertArrayEquals(
        new String[] {"a", "c"},
        TigerTuning.getInstance().getStringArray("vals", new String[0]));
  }

  // ── getPose2d ────────────────────────────────────────────────────────────

  @Test
  void getPose2d_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(
        path("p1.json", "{\"pose\": {\"x\": 1.0, \"y\": 2.0, \"rotation\": 90.0}}"));
    Pose2d result = TigerTuning.getInstance().getPose2d("pose", new Pose2d());
    assertEquals(1.0, result.getX(), 1e-9);
    assertEquals(2.0, result.getY(), 1e-9);
    assertEquals(90.0, result.getRotation().getDegrees(), 1e-9);
  }

  @Test
  void getPose2d_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("p2.json", "{}"));
    Pose2d def = new Pose2d(5.0, 6.0, Rotation2d.fromDegrees(45.0));
    Pose2d result = TigerTuning.getInstance().getPose2d("missing", def);
    assertEquals(5.0, result.getX(), 1e-9);
    assertEquals(6.0, result.getY(), 1e-9);
    assertEquals(45.0, result.getRotation().getDegrees(), 1e-9);
  }

  @Test
  void getPose2d_noDefault_keyAbsent_returnsZero() throws IOException {
    TigerTuning.initialize(path("p3.json", "{}"));
    Pose2d result = TigerTuning.getInstance().getPose2d("missing");
    assertEquals(0.0, result.getX(), 1e-9);
    assertEquals(0.0, result.getY(), 1e-9);
    assertEquals(0.0, result.getRotation().getDegrees(), 1e-9);
  }

  @Test
  void getPose2d_partialFields_usesDefaultForMissing() throws IOException {
    TigerTuning.initialize(path("p4.json", "{\"pose\": {\"x\": 3.0}}"));
    Pose2d def = new Pose2d(0.0, 7.0, Rotation2d.fromDegrees(30.0));
    Pose2d result = TigerTuning.getInstance().getPose2d("pose", def);
    assertEquals(3.0, result.getX(), 1e-9);
    assertEquals(7.0, result.getY(), 1e-9);
    assertEquals(30.0, result.getRotation().getDegrees(), 1e-9);
  }

  // ── getTranslation3d ─────────────────────────────────────────────────────

  @Test
  void getTranslation3d_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(
        path("t1.json", "{\"offset\": {\"x\": 1.0, \"y\": 2.0, \"z\": 3.0}}"));
    Translation3d result =
        TigerTuning.getInstance().getTranslation3d("offset", new Translation3d());
    assertEquals(1.0, result.getX(), 1e-9);
    assertEquals(2.0, result.getY(), 1e-9);
    assertEquals(3.0, result.getZ(), 1e-9);
  }

  @Test
  void getTranslation3d_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("t2.json", "{}"));
    Translation3d def = new Translation3d(4.0, 5.0, 6.0);
    Translation3d result = TigerTuning.getInstance().getTranslation3d("missing", def);
    assertEquals(4.0, result.getX(), 1e-9);
    assertEquals(5.0, result.getY(), 1e-9);
    assertEquals(6.0, result.getZ(), 1e-9);
  }

  @Test
  void getTranslation3d_noDefault_keyAbsent_returnsZero() throws IOException {
    TigerTuning.initialize(path("t3.json", "{}"));
    Translation3d result = TigerTuning.getInstance().getTranslation3d("missing");
    assertEquals(0.0, result.getX(), 1e-9);
    assertEquals(0.0, result.getY(), 1e-9);
    assertEquals(0.0, result.getZ(), 1e-9);
  }

  // ── getRotation3d ────────────────────────────────────────────────────────

  @Test
  void getRotation3d_keyPresent_returnsJsonValue() throws IOException {
    TigerTuning.initialize(
        path("r1.json", "{\"rot\": {\"roll\": 10.0, \"pitch\": 20.0, \"yaw\": 30.0}}"));
    Rotation3d result = TigerTuning.getInstance().getRotation3d("rot", new Rotation3d());
    assertEquals(10.0, Math.toDegrees(result.getX()), 1e-9);
    assertEquals(20.0, Math.toDegrees(result.getY()), 1e-9);
    assertEquals(30.0, Math.toDegrees(result.getZ()), 1e-9);
  }

  @Test
  void getRotation3d_keyAbsent_returnsDefault() throws IOException {
    TigerTuning.initialize(path("r2.json", "{}"));
    Rotation3d def =
        new Rotation3d(Math.toRadians(1.0), Math.toRadians(2.0), Math.toRadians(3.0));
    Rotation3d result = TigerTuning.getInstance().getRotation3d("missing", def);
    assertEquals(1.0, Math.toDegrees(result.getX()), 1e-9);
    assertEquals(2.0, Math.toDegrees(result.getY()), 1e-9);
    assertEquals(3.0, Math.toDegrees(result.getZ()), 1e-9);
  }

  @Test
  void getRotation3d_noDefault_keyAbsent_returnsZero() throws IOException {
    TigerTuning.initialize(path("r3.json", "{}"));
    Rotation3d result = TigerTuning.getInstance().getRotation3d("missing");
    assertEquals(0.0, result.getX(), 1e-9);
    assertEquals(0.0, result.getY(), 1e-9);
    assertEquals(0.0, result.getZ(), 1e-9);
  }

  // ── singleton / file loading ──────────────────────────────────────────────

  @Test
  void invalidFilePath_allPrimitiveGettersReturnDefaults() {
    TigerTuning.initialize("/nonexistent/path/file.json");
    TigerTuning tt = TigerTuning.getInstance();
    assertEquals(1.0, tt.getDouble("k", 1.0));
    assertEquals(2, tt.getInt("k", 2));
    assertTrue(tt.getBoolean("k", true));
    assertEquals("x", tt.getString("k", "x"));
    assertArrayEquals(new double[] {1.0}, tt.getDoubleArray("k", new double[] {1.0}));
    assertArrayEquals(new String[] {"z"}, tt.getStringArray("k", new String[] {"z"}));
  }

  @Test
  void initialize_replacesExistingInstance() throws IOException {
    TigerTuning.initialize(path("swap1.json", "{\"val\": 1.0}"));
    assertEquals(1.0, TigerTuning.getInstance().getDouble("val", 0.0));

    TigerTuning.initialize(path("swap2.json", "{\"val\": 2.0}"));
    assertEquals(2.0, TigerTuning.getInstance().getDouble("val", 0.0));
  }

  @Test
  void getInstance_returnsSameInstance() throws IOException {
    TigerTuning.initialize(path("same.json", "{}"));
    assertSame(TigerTuning.getInstance(), TigerTuning.getInstance());
  }
}
