#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <map>
#include <vector>
#include <set>
#include <unistd.h>
#include <functional>
#include <omp.h>


using namespace std;

//Optimierungspotenzial Vector zum iterieren und 


typedef set<string> Dictionary;

struct Password
{
    string salt, cyphertext, plaintext;

    Password(){} //wozu?!?!

    Password(const string & hash)
      : salt(hash.substr(0,2))
    , cyphertext(hash.substr(2))
    {}
};

typedef map<string, Password> UserPasswords;

bool read_line_by_line(const string & filename, function<bool (string)> callback)
{
    ifstream input(filename);
    string line;
    while (getline(input, line))
    {
        if (! callback(line))
            return false;
    }
    return true;
};

bool read_password_file(
    const string & filename, // in
    UserPasswords & passwords) // out
{
    return read_line_by_line(filename, [&passwords](const string & line)
    {
        istringstream line_as_stream(line);

        string username;
        if (! getline(line_as_stream, username, ':'))
        {
            cerr << "wrong line format, could not read username" << endl;
            return false;
        }

        string hash_part_of_line;
        if (! getline(line_as_stream, hash_part_of_line, '\r'))
        {
            cerr << "wrong line format, could not read password" << endl;
            return false;
        }

        passwords[username] = Password(hash_part_of_line);
        return true;
    });
}

bool read_dictionary_file(
    const string & filename, // in
    Dictionary & dictionary) // out
{
    return read_line_by_line(filename, [&dictionary](const string & line)
    {
        dictionary.insert(line);
        return true;
    });
}

void write_decrypted_passwords(
    const UserPasswords & passwords)
{
    auto passwords_decrypted = 0;
    ofstream output("output.txt");
    for (auto & entry : passwords)
    {
        auto & password = entry.second;
        if (! password.plaintext.empty())
        {
            auto & username = entry.first;
            output << username << ';' << password.plaintext << endl;
            passwords_decrypted += 1;
        }
    }
    cout << "Decrypted " << passwords_decrypted << " passwords" << endl;
}


void set_found_passwords (string username,string password,UserPasswords & passwords)
{
  passwords[username].plaintext = password;

}


void crack_passwords(UserPasswords &passwords, Dictionary &pw_dictionary)
  {

   int found_pass_count=0; // Können abbrechen sobald 2 Passwörter geefunden

   for (int i=0;i<passwords.size();i++) //damit parallelisierbar
      {
     for (int j=0;j<pw_dictionary.size();j++)
       {
            map<string, Password>::iterator pass_user_it=next(passwords.begin(),i);
            set<string>::iterator dict_it=next(pw_dictionary.begin(),j);

            string crypted_dict_entry=crypt((const char *)&passwords[pass_user_it->first].salt,(const char *) &pw_dictionary); // Doesn't work yet get Pointer how to change to string ?!

             if(strcmp(crypted_dict_entry,(passwords[pass_user_it->first].salt+passwords[pass_user_it->first].cyphertext))
                {
                 passwords[pass_user_it->first].plaintext =*dict_it;
                 found_pass_count++;

                 break;
                 }
             else
                {
                for(int k=0;k<=9;k++)
                 {
                 string passconcat = *dict_it + to_string(k);
                 crypted_dictionary_entry =  crypt((const char *)&passwords[pass_user_it->first].salt,(const char *) &passconcat);

                 if(strcmp(crypted_dict_entry,passwords[pass_user_it->first].cyphertext))
                    {
                      passwords[pass_user_it->first].plaintext = passconcat;
                      found_pass_count++;
                      break; //wie aus zweiter Schelife dei geschachtelt ist raus kommen
                    }

                 }
                }
         }
        }
   if(found_pass_count==2) //Wehn two password found exit Function
     return; 

  }





int main(int argc, char** argv)
{
    if (argc < 3)
    {
        cerr << "usage: decrypt <password_file> <dictionary_file>" << endl;
        return -1;
    }


    UserPasswords passwords_by_username;
    Dictionary possible_keys;
    map<string, Password>::iterator pass_user_it = passwords_by_username.begin();
   // set<string>::iterator dict_it= possible_keys.begin();
    int passwortcounter;
 
    bool ok = read_password_file(argv[1], passwords_by_username);
    if (! ok || passwords_by_username.empty())
    {
        cerr << "could not read password_file" << endl;
        return -1;
    }
    cout << "PasswordDatabase contains " << passwords_by_username.size() << " users" << endl;

    ok = read_dictionary_file(argv[2], possible_keys);
    if (! ok || possible_keys.empty())
    {
        cerr << "could not read dictionary_file" << endl;
        return -1;
    }
    cout << "Dictionary contains " << possible_keys.size() << " possible keys" << endl;

    //Define Iterators for Map and Dictionary
    //
    //
    //

   // string encrypt = crypt(((const string &) dict_it),"iu");
   //               cout << *dict_it <<" " << encrypt << "\n" << (&dict_it)  << endl;




    // decrypt passwords and write decrypted passwords into password.plain_text wie kommt das Ergebnis in die File?!?!
    write_decrypted_passwords(passwords_by_username);
    return 0;
}
