#include <memory>
#include <list>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <sys/time.h>

#include <cppunit/extensions/HelperMacros.h>
#include <sqlite3.h>
#include "Bigram.hh"

class BigramTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(BigramTest);
    CPPUNIT_TEST(test_disassemble);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_add_text);
    CPPUNIT_TEST(test_search);
    CPPUNIT_TEST(test_digest_file);
    CPPUNIT_TEST(test_add_document);
    CPPUNIT_TEST(test_path_digest_map);

    CPPUNIT_TEST(test_sqlite_lookup);
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
    void test_digest_file();
    void test_add_document();
    void test_path_digest_map();
    void test_sqlite_lookup();
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
    CPPUNIT_ASSERT_EQUAL(int('h'), int(chars[0].first));
    CPPUNIT_ASSERT_EQUAL(int('o'), int(chars[1].first));
    CPPUNIT_ASSERT_EQUAL(int('g'), int(chars[2].first));
    CPPUNIT_ASSERT_EQUAL(int('e'), int(chars[3].first));

    CPPUNIT_ASSERT_EQUAL(size_t(0), chars[0].second);
    CPPUNIT_ASSERT_EQUAL(size_t(1), chars[1].second);

    std::string text_k = "漢字カタカナ";

    auto chars_k = Bigram::disassemble(text_k);

    CPPUNIT_ASSERT_EQUAL(6, int(chars_k.size()));
    CPPUNIT_ASSERT_EQUAL(28450, int(chars_k[0].first));
    CPPUNIT_ASSERT_EQUAL(23383, int(chars_k[1].first));
    CPPUNIT_ASSERT_EQUAL(size_t(3), chars_k[1].second);
}

void BigramTest::test_add_text() {
    dict_->add(fileid_, text_, 0);
    std::set<Bigram::Record> result = dict_->lookup('l', 'v');
    CPPUNIT_ASSERT_EQUAL(1, int(result.size()));
    CPPUNIT_ASSERT_EQUAL(7, int((*(result.cbegin())).position().position()));
}

void BigramTest::test_search() {
    dict_->add(fileid_, text_, 0);

    std::string word = "land";
    auto result = dict_->search(word);
    CPPUNIT_ASSERT_EQUAL(1, int(result.size()));

    int pos = text_.find(word);
    CPPUNIT_ASSERT_EQUAL(*(result.cbegin()), Bigram::Position(fileid_, pos));
}

void BigramTest::test_digest_file() {
    std::string hash = Bigram::digest_file("test/lipsum.txt");
    CPPUNIT_ASSERT_EQUAL(std::string("b1f3a9369533e35392b361ba5ecfa3919814d114"), hash);
}

void BigramTest::test_add_document() {
    dict_->add(Bigram::Path("test/lipsum.txt"));

    auto result = dict_->search("ultrices");
    CPPUNIT_ASSERT_EQUAL(size_t(4), result.size());

    auto paths = dict_->lookup_digest("b1f3a9369533e35392b361ba5ecfa3919814d114");
    CPPUNIT_ASSERT_EQUAL(1, int(paths.size()));
    CPPUNIT_ASSERT(paths.find(Bigram::Path("test/lipsum.txt")) != paths.end());
}

void BigramTest::test_path_digest_map() {
    dict_->register_path(Bigram::Path("test/lipsum.txt"),
			 "b1f3a9369533e35392b361ba5ecfa3919814d114");
    dict_->register_path(Bigram::Path("test/another.txt"),
			 "b1f3a9369533e35392b361ba5ecfa3919814d114");

    auto paths = dict_->lookup_digest("b1f3a9369533e35392b361ba5ecfa3919814d114");
    CPPUNIT_ASSERT_EQUAL(2, int(paths.size()));
    CPPUNIT_ASSERT(paths.find(Bigram::Path("test/lipsum.txt")) != paths.end());
    CPPUNIT_ASSERT(paths.find(Bigram::Path("test/another.txt")) != paths.end());
}

void BigramTest::test_sqlite_lookup() {
    remove("test2.sqlite");

    std::shared_ptr<Bigram::Driver> drv(new Bigram::SQLiteDriver("test2.sqlite"));
    Bigram::Dictionary dict(drv);

    unsigned int position = 12345;
    Bigram::Record rec('h', 'o', Bigram::Position("", position));
    dict.add(rec);

    std::set<Bigram::Record> result = dict.lookup('h', 'o');
    CPPUNIT_ASSERT(result.size() > 0);
    CPPUNIT_ASSERT_EQUAL(*(result.cbegin()), rec);
    CPPUNIT_ASSERT(result.find(rec) != result.end());
}

void BigramTest::test_sqlite() {
    remove("test.sqlite");

    std::shared_ptr<Bigram::Driver> drv(new Bigram::SQLiteDriver("test.sqlite"));
    Bigram::Dictionary dict(drv);

    std::ifstream is("test/lipsum.txt");

    struct timeval start, end;
    gettimeofday(&start, NULL);

    try {
	dict.add(fileid_, is);
    } catch(const std::string &str) {
	CPPUNIT_FAIL(str.c_str());
    }

    gettimeofday(&end, NULL);
    auto ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    std::cout << " " << ms << " ms ";

    auto result = dict.search("ultrices");
    CPPUNIT_ASSERT_EQUAL(size_t(4), result.size());
}

// Local Variables:
// coding: utf-8
// End:
