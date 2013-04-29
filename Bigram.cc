#include <string>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <ostream>
#include <istream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <utility>
#include <list>
#include <iterator>

#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "utf8/source/utf8.h"
#include "Bigram.hh"

using namespace Bigram;

Dictionary::Dictionary(std::shared_ptr<Driver> drv)
     : driver_(drv)
{
}

Dictionary::Dictionary()
    : driver_(new MemoryDriver)
{
}

std::set<Record> Dictionary::lookup(int char1, int char2) const
{
    return driver_->lookup(char1, char2);
}

void Dictionary::add(const std::string &fileid, std::istream &is)
{
    std::string line;

    int offset = 0;
    while (std::getline(is, line)) {
	add(fileid, line, offset);
	offset += line.length();
    }
}

void Dictionary::add(const std::string &fileid, const std::string &text, size_t offset)
{
    auto chars = disassemble(text);
    int count = 0;
    for (auto it = chars.cbegin();
         it != chars.cend() && (it + 1) != chars.cend();
         it ++, count ++) {
        Bigram::Record rec((*it).first, (*(it + 1)).first,
                           Bigram::Position(fileid, count + offset));
        add(rec);
    }
}

void Dictionary::add(const Path &filepath)
{
    std::string hash = Bigram::digest_file(filepath);
    std::ifstream is(filepath);
    add(hash, is);

    register_path(filepath, hash);
}

void Dictionary::add(const Record &rec)
{
    driver_->add(rec);
}

std::list<Position>
Dictionary::search(const std::string &text) const
{
    std::map<Position, int> map;

    auto chars = disassemble(text);
    for (int i = 0; i < chars.size() - 1; i ++) {
        auto recs = lookup(chars[i].first, chars[i + 1].first);
        for (auto rec : recs) {
	    auto offset = rec.position().position() - chars[i].second;
	    Position pos(rec.position().docid(), offset);
            map[pos]++;
        }
    }

    std::list<std::pair<Position, int>> result(map.cbegin(), map.cend());
    result.remove_if([&chars](std::pair<Position, int> elem){
	    return elem.second < chars.size() - 1;
	});
    std::list<Position> dest;
    std::transform(result.cbegin(), result.cend(), std::back_inserter(dest),
		   [](const std::pair<Position, int> &a){
		       return a.first;
		   });
    return dest;
}

void Dictionary::register_path(const Path &path, const std::string &digest)
{
    driver_->register_path(path, digest);
}

std::set<Path> Dictionary::lookup_digest(const std::string &digest)
{
    return driver_->lookup_digest(digest);
}

Record::Record(int char1, int char2, const Position &pos)
    : first_(char1), second_(char2), position_(pos)
{
}

bool Position::operator==(const Position &pos) const
{
    return this == &pos || pos.docid_ == docid_ && pos.position_ == position_;
}

bool Position::operator<(const Position &pos) const
{
    if (this == &pos) return false;
    if (pos.docid_ < docid_) return true;
    if (pos.docid_ > docid_) return false;
    if (pos.position_ < position_) return true;
    return false;
}

bool Record::operator==(const Record &rec) const
{
    return this == &rec
        || rec.first() == first_ && rec.second() == second_ && rec.position() == position_;
}

bool Record::operator<(const Record &rec) const
{
    if (this == &rec) return false;
    if (rec.first() < first_) return true;
    if (rec.first() > first_) return false;
    if (rec.second() < second_) return true;
    if (rec.second() > second_) return false;
    if (rec.position() < position_) return true;
    return false;
}

std::ostream& operator<<(std::ostream &os, const Position& pos)
{
    os << "Bigram::Position(\"" << pos.docid() << "\", " << pos.position() << ")";
    return os;
}

std::ostream& operator<<(std::ostream &os, const Record& rec)
{
    os << "Bigram::Record(" << rec.first() << ", " << rec.second()
       << ", " << rec.position() << ")";
    return os;
}

std::vector<std::pair<CodePoint, size_t>> Bigram::disassemble(const std::string &text)
{
    auto it = text.cbegin();
    auto end = text.cend();
    std::vector<std::pair<CodePoint, size_t>> dest;
    while (it != end) {
        auto offset = it - text.cbegin();
        int cp = utf8::next(it, end);
        dest.push_back(std::pair<CodePoint, size_t>(CodePoint(cp), offset));
    }
    return dest;
}

void MemoryDriver::add(const Record &rec)
{
    records_.insert(rec);
}

std::set<Record> MemoryDriver::lookup(int char1, int char2) const
{
    std::set<Record> dest;
    for (auto &rec : records_) {
        if (rec.first() == char1 && rec.second() == char2)
            dest.insert(rec);
    }
    return dest;
}

void MemoryDriver::register_path(const Path &path, const std::string &digest)
{
    path_digest_map_[digest].insert(path);
}

std::set<Path> MemoryDriver::lookup_digest(const std::string &digest)
{
    return path_digest_map_[digest];
}

