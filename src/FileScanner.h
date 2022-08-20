/*
 * Copyright 2015 Gary R. Van Sickle (grvs@users.sourceforge.net).
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

/** @file */

#ifndef FILESCANNER_H_
#define FILESCANNER_H_

#include <config.h>

#include <stdexcept>
#include <string>
#include <memory>
#include <functional>

#include "libext/FileID.h"
#include "sync_queue_impl_selector.h"
#include "MatchList.h"


extern "C" void* resolve_CountLinesSinceLastMatch(void);


/// The regular expression engines we support.
/// @note Which of these is supported depends on which libraries were available at compile-time.
enum class RegexEngine
{
	NONE,	//!< No engine available.
	CXX11, 	//!< C++11's built-in <regex> support.
	PCRE,	//!< The original libpcre.
	PCRE2,	//!< libpcre2

#if HAVE_LIBPCRE2
	DEFAULT = PCRE2,
#elif HAVE_LIBPCRE
	DEFAULT = PCRE,
#else
	DEFAULT = NONE,
#endif
};


/**
 * FileScanner will throw this if it runs into trouble compiling the regex.
 */
struct FileScannerException : public std::runtime_error
{
	FileScannerException(const std::string &message) : std::runtime_error(message) {};
};


/**
 * Base class for the classes which do the actual regex scanning of the file contents.
 */
class FileScanner
{
public:

	/**
	 * Factory Method for creating a new FileScanner-derived class.
	 *
	 * @param in_queue
	 * @param output_queue
	 * @param regex
	 * @param ignore_case
	 * @param word_regexp
	 * @param pattern_is_literal
	 * @param engine
	 * @return
	 */
	static std::unique_ptr<FileScanner> Create(sync_queue<std::shared_ptr<FileID>> &in_queue,
			sync_queue<MatchList> &output_queue,
			std::string regex,
			bool ignore_case,
			bool word_regexp,
			bool pattern_is_literal,
			RegexEngine engine = RegexEngine::DEFAULT);

public:
	FileScanner(sync_queue<std::shared_ptr<FileID>> &in_queue,
			sync_queue<MatchList> &output_queue,
			std::string regex,
			bool ignore_case,
			bool word_regexp,
			bool pattern_is_literal);
	virtual ~FileScanner();

	virtual void ThreadLocalSetup(int thread_count) { (void)thread_count; };

	void Run(int thread_index);

protected:

	/// @name Member-Function Pseudo-Multiversioning
	/// All this mechanism is to support something along the lines of gcc's function multiversioning,
	/// which doesn't work prior to gcc 4.9, at all on Cygwin even when the compiler/binutils support it,
	/// and doesn't exist on OSX/clang.  And I didn't even bother looking at the *BSDs.
	/// @{

	friend void* ::resolve_CountLinesSinceLastMatch(void);

	/// The member function pointer which will be set at runtime to point to the best function version.
	static size_t (*CountLinesSinceLastMatch)(const char * __restrict__ prev_lineno_search_end,
				const char * __restrict__ start_of_current_match) noexcept;

	//__attribute__((target("default")))
	static size_t CountLinesSinceLastMatch_default(const char * __restrict__ prev_lineno_search_end,
			const char * __restrict__ start_of_current_match) noexcept;

	//__attribute__((target("sse4.2", "popcnt")))
	static size_t CountLinesSinceLastMatch_sse4_2_popcnt(const char * __restrict__ prev_lineno_search_end,
			const char * __restrict__ start_of_current_match) noexcept;

	//__attribute__((target("sse4.2", "no-popcnt")))
	static size_t CountLinesSinceLastMatch_sse4_2_no_popcnt(const char * __restrict__ prev_lineno_search_end,
				const char * __restrict__ start_of_current_match) noexcept;

	//__attribute__((target("sse2")))
	static size_t CountLinesSinceLastMatch_sse2(const char * __restrict__ prev_lineno_search_end,
					const char * __restrict__ start_of_current_match) noexcept;


	bool ConstructCodeUnitTable(const uint8_t *pcre2_bitmap) noexcept;
	void ConstructRangePairTable() noexcept;

	using FindFirstPossibleCodeUnit_type = std::function<const char *(const FileScanner&, const char * __restrict__ cbegin, size_t len)>;

	static FindFirstPossibleCodeUnit_type resolve_FindFirstPossibleCodeUnit();

	FindFirstPossibleCodeUnit_type FindFirstPossibleCodeUnit;

	const char * FindFirstPossibleCodeUnit_default(const char * __restrict__ cbegin, size_t len) const noexcept;

