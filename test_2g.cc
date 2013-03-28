#include <memory>
#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>
#include "Bigram.hh"

class BigramTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(BigramTest);
    CPPUNIT_TEST(test_disassemble);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_add_text);
    CPPUNIT_TEST(test_search);

    CPPUNIT_TEST(test_sqlite);

    CPPUNIT_TEST_SUITE_END();

protected:
    std::shared_ptr<Bigram::Dictionary> dict_;
    std::string text_;
    std::string fileid_;

public:
    void setUp();
    void tearDown();

protected:
    void test_disassemble();
    void test_add();
    void test_add_text();
    void test_search();
    void test_sqlite();
};

CPPUNIT_TEST_SUITE_REGISTRATION( BigramTest );

void BigramTest::setUp() {
    dict_.reset(new Bigram::Dictionary());

    text_ = "Cras pulvinar sollicitudin purus sed viverra. "
        "Nunc facilisis odio in mi blandit vel vulputate elit semper.";
    fileid_ = "xxxxxxxxxx";
}

void BigramTest::tearDown() {
    dict_.reset();
}

void BigramTest::test_add() {
    // fileid may be a hash value of the input file
    std::string fileid = "xfile";
    // position in byte
    unsigned int position = 12345;
    Bigram::Record rec('h', 'o', Bigram::Position(fileid, position));
    dict_->add(rec);

    std::set<Bigram::Record> result = dict_->lookup('h', 'o');
    CPPUNIT_ASSERT(result.size() > 0);
    CPPUNIT_ASSERT_EQUAL(*(result.cbegin()), rec);
    CPPUNIT_ASSERT(result.find(rec) != result.end());
}

void BigramTest::test_disassemble() {
    std::string text = "hogefuga";

    auto chars = Bigram::disassemble(text);

    CPPUNIT_ASSERT_EQUAL(8, int(chars.size()));
    CPPUNIT_ASSERT_EQUAL(int(chars[0].first), int('h'));
    CPPUNIT_ASSERT_EQUAL(int(chars[1].first), int('o'));
    CPPUNIT_ASSERT_EQUAL(int(chars[2].first), int('g'));
    CPPUNIT_ASSERT_EQUAL(int(chars[3].first), int('e'));

    std::string text_k = "漢字カタカナ";

    auto chars_k = Bigram::disassemble(text_k);

    CPPUNIT_ASSERT_EQUAL(6, int(chars_k.size()));
    CPPUNIT_ASSERT_EQUAL(int(chars_k[0].first), 28450);
    CPPUNIT_ASSERT_EQUAL(int(chars_k[1].first), 23383);
}

void BigramTest::test_add_text() {
    auto chars = Bigram::disassemble(text_);
    int count = 0;
    for (auto it = chars.cbegin();
         it != chars.cend() && (it + 1) != chars.cend();
         it ++, count ++) {
        Bigram::Record rec((*it).first, (*(it + 1)).first,
                           Bigram::Position(fileid_, count));
        dict_->add(rec);
    }
}

void BigramTest::test_search() {
    std::string word = "land";
    auto result = dict_->search(word);
    CPPUNIT_ASSERT_EQUAL(int(result.size()), 1);

    int pos = text_.find(word);
    CPPUNIT_ASSERT_EQUAL(*(result.cbegin()), Bigram::Position(fileid_, pos));
    CPPUNIT_ASSERT(*(result.cbegin()) == Bigram::Position(fileid_, pos));
}

void BigramTest::test_sqlite() {
    std::shared_ptr<Bigram::Driver> drv(new Bigram::SQLiteDriver("test.sqlite"));
    Bigram::Dictionary dict(drv);

    std::ifstream is("test/lipsum.txt");

    dict.addDocument(is);
}

// Local Variables:
// coding: utf-8
// End:
