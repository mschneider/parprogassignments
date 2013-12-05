#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <map>
#include <set>

#include <functional>

struct password_hash
{
    std::string salt, cyphertext;

    password_hash(){}

    password_hash(const std::string & hash)
    : salt(hash.substr(0,2))
    , cyphertext(hash.substr(2))
    {}
};

bool read_line_by_line(const std::string & filename, std::function<bool (std::string)> callback)
{
    std::ifstream input(filename);
    std::string line;
    while (std::getline(input, line))
    {
        if (! callback(line))
            return false;
    }
    return true;
};

bool read_password_file(
    const std::string & filename, // in
    std::map<std::string, password_hash> & password_hashes_by_username) // out
{
    return read_line_by_line(filename, [&password_hashes_by_username](const std::string & line)
    {
        std::istringstream line_as_stream(line);

        std::string username;
        if (! std::getline(line_as_stream, username, ':'))
        {
            std::cerr << "wrong line format, could not read username" << std::endl;
            return false;
        }

        std::string hash_part_of_line;
        if (! std::getline(line_as_stream, hash_part_of_line, '\r'))
        {
            std::cerr << "wrong line format, could not read password_hash" << std::endl;
            return false;
        }

        password_hashes_by_username[username] = password_hash(hash_part_of_line);
        return true;
    });
}

bool read_dictionary_file(
    const std::string & filename, // in
    std::set<std::string> & possible_keys) // out
{
    return read_line_by_line(filename, [&possible_keys](const std::string & line)
    {
        possible_keys.insert(line);
        return true;
    });
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "usage: decrypt <password_file> <dictionary_file>" << std::endl;
        return -1;
    }

    std::map<std::string, password_hash> password_hashes_by_username;
    std::set<std::string> possible_keys;
    bool ok = read_password_file(argv[1], password_hashes_by_username);
    if (! ok)
    {
        std::cerr << "could not read password_file" << std::endl;
        return -1;
    }

    ok = read_dictionary_file(argv[2], possible_keys);
    if (! ok)
    {
        std::cerr << "could not read dictionary_file" << std::endl;
        return -1;
    }

    return 0;
}
