/***************************************************************************
 *   Copyright (C) 2009-2011 by Francesco Biscani                          *
 *   bluescarni@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "../src/base_series.hpp"

#define BOOST_TEST_MODULE base_series_test
#include <boost/test/unit_test.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <stdexcept>

#include "../src/debug_access.hpp"
#include "../src/echelon_descriptor.hpp"
#include "../src/integer.hpp"
#include "../src/numerical_coefficient.hpp"
#include "../src/polynomial_term.hpp"
#include "../src/symbol.hpp"

// TODO: test merge_terms with negative sign.

using namespace piranha;

typedef boost::mpl::vector<numerical_coefficient<double>,numerical_coefficient<integer>> cf_types;
typedef boost::mpl::vector<unsigned,integer> expo_types;

struct construction_tag {};

namespace piranha
{

template <>
struct debug_access<construction_tag>
{
	template <typename Cf>
	struct runner
	{
		template <typename Expo>
		void operator()(const Expo &)
		{
			typedef polynomial_term<Cf,Expo> term_type;
			typedef typename term_type::key_type key_type;
			typedef base_series<term_type,void> series_type;
			typedef echelon_descriptor<term_type> ed_type;
			ed_type ed;
			ed.template add_symbol<term_type>(symbol("x"));
			// Default constructor.
			BOOST_CHECK(series_type().empty());
			BOOST_CHECK_EQUAL(series_type().size(), (unsigned)0);
			// Copy constructor.
			series_type s;
			s.insert(term_type(1,key_type{Expo(1)}),ed);
			series_type t(s);
			BOOST_CHECK(*s.m_container.begin() == *t.m_container.begin());
			BOOST_CHECK(s.m_container.begin()->m_cf.get_value() == t.m_container.begin()->m_cf.get_value());
			// Move constructor.
			series_type u{series_type(s)};
			BOOST_CHECK(*u.m_container.begin() == *t.m_container.begin());
			BOOST_CHECK(u.m_container.begin()->m_cf.get_value() == t.m_container.begin()->m_cf.get_value());
			// Copy assignment.
			u = t;
			BOOST_CHECK(*u.m_container.begin() == *t.m_container.begin());
			BOOST_CHECK(u.m_container.begin()->m_cf.get_value() == t.m_container.begin()->m_cf.get_value());
			// Move assignment.
			u = std::move(t);
			BOOST_CHECK(*u.m_container.begin() == *s.m_container.begin());
			BOOST_CHECK(u.m_container.begin()->m_cf.get_value() == s.m_container.begin()->m_cf.get_value());			
		}
	};
	template <typename Cf>
	void operator()(const Cf &)
	{
		boost::mpl::for_each<expo_types>(runner<Cf>());
	}
};
}

typedef debug_access<construction_tag> constructor_tester;

BOOST_AUTO_TEST_CASE(base_series_constructor_test)
{
	boost::mpl::for_each<cf_types>(constructor_tester());
}

struct insertion_tag {};

namespace piranha
{
template <>
class debug_access<insertion_tag>
{
	public:
		template <typename Cf>
		struct runner
		{
			template <typename Expo>
			void operator()(const Expo &)
			{
				typedef polynomial_term<Cf,Expo> term_type;
				typedef typename term_type::key_type key_type;
				typedef base_series<term_type,void> series_type;
				typedef echelon_descriptor<term_type> ed_type;
				ed_type ed;
				ed.template add_symbol<term_type>(symbol("x"));
				// Insert well-behaved term.
				series_type s;
				s.insert(term_type(1,key_type{Expo(1)}),ed);
				BOOST_CHECK(!s.empty());
				BOOST_CHECK_EQUAL(s.size(),unsigned(1));
				// Insert incompatible term.
				BOOST_CHECK_THROW(s.insert(term_type(1,key_type{}),ed),std::invalid_argument);
				BOOST_CHECK_EQUAL(s.size(),unsigned(1));
				// Insert ignorable term.
				s.insert(term_type(0,key_type{Expo(1)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(1));
				// Insert another new term.
				s.insert(term_type(1,key_type{Expo(2)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(2));
				// Insert equivalent terms.
				s.insert(term_type(2,key_type{Expo(2)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(2));
				s.template insert<false>(term_type(-2,key_type{Expo(2)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(2));
				// Insert terms that will prompt for erase of a term.
				s.insert(term_type(-2,key_type{Expo(2)}),ed);
				s.insert(term_type(-2,key_type{Expo(2)}),ed);
				s.insert(term_type(-1,key_type{Expo(2)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(1));
				// Insertion of different term type.
				typedef polynomial_term<numerical_coefficient<float>,Expo> other_term_type;
				s.insert(other_term_type(1,key_type{Expo(1)}),ed);
				BOOST_CHECK_EQUAL(s.size(),unsigned(1));
				BOOST_CHECK_EQUAL(s.m_container.begin()->m_cf.get_value(),Cf(1).get_value() + float(1));
			}
		};
		template <typename Cf>
		void operator()(const Cf &)
		{
			boost::mpl::for_each<expo_types>(runner<Cf>());
		}
};
}

typedef debug_access<insertion_tag> insertion_tester;

BOOST_AUTO_TEST_CASE(base_series_insertion_test)
{
	boost::mpl::for_each<cf_types>(insertion_tester());
}

struct merge_terms_tag {};

namespace piranha
{
template <>
class debug_access<merge_terms_tag>
{
	public:
		template <typename Cf>
		struct runner
		{
			template <typename Expo>
			void operator()(const Expo &)
			{
				typedef polynomial_term<Cf,Expo> term_type;
				typedef typename Cf::type value_type;
				typedef typename term_type::key_type key_type;
				typedef base_series<term_type,void> series_type;
				typedef echelon_descriptor<term_type> ed_type;
				ed_type ed;
				ed.template add_symbol<term_type>(symbol("x"));
				series_type s1, s2;
				s1.insert(term_type(1,key_type{Expo(1)}),ed);
				s2.insert(term_type(2,key_type{Expo(2)}),ed);
				// Merge with copy.
				s1.template merge_terms<true>(s2,ed);
				BOOST_CHECK_EQUAL(s1.size(),unsigned(2));
				auto it = s1.m_container.begin();
				BOOST_CHECK(it->m_cf.get_value() == 1 || it->m_cf.get_value() == 2);
				++it;
				BOOST_CHECK(it->m_cf.get_value() == 1 || it->m_cf.get_value() == 2);
				// Merge with move.
				series_type s3;
				s3.insert(term_type(3,key_type{Expo(3)}),ed);
				s1.template merge_terms<true>(std::move(s3),ed);
				BOOST_CHECK(s3.empty());
				BOOST_CHECK_EQUAL(s1.size(),unsigned(3));
				// Merge with self.
				s1.template merge_terms<true>(s1,ed);
				BOOST_CHECK_EQUAL(s1.size(),unsigned(3));
				it = s1.m_container.begin();
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) || it->m_cf.get_value() == value_type(3) + value_type(3));
				++it;
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) || it->m_cf.get_value() == value_type(3) + value_type(3));
				++it;
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) || it->m_cf.get_value() == value_type(3) + value_type(3));
				// Merge with self + move.
				s1.template merge_terms<true>(std::move(s1),ed);
				BOOST_CHECK_EQUAL(s1.size(),unsigned(3));
				it = s1.m_container.begin();
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) + value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) + value_type(2) + value_type(2) ||
					it->m_cf.get_value() == value_type(3) + value_type(3) + value_type(3) + value_type(3));
				++it;
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) + value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) + value_type(2) + value_type(2) ||
					it->m_cf.get_value() == value_type(3) + value_type(3) + value_type(3) + value_type(3));
				++it;
				BOOST_CHECK(it->m_cf.get_value() == value_type(1) + value_type(1) + value_type(1) + value_type(1) ||
					it->m_cf.get_value() == value_type(2) + value_type(2) + value_type(2) + value_type(2) ||
					it->m_cf.get_value() == value_type(3) + value_type(3) + value_type(3) + value_type(3));
				// Merge with different series type.
				s1.m_container.clear();
				s1.insert(term_type(1,key_type{Expo(1)}),ed);
				typedef polynomial_term<numerical_coefficient<long>,Expo> term_type2;
				typedef typename term_type2::key_type key_type2;
				typedef base_series<term_type2,void> series_type2;
				typedef echelon_descriptor<term_type2> ed_type2;
				ed_type2 ed2;
				ed2.template add_symbol<term_type2>(symbol("x"));
				series_type2 s4;
				s4.insert(term_type2(1,key_type2{Expo(1)}),ed2);
				s1.template merge_terms<true>(s4,ed);
				BOOST_CHECK_EQUAL(s1.size(), unsigned(1));
				it = s1.m_container.begin();
				value_type tmp(1);
				tmp += long(1);
				BOOST_CHECK(it->m_cf.get_value() == tmp);
				s4.m_container.clear();
				s4.insert(term_type2(1,key_type2{Expo(2)}),ed2);
				s1.template merge_terms<true>(s4,ed);
				it = s1.m_container.begin();
				BOOST_CHECK_EQUAL(s1.size(), unsigned(2));
				BOOST_CHECK(it->m_cf.get_value() == tmp || it->m_cf.get_value() == long(1));
				++it;
				BOOST_CHECK(it->m_cf.get_value() == tmp || it->m_cf.get_value() == long(1));
			}
		};
		template <typename Cf>
		void operator()(const Cf &)
		{
			boost::mpl::for_each<expo_types>(runner<Cf>());
		}
};
}

typedef debug_access<merge_terms_tag> merge_terms_tester;

BOOST_AUTO_TEST_CASE(base_series_merge_terms_test)
{
	boost::mpl::for_each<cf_types>(merge_terms_tester());
}

struct merge_args_tag {};

namespace piranha
{
template <>
class debug_access<merge_args_tag>
{
	public:
		template <typename Cf>
		struct runner
		{
			template <typename Expo>
			void operator()(const Expo &)
			{
				typedef polynomial_term<Cf,Expo> term_type;
				typedef typename Cf::type value_type;
				typedef typename term_type::key_type key_type;
				typedef base_series<term_type,void> series_type;
				typedef echelon_descriptor<term_type> ed_type;
				series_type s;
				ed_type ed1, ed2;
				s.insert(term_type(1,key_type{}),ed1);
				ed2.template add_symbol<term_type>(symbol("x"));
				auto merge_out = s.merge_args(ed1,ed2);
				BOOST_CHECK_EQUAL(merge_out.size(),unsigned(1));
				BOOST_CHECK(merge_out.m_container.find(term_type(1,key_type{Expo(0)})) != merge_out.m_container.end());
				auto compat_check = [](const series_type &series, const ed_type &ed) -> void {
					for (auto it = series.m_container.begin(); it != series.m_container.end(); ++it) {
						BOOST_CHECK(it->is_compatible(ed));
					}
				};
				compat_check(merge_out,ed2);
				s = std::move(merge_out);
				s.insert(term_type(1,key_type{Expo(1)}),ed2);
				s.insert(term_type(2,key_type{Expo(2)}),ed2);
				ed1 = ed2;
				ed2.template add_symbol<term_type>(symbol("y"));
				merge_out = s.merge_args(ed1,ed2);
				BOOST_CHECK_EQUAL(merge_out.size(),unsigned(3));
				BOOST_CHECK(merge_out.m_container.find(term_type(1,key_type{Expo(0),Expo(0)})) != merge_out.m_container.end());
				BOOST_CHECK(merge_out.m_container.find(term_type(1,key_type{Expo(1),Expo(0)})) != merge_out.m_container.end());
				BOOST_CHECK(merge_out.m_container.find(term_type(2,key_type{Expo(2),Expo(0)})) != merge_out.m_container.end());
				compat_check(merge_out,ed2);
			}
		};
		template <typename Cf>
		void operator()(const Cf &)
		{
			boost::mpl::for_each<expo_types>(runner<Cf>());
		}
};
}

typedef debug_access<merge_args_tag> merge_args_tester;

BOOST_AUTO_TEST_CASE(base_series_merge_args_test)
{
	boost::mpl::for_each<cf_types>(merge_args_tester());
}