	const char * FindFirstPossibleCodeUnit_sse4_2(const char * __restrict__ cbegin, size_t len) const noexcept;

	//friend void* ::resolve_find_first_of(void);
	//__attribute__((target("default")))
	const char * find_first_in_ranges_sse4_2_popcnt(const char * __restrict__ cbegin, size_t len) const noexcept;

	const char * find_first_of_default(const char * __restrict__ cbegin, size_t len) const noexcept;
	const char * find_first_of_sse4_2_no_popcnt(const char * __restrict__ cbegin, size_t len) const noexcept;
	const char * find_first_of_sse4_2_popcnt(const char * __restrict__ cbegin, size_t len) const noexcept;

	const char * find_sse4_2_no_popcnt(const char * __restrict__ cbegin, size_t len) const noexcept;
	const char * find_sse4_2_popcnt(const char * __restrict__ cbegin, size_t len) const noexcept;

	/**
	 * Member function pointer to the multiversioned LiteralMatch function.
	 *
	 * @param file_data
	 * @param file_size
	 * @param start_offset
	 * @param ovector
	 * @return
	 */
	int (FileScanner::*LiteralMatch)(const char *file_data, size_t file_size, size_t start_offset, size_t *ovector) const noexcept;

	/**
	 * Runtime resolver function for the LiteralMatch function.
	 */
	static decltype(LiteralMatch) resolve_LiteralMatch(FileScanner *obj) noexcept;

	int LiteralMatch_default(const char *file_data, size_t file_size, size_t start_offset, size_t *ovector) const noexcept;

	int LiteralMatch_sse4_2(const char *file_data, size_t file_size, size_t start_offset, size_t *ovector) const noexcept;

	///@}

	/**
	 * Analyzes the given @c regex and returns true if it's a literal string.
	 *
	 * @returns  true if @c regex is literal.
	 */
	[[nodiscard]] static bool IsPatternLiteral(const std::string &regex) noexcept;

	/**
	 *
	 * @param regex
	 * @return
	 */
	static uint8_t GetLiteralPrefixLen(const std::string &regex) noexcept;


	/// The original regex (as a std::string) passed in during construction.
	std::string m_regex;

	bool m_ignore_case;

	bool m_word_regexp;

	bool m_pattern_is_literal;

	/// 256-byte array used to match the first possible code unit.
	alignas(16) uint8_t m_compiled_cu_bitmap[256];

	/// 1+index of last valid value in m_compiled_cu_bitmap.
	uint16_t m_end_fpcu_table {0};

	/// Array used to match character ranges.
	alignas(16) uint8_t m_compiled_range_bitmap[256];

	/// One past the end of the last valid value in m_compiled_range_bitmap.
	uint16_t m_end_ranges_table {0};

	/**
	 * The literal string to search for.  This string has been allocated by overaligned_alloc(), and so must be
	 * deallocated by std::free().  We package it up in a std::unique_ptr<> with a custom deleter to prevent leaks.
	 */
	std::unique_ptr<uint8_t,void(*)(void*)> m_literal_search_string { nullptr, std::free };
	size_t m_literal_search_string_len {0};

	/// Flag set by regex analysis if matching should use m_literal_search_string as the full literal string to match.
	bool m_use_literal {false};

	/// Flag set by regex analysis if matching should use m_literal_search_string as the literal prefix of a larger regular expression.
	bool m_use_lit_prefix {false};

private:

	/**
	 * Helper to assign each thread which starts Run() to a different core.
	 * Note that this currently only works on Linux, and does not appear to make
	 * a measurable difference in performance, likely because we're I/O bound regardless.
	 *
	 * Maintaining this for experimental purposes.
	 */
	void AssignToNextCore();

	/**
	 * Scan @a file_data for matches of the regex.  Add hits to @a ml.
	 *
	 * @param file_data
	 * @param file_size
	 * @param ml
	 */
	virtual void ScanFile(int thread_index, const char * __restrict__ file_data, size_t file_size, MatchList &ml) = 0;

	sync_queue<std::shared_ptr<FileID>>& m_in_queue;

	sync_queue<MatchList> &m_output_queue;

	int m_next_core;

	bool m_use_mmap;

	/**
	 * Switch to make Run() assign its std::thread to different cores on the machine.
	 * If false, the underlying std::thread logic is allowed to decide which threads run on
	 * which cores.  See note above for AssignToNextCore(); turning this on makes no real difference.
	 *
	 * Maintaining this for experimental purposes.
	 */
	bool m_manually_assign_cores;
};

#endif /* FILESCANNER_H_ */
