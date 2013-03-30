#include <string>
#include <vector>
#include <set>
#include <map>
#include <ostream>
#include <istream>
#include <memory>

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

void Dictionary::addDocument(std::istream &is)
{
}

std::set<Record> Dictionary::lookup(int char1, int char2) const
{
    return driver_->lookup(char1, char2);
}

void Dictionary::add(const Record &rec)
{
    driver_->add(rec);
}

std::set<Position>
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

    return std::set<Position>();
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

void SQLiteDriver::add(const Record &rec)
{
}

std::set<Record> SQLiteDriver::lookup(int char1, int char2) const
{
    return std::set<Record>();
}
