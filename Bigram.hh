#ifndef BIGRAM_H
#define BIGRAM_H

namespace Bigram
{
    class Position {
    public:
        Position(const std::string &docid, unsigned int position)
            : docid_(docid), position_(position) {}
        bool operator==(const Position &pos) const;
        std::string docid() const {return docid_;}
        unsigned int position() const {return position_;}
    private:
        Position();
        const std::string docid_;
        unsigned int position_;
    };

    class Record {
    public:
        Record(int char1, int char2, const Position &pos);
    private:
        Record();
    };

    class Dictionary {
    public:
        std::set<Record> lookup(int char1, int char2) const;
        void add(Record);
        std::set<Position> search(const std::string &text) const;
    };

    std::vector<int> disassemble(const std::string &text);
    inline bool operator< (const Record &a, const Record &b) {
        return true;
    }
}

std::ostream& operator<<(std::ostream &os, const Bigram::Position& pos);

#endif // BIGRAM_H