SQLiteDriver::SQLiteDriver(const std::string &filename)
    : db_(nullptr), insert_statement_(nullptr)
{
    int rc = sqlite3_open(filename.c_str(), &db_);
    if (rc) {
	sqlite3_close(db_);
	throw new std::bad_alloc();
    }

    std::ostringstream oss;
    oss << "CREATE TABLE IF NOT EXISTS dictionary ("
	<< "first INTEGER, "
	<< "second INTEGER, "
	<< "docid VARCHAR(20), "
	<< "position INTEGER, "
	<< "PRIMARY KEY(first, second, docid, position)"
	<< ");";
    char *zErrMsg;
    rc = sqlite3_exec(db_, oss.str().c_str(), nullptr, nullptr, &zErrMsg);

    if(rc!=SQLITE_OK){
	std::string err(zErrMsg);
	sqlite3_free(zErrMsg);
	throw err;
    }

    try {
	prepare_insert_statement();
    } catch (int) {
	throw std::bad_alloc();
    }

    {
	std::ostringstream oss;
	oss << "CREATE TABLE IF NOT EXISTS path_map ("
	    << "path VARCHAR(255), "
	    << "docid VARCHAR(20)"
	    << ");";
	char *zErrMsg;
	rc = sqlite3_exec(db_, oss.str().c_str(), nullptr, nullptr, &zErrMsg);

	if(rc!=SQLITE_OK){
	    std::string err(zErrMsg);
	    sqlite3_free(zErrMsg);
	    throw err;
	}

    }
}

void SQLiteDriver::prepare_insert_statement()
{
    std::ostringstream oss;
    oss << "INSERT INTO dictionary (first, second, docid, position) VALUES "
	<< "(?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(db_, oss.str().c_str(), oss.str().length(),
				&insert_statement_, nullptr);
    if (rc != SQLITE_OK) {
	throw rc;
    }
}

void SQLiteDriver::add(const Record &rec)
{
    if (sqlite3_bind_int(insert_statement_, 1, rec.first())) throw;
    if (sqlite3_bind_int(insert_statement_, 2, rec.second())) throw;
    if (sqlite3_bind_text(insert_statement_, 3, rec.position().docid().c_str(),
			  rec.position().docid().length(), nullptr)) throw;
    if (sqlite3_bind_int(insert_statement_, 4, rec.position().position())) throw;

    int rc = sqlite3_step(insert_statement_);

    if (rc != SQLITE_DONE) {
	std::cout << " error code: " << rc << std::endl;
	throw rc;
    }

    if (sqlite3_reset(insert_statement_)) throw;
}

std::set<Record> SQLiteDriver::lookup(int char1, int char2) const
{
    std::ostringstream oss;
    oss << "SELECT first, second, docid, position FROM dictionary "
	<< "WHERE first=" << char1 << " AND second=" << char2;

    std::set<Record> dest;

    struct X {
	X(std::set<Record> &dest, int c1, int c2) : dest_(dest), c1_(c1), c2_(c2) {
	}
	int operator()(int argc, char** argv, char** columns) {
	    for (int i = 0; i < argc; i ++) {
		std::string fileid;
		std::istringstream ost(argv[2]);
		ost >> fileid;
		std::istringstream ost2(argv[3]);
		int pos;
		ost2 >> pos;
		dest_.insert(Record(c1_, c2_, Bigram::Position(fileid, pos)));
	    }
	    return 0;
	}
	int c1_, c2_;
	std::set<Record> &dest_;
	static int doit(void* self, int argc, char** argv, char** columns) {
	    return (*((X*)self))(argc, argv, columns);
	}
    } callback(dest, char1, char2);

    char *zErrMsg;
    int rc = sqlite3_exec(
	db_, oss.str().c_str(),
	X::doit,
	&callback, &zErrMsg);

    if(rc!=SQLITE_OK){
	std::string err(zErrMsg);
	sqlite3_free(zErrMsg);
	throw err;
    }

    return dest;
}

void SQLiteDriver::register_path(const Path &path, const std::string &digest)
{
    std::ostringstream oss;
    oss << "INSERT INTO path_map (path, docid) "
	<< "VALUES (\"" << std::string(path) << "\", \"" << digest << "\")";

    char *zErrMsg;
    int rc = sqlite3_exec(db_, oss.str().c_str(), nullptr, nullptr, &zErrMsg);

    if(rc!=SQLITE_OK){
	std::string err(zErrMsg);
	sqlite3_free(zErrMsg);
	throw err;
    }
}

std::set<Path> SQLiteDriver::lookup_digest(const std::string &digest)
{

    std::ostringstream oss;
    oss << "SELECT path FROM path_map "
	<< "WHERE docid=\"" << digest << "\"";

    std::set<Path> dest;

    struct X {
	X(std::set<Path> &dest) : dest_(dest) {
	}
	int operator()(int argc, char** argv, char** columns) {
	    for (int i = 0; i < argc; i ++) {
		std::string path;
		std::istringstream ost(argv[0]);
		ost >> path;
		dest_.insert(Path(path));
	    }
	    return 0;
	}
	int c1_, c2_;
	std::set<Path> &dest_;
	static int doit(void* self, int argc, char** argv, char** columns) {
	    return (*((X*)self))(argc, argv, columns);
	}
    } callback(dest);

    char *zErrMsg;
    int rc = sqlite3_exec(
	db_, oss.str().c_str(),
	X::doit,
	&callback, &zErrMsg);

    if(rc!=SQLITE_OK){
	std::string err(zErrMsg);
	sqlite3_free(zErrMsg);
	throw err;
    }

    return dest;
}

std::string Bigram::digest_file(const std::string &path)
{
    std::ifstream is(path);
    SHA_CTX c;
    SHA1_Init(&c);

    std::string line;
    while (std::getline(is, line)) {
	SHA1_Update(&c, line.c_str(), line.length());
    }

    unsigned char md[SHA_DIGEST_LENGTH];
    SHA1_Final(md, &c);

    return std::string((const char*)md, SHA_DIGEST_LENGTH);

    // std::ostringstream ost;
    // ost.fill('0');
    // ost.width(2);
    // ost << std::hex;
    // for (auto p = md; p < md + SHA_DIGEST_LENGTH; p ++) {
    // 	ost << (int)*p;
    // }

    // return ost.str();
}
