#ifndef BIGRAM_H
#define BIGRAM_H

namespace Bigram
{
    class Position {
    public:
        Position(const std::string &docid, unsigned int position)
            : docid_(docid), position_(position) {}
        bool operator==(const Position &pos) const;
        bool operator<(const Position &pos) const;
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
        bool operator==(const Record &rec) const;
        bool operator<(const Record &rec) const;
        int first() const {return first_;}
        int second() const {return second_;}
        const Position& position() const {return position_;}
    private:
        Record();
        int first_;
        int second_;
        Position position_;
    };

    class Driver {
    public:
        virtual void add(const Record &rec) = 0;
        virtual std::set<Record> lookup(int char1, int char2) const = 0;
    };
    class MemoryDriver : public Driver {
    public:
        void add(const Record &rec);
        std::set<Record> lookup(int char1, int char2) const;
    private:
        std::set<Record> records_;
    };
    class SQLiteDriver : public Driver {
    public:
        SQLiteDriver(const std::string &filename) {}
        void add(const Record &rec);
        std::set<Record> lookup(int char1, int char2) const;
    private:
    };

    class Dictionary {
    public:
        Dictionary(std::shared_ptr<Driver> drv);
        Dictionary();
        void addDocument(std::istream &is);
        std::set<Record> lookup(int char1, int char2) const;
        void add(const Record &rec);
        std::set<Position> search(const std::string &text) const;

    private:
        std::shared_ptr<Driver> driver_;
    };

    std::vector<int> disassemble(const std::string &text);
}

std::ostream& operator<<(std::ostream &os, const Bigram::Position& pos);
std::ostream& operator<<(std::ostream &os, const Bigram::Record& pos);

#endif // BIGRAM_H
