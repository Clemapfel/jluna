// 
// Copyright 2022 Clemens Cords
// Created on 22.02.22 by clem (mail@clemens-cords.com)
//

#pragma once

#include <julia.h>

#include <include/type.hpp>
#include <include/proxy.hpp>

namespace jluna
{
    /// @brief customizable wrapper for non-julia type T
    /// @note for information on how to use this class, visit https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
    template<typename T>
    class Usertype
    {
        template<typename U>
        static inline std::function<void(T&, U)> noop_set = [](T&, U) {return;};

        public:
            /// @brief original type
            using original_type = T;

            Usertype() = delete;

            /// @brief enable interface
            /// @param name
            static void enable(const std::string&);

            /// @brief is enabled
            /// @returns bool
            static bool is_enabled();

            /// @brief add field
            /// @param name: julia-side name of field
            /// @param type: type of symbol. User the other overload if the type is a typevar, such as "P" (where P is a parameter)
            /// @param box_get: lambda with signature (T&) -> Any*
            /// @param unbox_set: lambda with signature (T&, Any*)
            /// @note this function will throw if called after implement()
            template<typename Field_t>
            static void add_property(
                const std::string& name,
                std::function<Field_t(T&)> box_get,
                std::function<void(T&, Field_t)> unbox_set = noop_set<Field_t>
            );

            ///
            static void implement(Module module = Main);

            static bool is_implemented();

            /// @brief box interface
            static Any* box(T&);

            /// @brief unbox interface
            static T unbox(Any*);

        private:
            static inline bool _enabled = false;
            static inline bool _implemented = false;

            static inline std::unique_ptr<Type> _type = std::unique_ptr<Type>(nullptr);
            static inline std::unique_ptr<Symbol> _name = std::unique_ptr<Symbol>(nullptr);

            static inline std::vector<Symbol> _fieldnames_in_order = {};
            static inline std::map<Symbol, std::tuple<
                std::function<Any*(T&)>,        // getter
                std::function<void(T&, Any*)>,   // setter
                Type
            >> _mapping = {};
    };

    /// @brief exception thrown when usertype is used before being implemented
    template<typename T>
    struct UsertypeNotEnabledException : public std::exception
    {
        public:
            /// @brief ctor
            UsertypeNotEnabledException();

            /// @brief what
            /// @returns message
            const char* what() const noexcept override final;

        private:
            std::string _msg;
    };
}

#include ".src/usertype.inl"

function set_xor(set::Set{T}) where T

	res = undef
	front::Bool = true
	for e in set
		if front
			res = e
			front = false
		else
			res = Base.xor(res, e)
		end
	end
	return res
end

function solve(set_in::Set, k::Integer) ::Set{Set}

	pool = Vector{Set{Set}}()
	push!(pool, Set())

	# seed the pool with all sets of size 1
	for n in set_in
		push!(pool[1], Set([n]))
	end

	# grow sets to generate all possible sets of size k-1
	index = 2
	while index <= k-1

		push!(pool, Set{Set}())
		for set in pool[index-1]
			for n in set_in
				push!(pool[index], union(set, Set([n])))
			end
		end
		index += 1
	end

	# reject some k-1 because:
	# For a set s = {s1, s2, ..., s3} such that xor(s) != 0, m = xor(s): xor(union(s, m)) = 0
	out = Set{Set}()
	for set in pool[length(pool)]

		xor_result = set_xor(set)
		if !(xor_result in set) && (xor_result in set_in)
			push!(out, union(set, Set([xor_result])))
		end
	end

	return out
end

# usage:
solve(Set([x for x in 1:256]), 5)