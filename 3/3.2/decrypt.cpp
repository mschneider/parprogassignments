#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <map>
#include <set>

#include <functional>

extern "C" {
    #include <unistd.h>
}

typedef std::set<std::string> Dictionary;

struct Password
{
    std::string salt, cyphertext, plaintext;

    Password(){}

    Password(const std::string & hash)
    : salt(hash.substr(0,2))
    , cyphertext(hash.substr(2))
    {}
};

typedef std::map<std::string, Password> UserPasswords;

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
    UserPasswords & passwords) // out
{
    return read_line_by_line(filename, [&passwords](const std::string & line)
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
            std::cerr << "wrong line format, could not read password" << std::endl;
            return false;
        }

        passwords[username] = Password(hash_part_of_line);
//        std::cout << "u:'" << username << '\'' << username.size() << " s:'"
//                           << passwords[username].salt << '\'' <<  passwords[username].salt.size() << " c:'"
//                           << passwords[username].cyphertext << '\'' <<  passwords[username].cyphertext.size() << std::endl;
        return true;
    });
}

bool read_dictionary_file(
    const std::string & filename, // in
    Dictionary & dictionary) // out
{
    return read_line_by_line(filename, [&dictionary](const std::string & line)
    {
        //std::cout << "p:'" << line << '\'' << line.size() << std::endl;
        dictionary.insert(line);
        return true;
    });
}

void write_decrypted_passwords(
    const UserPasswords & passwords)
{
    auto passwords_decrypted = 0;
    std::ofstream output("output.txt");
    for (auto & entry : passwords)
    {
        auto & password = entry.second;
        if (! password.plaintext.empty())
        {
            auto & username = entry.first;
            output << username << ';' << password.plaintext << std::endl;
            passwords_decrypted += 1;
        }
    }
    std::cout << "Decrypted " << passwords_decrypted << " passwords" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "usage: decrypt <password_file> <dictionary_file>" << std::endl;
        return -1;
    }

    UserPasswords passwords_by_username;
    Dictionary possible_keys;
    bool ok = read_password_file(argv[1], passwords_by_username);
    if (! ok || passwords_by_username.empty())
    {
        std::cerr << "could not read password_file" << std::endl;
        return -1;
    }
    std::cout << "PasswordDatabase contains " << passwords_by_username.size() << " users" << std::endl;

    ok = read_dictionary_file(argv[2], possible_keys);
    if (! ok || possible_keys.empty())
    {
        std::cerr << "could not read dictionary_file" << std::endl;
        return -1;
    }
    std::cout << "Dictionary contains " << possible_keys.size() << " possible keys" << std::endl;

    // decrypt passwords and write decrypted passwords into password.plain_text
    for (auto & entry : passwords_by_username)
    {
        auto & password = entry.second;
        for (const auto & possible_password : possible_keys)
        {
            const auto cyphertext = crypt(possible_password.c_str(), password.salt.c_str()) + 2;
            //std::cout << "k:'" <<  possible_password << "'" <<  " c:'" << cyphertext << "'" << std::endl;
            if (password.cyphertext.compare(cyphertext) == 0)
            {
                password.plaintext = possible_password;
                std::cout << "found password:" << possible_password
                          << " of:" << entry.first
                          << " c1:" << password.cyphertext
                          << " c2:" << cyphertext << std::endl;
            }
        }
        std::cout << "could not find password of:" << entry.first << std::endl;
    }

    write_decrypted_passwords(passwords_by_username);
    return 0;
}
