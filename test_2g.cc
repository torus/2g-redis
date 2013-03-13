#include <cppunit/extensions/HelperMacros.h>

class BigramTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE( BigramTest );
    CPPUNIT_TEST( test_init );
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp();
    void tearDown();

protected:
    void test_init();
};

CPPUNIT_TEST_SUITE_REGISTRATION( BigramTest );

void BigramTest::setUp() {
}

void BigramTest::tearDown() {
}

void BigramTest::test_init() {
    CPPUNIT_ASSERT_EQUAL(0, 0);
}
