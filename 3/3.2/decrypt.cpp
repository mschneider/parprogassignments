#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <vector>

#include <functional>

extern "C" {
    #include <unistd.h>
    #include <openssl/des.h>
}

typedef std::vector<std::string> Dictionary;

struct User
{
    std::string name, salt, hash, plaintext;

    User(){}

    User(const std::string name, const std::string & hash)
    : name(name)
    , salt(hash.substr(0,2))
    , hash(hash)
    {}
};

typedef std::vector<User> UserPasswords;

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

        //if (username.compare("user906") == 0)
        passwords.emplace_back(username, hash_part_of_line);
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
        dictionary.emplace_back(line);
        return true;
    });
}

void write_decrypted_passwords(
    const UserPasswords & users)
{
    auto passwords_decrypted = 0;
    std::ofstream output("output.txt");
    for (auto & user : users)
    {
        if (! user.plaintext.empty())
        {
            output << user.name << ';' << user.plaintext << std::endl;
            passwords_decrypted += 1;
        }
    }
    std::cout << "Decrypted " << passwords_decrypted << " passwords" << std::endl;
}

inline bool check_password(
    const std::string & possible_password,
    const std::string & salt,
    const std::string & comparison_cyphertext)
{
    char buffer[16];
    DES_fcrypt(possible_password.c_str(), salt.c_str(), buffer);
    const bool was_password = comparison_cyphertext.compare(buffer) == 0;
    return was_password;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "usage: decrypt <password_file> <dictionary_file>" << std::endl;
        return -1;
    }

    UserPasswords user_passwords;
    Dictionary possible_keys;
    bool ok = read_password_file(argv[1], user_passwords);
    if (! ok || user_passwords.empty())
    {
        std::cerr << "could not read password_file" << std::endl;
        return -1;
    }
    std::cout << "PasswordDatabase contains " << user_passwords.size() << " users" << std::endl;

    ok = read_dictionary_file(argv[2], possible_keys);
    if (! ok || possible_keys.empty())
    {
        std::cerr << "could not read dictionary_file" << std::endl;
        return -1;
    }
    std::cout << "Dictionary contains " << possible_keys.size() << " possible keys" << std::endl;

    #pragma omp parallel for
    for (int i = 0; i < user_passwords.size(); ++i)
    {
        User & user = user_passwords[i];
        std::cout << "cracking user " << user.name << std::endl;
        for (const auto & possible_password : possible_keys)
        {
            if (check_password(possible_password, user.salt, user.hash))
            {
                user.plaintext = possible_password;
                std::cout << "found password of:" << user.name << "'" << possible_password << "'" << std::endl;
                goto next_user;
            }
            std::string longer_password = possible_password + " ";
            for (char i = '0'; i <= '9'; ++i)
            {
                longer_password[longer_password.size() - 1] = i;
                if (check_password(longer_password, user.salt, user.hash))
                {
                    user.plaintext = longer_password;
                    std::cout << "found password of:" << user.name << "'" << longer_password << "'" << std::endl;
                    goto next_user;
                }
            }
        }
next_user:
        ;
    }

    write_decrypted_passwords(user_passwords);
    return 0;
}
