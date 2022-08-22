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

/** @file */

#ifndef TYPEMANAGER_H_
#define TYPEMANAGER_H_

#include <config.h>

#include <future/string_view.hpp>
#include <libext/string.hpp>

#include <iosfwd>
#include <tuple>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>


/**
 * TypeManager will throw this in certain circumstances.
 */
struct TypeManagerException : public std::runtime_error
{
	explicit TypeManagerException(const std::string &message) : std::runtime_error(message) {};
};

/**
 * Class which manages the file types which are to be scanned.
 */
class TypeManager
{
public:
	TypeManager();
	~TypeManager() = default;

	using name_string_type = std::string;

	/**
	 * Determine if the file with the given @p name should be scanned based on the
	 * enabled file types.
	 *
	 * @param name
	 * @return true if file should be scanned, false otherwise.
	 */
	bool FileShouldBeScanned(const name_string_type &name) const noexcept;

	/**
	 * Add the given file type to the types which will be scanned.  For handling the
	 * --type= command line param.  The first time this function is called, all currently-
	 * active types are removed, and only the given type (and types given subsequently)
	 * will be searched.
	 *
	 * @param type_name  Name of the type to enable.
	 * @return true on success, false if no such type.
	 */
	bool type(const std::string &type_name);

	/**
	 * Remove the given file type from the types which will be scanned.  For handling the
	 * --notype= command line param.  Adds the type to the m_removed_type_filters map, and
	 * removes it from the m_active_type_map map.
	 *
	 * @param type_name  Name of the type to disable.
	 * @return true on success, false if no such type.
	 */
	bool notype(const std::string &type_name);

	/**
	 * Determines if the given string #name_or_prefix matches one or more type names.
	 * Returns a list of the matching complete type name strings.
	 * Used during argument parsing.  If no matches, arg is unrecognized.  If more than one match,
	 * arg is ambiguous.
	 *
	 * @param name_or_prefix
	 * @return
	 */
	std::vector<std::string> GetMatchingTypenameList(const std::string &name_or_prefix);

	/**
	 * Determines if @p type is in the m_active_type_map as a type name.
	 *
	 * @param type  Name of the type to check.
	 * @return true if @p type names a type, false otherwise.
	 */
	bool IsType(const std::string &type) const;

	/**
	 * Adds a new filter spec to a (possibly new) @p type, based on @p filter_spec_string.
	 *
	 * @param delete_type_first   If true, treat as a "--type-set=", and delete any existing file type spec first.
	 * @param filter_spec_string
	 * @exception TypeManagerException  filter_spec_string cannot be parsed.
	 */
	void TypeAddFromFilterSpecString(bool delete_type_first, const std::string &filter_spec_string);

	/**
	 * Adds and then notype()s a new filter spec to the anonymous type used for '--ignore-file=', based on @p filter_spec_string.
	 * This is for use by the --ignore-file=FILTER:FILTERARGS command-line option.
	 *
	 * @param filter_spec_string
	 * @exception TypeManagerException  filter_spec_string cannot be parsed.
	 */
	void TypeAddIgnoreFileFromFilterSpecString(const std::string &filter_spec_string);

	/**
	 * Adds and then type()s a new filter spec to the anonymous type used for '--include=glob', based on @p filter_spec_string.
	 *
	 * @param filter_spec_string
	 * @exception TypeManagerException  filter_spec_string cannot be parsed.
	 */
	void TypeAddIncludeGlobFromFilterSpecString(const std::string &filter_spec_string);

	/**
	 * Deletes @p type from the m_active_type_map.
	 *
	 * @param type  Name of the type to delete.
	 * @return true if @p type named a type, false if it did not.
	 */
	bool TypeDel(const std::string &type);

	void CompileTypeTables();

	void PrintTypesForHelp(std::ostream &s) const;

private:

	void TypeAddIs(const std::string &type, const std::string &name);

	void TypeAddExt(const std::string &type, const std::string &ext);

	void TypeAddGlobExclude(const std::string &type, const std::string &glob);

	void TypeAddGlobInclude(const std::string &type, const std::string &glob);

	bool IsExcludedByAnyGlob(const std::string &name) const noexcept;

	/// Flag to keep track of the first call to type().
	bool m_first_type_has_been_seen = { false };

	/// Map of file type names to the associated filename filters.
	/// This contains both built-in types and user-defined types created by TypeAdd*().
	/// Both built-in and user types can be removed with TypeDel().
	std::map<std::string, std::vector<std::string>> m_builtin_and_user_type_map;

	/// Map of file type names to the associated filename filters.
	/// This is the active map, which will eventually be compiled into the
	/// hash tables used at filename-scanning time.
	std::map<std::string, std::vector<std::string>> m_active_type_map;

	/// Map of file type filters which have been removed by calls to notype().
	/// Maps to the type(s) which were specified in the notype() call(s).
	std::unordered_multimap<std::string, std::string> m_removed_type_filters;

	/// @name Compiled Type Tables
	/// These are the data structures used at directory tree traversal time to quickly
	/// determine whether a file should be scanned or not.  They are "compiled" by a call to
	/// CompileTypeTables() after all config file and command-line processing is complete.
	/// @{

	/// File extensions in microstring format, which can be binary searched quickly.
	std::vector<microstring> m_fast_include_extensions;

	/// File extensions which will be examined.  Maps to file type.
	std::unordered_multimap<std::string, std::string> m_include_extensions;

	/// Literal filenames which will be examined.  Maps to file type.
	std::unordered_multimap<std::string, std::string> m_included_literal_filenames;

	/// Vector of glob patterns to check for exclusion.
	/// This is separate from m_include_exclude_globs because only the exclude globs need to be
	/// checked if a previous file type check has determined that a file is to be included.
	std::vector<std::string> m_exclude_globs;

	/// Vector of glob patterns to check for inclusion/exclusion.
	/// These are in command-line order so that a sequence of them such as the following can be
	/// processed correctly:
	/// ... --include='*.cpp' --exclude='*.c*' --include='*.cpp' ...
	/// The bool is true if include, false if exclude.
	std::vector<std::pair<std::string, bool>> m_include_exclude_globs;

	/// Map of the regexes to try to match to the first line of the file (key) to
	/// the file type (value).
	std::unordered_multimap<std::string, std::string> m_included_first_line_regexes;

	///@}
};

#endif /* TYPEMANAGER_H_ */
