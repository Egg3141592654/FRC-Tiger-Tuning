package com.tigertuning.jni;

import org.junit.jupiter.api.Test;

public class TigerTuningJNITest {
  @Test
  void jniLinkTest() {
    // Test to verify that the JNI test link works correctly.
    TigerTuningJNI.initialize();
  }
}
