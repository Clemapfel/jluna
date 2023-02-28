// 
// Copyright 2022 Clemens Cords
// Created on 17.01.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <map>

namespace jluna::detail
{
    /// @brief test API used for JLUNA_TEST, not intended for end-users
    struct Test
    {
        static inline std::map<std::string, std::string> _failed = {};
        static inline std::mutex _mutex = std::mutex();

        class AssertionException : public std::exception
        {
            public:
                AssertionException(const std::string& s)
                    : message(s)
                {}

                const char* what() const noexcept final
                {
                    return message.c_str();
                }

            private:
                std::string message;
        };

        template<typename Lambda_t>
        static void test(const std::string& name, Lambda_t&& lambda)
        {
            std::cout << name << ": ";

            std::cout.setstate(std::ios_base::failbit);
            std::cerr.setstate(std::ios_base::failbit);

            bool failed = false;
            std::string what;
            try
            {
                lambda();
            }
            catch (const AssertionException&)
            {
                failed = true;
                what = "Test::assertion failed";
            }
            catch (const std::exception& e)
            {
                failed = true;
                what = e.what();
            }

            std::cout.clear();
            std::cerr.clear();

            if (not failed)
                std::cout << "[OK]";
            else
            {
                std::cout << "[FAILED]";
                _mutex.lock();
                Test::_failed.insert({name, what});
                _mutex.unlock();
            }

            std::cout << std::endl;
        }

        static void initialize()
        {
            std::cout << "starting test...\n" << std::endl;
            _failed = std::map<std::string, std::string>();
        }

        static bool conclude()
        {
            std::cout << std::endl;
            std::cout << "Number of tests unsuccessful: " << _failed.size() << std::endl;

            for (auto& pair : _failed)
            {
                std::cout << "__________________________________\n";
                std::cout << "| " << pair.first << ": \n|\n";
                std::cout << "| " << pair.second << "\n";
                std::cout << "|_________________________________\n\n" << std::endl;
            }

            return _failed.empty();
        }

        static void assert_that(bool b)
        {
            if (not b)
                throw AssertionException("");
        }

        template<typename Exception_t, typename Lambda_t>
        static void assert_that_throws(Lambda_t lambda)
        {
            bool thrown = false;
            try
            {
                lambda();
            }
            catch (Exception_t&)
            {
                thrown = true;
            }

            Test::assert_that(thrown);
        }
    };
}