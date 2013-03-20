#include <string>
#include <vector>
#include <set>

#include "Bigram.hh"

using namespace Bigram;

std::set<Record> Dictionary::lookup(int char1, int char2) const
{
    return std::set<Record>();
}

void Dictionary::add(Record)
{
}

Record::Record(int char1, int char2, const std::string &fileid, unsigned int position)
{
}

std::vector<int> Bigram::disassemble(const std::string &text)
{
    return std::vector<int>();
}

