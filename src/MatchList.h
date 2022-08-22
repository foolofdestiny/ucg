/*
 * Copyright 2015-2016 Gary R. Van Sickle (grvs@users.sourceforge.net).
 *
 * This file is part of UniversalCodeGrep.
 *
 * UniversalCodeGrep is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU General Public License as published by the Free
 * Software Foundation.
 *
 * UniversalCodeGrep is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * UniversalCodeGrep.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file MatchList.h */

#ifndef MATCHLIST_H_
#define MATCHLIST_H_

#include <config.h>

#include <string>
#include <vector>
#include <iosfwd>

#include "Match.h"
#include "OutputContext.h"

/**
 * Container class for holding all Matches found in a given file.
 * For performance reasons, this class is only move constructible and assignable, not copy constructible or assignable.
 * We'll be passing many instances of this class "by value" through the sync_queue<>s, and we want to make sure that it's
 * using the move constructors and assignment operators to do so.
 */
class MatchList
{
public:
	MatchList() = default;

	/// Delete the copy constructor and the move assignment operator.  With the std::vector<Match> in here, this is an expensive
	/// class to copy, so we only allow move-constructing.
	MatchList(const MatchList &lvalue) = delete;
	MatchList& operator=(const MatchList &other) = delete;

	/// Since we deleted the copy constructor, we have to explicitly declare that we want the default move ctor and assignment op.
	MatchList(MatchList&&) noexcept = default;
	MatchList& operator=(MatchList&&) /* noexcept @todo See static_assert below. */ = default;

	/// Also use the default destructor.
	~MatchList() noexcept = default;

	/// When any matches are found, give the MatchList the given #filename before sending it to the next stage.
	/// Passing #filename by value because we're storing it.
	void SetFilename(std::string filename);

	/// Add a match to this MatchList.  Note that this is done by moving, not copying, the given %match.
	void AddMatch(Match &&match);

	void Print(std::ostream &sstrm, OutputContext &output_context) const;

	/// Returns a bool indicating whether the MatchList is empty.
	/// @note You might expect that this needs to indicate 'empty' after a move-from has occurred.
	/// That's not the case.  A moved-from object only has to be destructible, and the move and copy operations
	/// have to still work the same as they did before the move operation.
	[[nodiscard]] bool empty() const noexcept { return m_match_list.empty(); };

	void clear() noexcept;

	[[nodiscard]] std::vector<Match>::size_type GetNumberOfMatchedLines() const noexcept;

private:

	/// The filename where the Matches in this MatchList were found.
	std::string m_filename;

	/// The Matches found in this file.
	std::vector<Match> m_match_list;
};

// Require MatchList to be nothrow move constructible so that a container of them can use move on reallocation.
static_assert(std::is_nothrow_move_constructible<MatchList>::value == true, "MatchList must be nothrow move constructible");

// Require MatchList to be nothrow move assignable for similar reasons.
// @note I can't get MatchList to be nothrow move assignable because std::string isn't (though std::vector<Match> is).
// See explanation in Match.h, which has the same issue, as to why we care.
//static_assert(std::is_nothrow_move_assignable<MatchList>::value == true, "MatchList must be nothrow move assignable");

// Require MatchList to be move assignable.
static_assert(std::is_move_assignable<MatchList>::value == true, "MatchList must be move assignable");

#ifndef __COVERITY__ // Coverity can't handle this static_assert().

// Require MatchList to not be copy constructible, so that uses don't end up accidentally copying it instead of moving.
static_assert(std::is_copy_constructible<MatchList>::value == false, "MatchList must not be copy constructible");

#endif // __COVERITY__

// Require MatchList to not be copy assignable, so that uses don't end up accidentally copying it instead of moving.
static_assert(std::is_copy_assignable<MatchList>::value == false, "MatchList must not be copy assignable");

#endif /* MATCHLIST_H_ */
