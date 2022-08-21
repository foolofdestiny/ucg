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

#include <config.h>

#include "DirInclusionManager.h"

// Std C++.
#include <string>
#include <array>


/**
 * Default directories which will be ignored.
 */
static constexpr std::array f_builtin_dir_excludes
{
	".bzr",
	".git",
	".hg",
	".metadata",
	".svn",
	"CMakeFiles",
	"CVS",
	"autom4te.cache",
	".deps",
	""
};

void DirInclusionManager::AddExclusions(const std::set<std::string>& exclusions)
{
	for(const auto& name : exclusions)
	{
		m_excluded_literal_dirs.insert(name);
	}
}

void DirInclusionManager::CompileExclusionTables()
{
	// Populate the exclusion set with the built-in defaults.
	size_t i = 0;
	std::string t;
	while(t = f_builtin_dir_excludes[i], !t.empty())
	{
		m_excluded_literal_dirs.insert(t);
		++i;
	}
}

bool DirInclusionManager::DirShouldBeExcluded(const std::string &name) const
{
	if(m_excluded_literal_dirs.find(name) != m_excluded_literal_dirs.end())
	{
		// This directory shouldn't be traversed.
		return true;
	}

	// No exclusion rules matched, descend into the directory.
	return false;
}
