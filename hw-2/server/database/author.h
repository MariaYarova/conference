#ifndef AUTHOR_H
#define AUTHOR_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class Author{
        private:
            std::string _first_name;
            std::string _last_name;
            std::string _email;
            std::string _login;

        public:

            static Author fromJSON(const std::string & str);

            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;
            const std::string &get_login() const;

            std::string &first_name();
            std::string &last_name();
            std::string &email();
            std::string &login();

            static Author read_by_login(std::string login);
            static std::vector<Author> read_all();
            static std::vector<Author> search(std::string first_name,std::string last_name, int shard);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;
    };
}

#endif