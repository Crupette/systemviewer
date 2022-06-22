#ifndef CSV_HPP
#define CSV_HPP 1

#include <tuple>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

namespace csv {

template<char T>
struct csv_delim : std::ctype<char> {
    csv_delim() : std::ctype<char>(get_table()) {}
    static mask const *get_table() {
        static mask rc[table_size];
        rc[T] = std::ctype_base::space;
        rc['\n'] = std::ctype_base::space;
        return &rc[0];
    }
};

template<char Seperator, typename... Vals>
class CSVFile {
public:
    struct CSVLine {
        std::tuple<Vals...> data;

        CSVLine(const std::string &line) {
            std::istringstream in(line);
            in.imbue(std::locale(std::locale(), new csv_delim<Seperator>));
            read_elements(in, std::make_index_sequence<sizeof...(Vals)>{});
        }

        template<std::size_t... I>
        void read_elements(std::istream &in, std::index_sequence<I...>) {
            std::initializer_list<bool>{read_element(in, std::get<I>(data))...};
        }

        template<typename T>
        bool read_element(std::istream &in, T &value) {
            in >> value; return true;
        }
    };
    struct Reader {
    private:
        std::vector<CSVLine> m_lines;
    public:
        Reader() {}
        Reader(CSVFile *parent) {
            for(std::string line; std::getline(parent->m_rfile, line);) {
                m_lines.emplace_back(line);
            }
        }
        std::vector<std::tuple<Vals...>> get() const {
            std::vector<std::tuple<Vals...>> r;
            for(auto &line : m_lines) r.push_back(line.data);
            return r;
        }

    };
    struct Writer {
    private:
        std::vector<std::tuple<Vals...>> m_data;
    public:
        void put(std::tuple<Vals...> line) { m_data.push_back(line); }
        void write(CSVFile *parent) {
            for(auto &line : m_data) {
                write_line(parent, line, std::make_index_sequence<sizeof...(Vals)>{});
                parent->m_wfile.seekp(-1, std::ios_base::end);
                parent->m_wfile << "\n";
            }
        }
        template<std::size_t... I>
        void write_line(CSVFile *parent, std::tuple<Vals...> line, std::index_sequence<I...>) {
            std::initializer_list<bool>{write_element(parent, std::get<I>(line))...};
        }
        template<typename T>
        bool write_element(CSVFile *parent, T &value) {
            parent->m_wfile << value << Seperator; return true;
        }
    };

    friend struct Reader;
    friend struct Writer;

    CSVFile(const std::string &path, bool write = false) {
        //std::ios_base::iostate emask = m_file.exceptions() | std::ios::failbit;
        if(!write) {
            m_rfile.open(path);
            m_reader = Reader(this);
        }else{
            m_wfile.open(path);
        }
    }
    ~CSVFile() { m_rfile.close(); m_wfile.close(); }

    const std::tuple<Vals...> operator[](std::size_t i) { return m_reader.m_lines[i].data; }
    std::size_t size() const { return m_reader.m_lines.size(); }

    std::vector<std::tuple<Vals...>> get() const { return m_reader.get(); }
    void put(std::tuple<Vals...> vals) { m_writer.put(vals); }
    void write() { m_writer.write(this); }

protected:
    std::ifstream m_rfile;
    std::ofstream m_wfile;
    Reader m_reader;
    Writer m_writer;
};

}

#endif
