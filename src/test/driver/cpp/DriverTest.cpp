#include "driverheader.h"
#include "gtest/gtest.h"

TEST(DriverTest, DoThingDoesNotCrash) { EXPECT_NO_FATAL_FAILURE(c_doThing()); }
