// Copyright 2018 Your Name <your_email>

#include <header.hpp>
#include <boost/filesystem.hpp>

struct Date {
    int year;
    int month;
    int day;
    std::string name;
};

std::map<std::string, std::map<int, std::list<Date>>> data;

void process(boost::filesystem::path &p, std::regex &regex) {
    try {
        if (boost::filesystem::is_directory(p)) {
            for (auto &p1 : boost::filesystem::directory_iterator(p)) {
                auto i = p1.path();
                process(i, regex);
            }
        } else if (boost::filesystem::is_regular_file(p)) {
            if (std::regex_match(p.filename().string(), regex)) {
                std::string number = p.stem().string().substr(8, 8);
                int num = std::stoi(number);
                std::string date = p.stem().string().substr(17, 8);
                std::string broker = p.parent_path().filename().string();
                int year = std::stoi(date.substr(0, 4));
                int month = std::stoi(date.substr(4, 2));
                int day = std::stoi(date.substr(6, 2));
                if (data.find(broker) != data.end()) {
                    if (data[broker].find(num) != data[broker].end()){
                    data[broker][num].emplace_back(
                            Date{
                                year, month, day, date
                            });
                    } else {
                        data[broker].emplace(num, std::list<Date>{
                            Date{
                                Date{year, month, day, date}
                        }});
                    }
                } else {
                    auto map = std::map<int, std::list<Date>>();
                    map.emplace(num, std::list<Date>{
                        Date{
                            year, month, day, date
                        }
                    });
                    data.emplace(broker, map);
                }
            }
        } else if (boost::filesystem::is_symlink(p)) {
            auto i = boost::filesystem::read_symlink(p);
            process(i, regex);
        }
    }
    catch (const boost::filesystem::filesystem_error &ex){
        std::cerr << ex.what();
    }
}

int main(int argc, char* argv[]){
    auto regEx = R"~(balance_\d{8}_\d{8}\.txt)~";
    std::regex regex = std::regex(regEx);
    std::string path_to_ftp;
    data = std::map<std::string, std::map<int, std::list<Date>>>();
    if (argc > 1) {
        path_to_ftp = argv[1];
    } else {
        path_to_ftp = "..";
    }
    boost::filesystem::path p(path_to_ftp);
    process(p, regex);
    for (auto &a : data) {
        for (auto &b : a.second) {
            auto max = *std::max_element(b.second.begin(),
                    b.second.end(), [](const Date& one, const Date& two){
                return (one.year - two.year) * 365 +
                (one.month - two.month) * 30 + one.day - two.day;
            });
            std::cout << "broker:" << a.first << " account:"
            << b.first << " files:" << b.second.size()
            << " lastdate:" << max.name << std::endl;
        }
    }
}
