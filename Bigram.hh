#ifndef BIGRAM_H
#define BIGRAM_H

namespace Bigram
{
    class Record {
    public:
        Record(int char1, int char2, const std::string &fileid, unsigned int position);
    private:
        Record();
    };

    class Dictionary {
    public:
        std::set<Record> lookup(int char1, int char2) const;
        void add(Record);
    };

    std::vector<int> disassemble(const std::string &text);
    inline bool operator< (const Record &a, const Record &b) {
        return true;
    }
}

#endif // BIGRAM_H
