#include <string>
#include <vector>
#include <set>
#include <ostream>
#include <istream>
#include <memory>

#include "Bigram.hh"

using namespace Bigram;

Dictionary::Dictionary(std::shared_ptr<Driver> drv)
{
}

void Dictionary::addDocument(std::istream &is)
{
}

std::set<Record> Dictionary::lookup(int char1, int char2) const
{
    return std::set<Record>();
}

void Dictionary::add(Record)
{
}

std::set<Position>
Dictionary::search(const std::string &text) const
{
    return std::set<Position>();
}

Record::Record(int char1, int char2, const Position &pos)
{
}

bool Position::operator==(const Position &pos) const
{
    return this == &pos || pos.docid_ == docid_ && pos.position_ == position_;
}

std::ostream& operator<<(std::ostream &os, const Position& pos)
{
    os << "Bigram::Position(" << pos.docid() << ", " << pos.position() << ")";
    return os;
}

std::vector<int> Bigram::disassemble(const std::string &text)
{
    return std::vector<int>();
}

