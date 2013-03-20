#include <memory>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>
#include "Bigram.hh"

class BigramTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(BigramTest);
    CPPUNIT_TEST(test_disassemble);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_add_text);
    CPPUNIT_TEST_SUITE_END();

protected:
    std::shared_ptr<Bigram::Dictionary> dict_;

public:
    void setUp();
    void tearDown();

protected:
    void test_disassemble();
    void test_add();
    void test_add_text();
};

CPPUNIT_TEST_SUITE_REGISTRATION( BigramTest );

void BigramTest::setUp() {
    dict_.reset(new Bigram::Dictionary());
}

void BigramTest::tearDown() {
    dict_.reset();
}

void BigramTest::test_add() {
    // fileid may be a hash value of the input file
    std::string fileid = "xfile";
    // position in byte
    unsigned int position = 12345;
    Bigram::Record rec('h', 'o', fileid, position);
    dict_->add(rec);

    std::set<Bigram::Record> result = dict_->lookup('h', 'o');
    CPPUNIT_ASSERT(result.find(rec) != result.end());
}

void BigramTest::test_disassemble() {
    std::string text = "hogefuga";

    std::vector<int> chars = Bigram::disassemble(text);
    // std::cout << chars.size() << std::endl;

    CPPUNIT_ASSERT(chars.size() == 8);

    // CPPUNIT_ASSERT_EQUAL(chars[0], int('h'));
    // CPPUNIT_ASSERT_EQUAL(chars[1], int('o'));
    // CPPUNIT_ASSERT_EQUAL(chars[2], int('g'));
    // CPPUNIT_ASSERT_EQUAL(chars[3], int('e'));
}

void BigramTest::test_add_text() {
    std::string text = "Cras pulvinar sollicitudin purus sed viverra. "
        "Nunc facilisis odio in mi blandit vel vulputate elit semper.";

    std::vector<int> chars = Bigram::disassemble(text);


}

// Local Variables:
// coding: utf-8
// End:
