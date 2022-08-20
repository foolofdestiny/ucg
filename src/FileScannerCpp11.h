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

#ifndef FILESCANNERCPP11_H_
#define FILESCANNERCPP11_H_

#include <config.h>

#include "FileScanner.h"

#include <regex>

/*
 *
 */
class FileScannerCpp11: public FileScanner
{
public:
	FileScannerCpp11(sync_queue<std::shared_ptr<FileID>> &in_queue,
			sync_queue<MatchList> &output_queue,
			std::string regex,
			bool ignore_case,
			bool word_regexp,
			bool pattern_is_literal);
	~FileScannerCpp11() override;

private:

	/**
	 * Scan @a file_data for matches of m_pcre_regex using the C++11 library's <regex> support.  Add hits to @a ml.
	 *
	 * @param file_data
	 * @param file_size
	 * @param ml
	 */
	void ScanFile(int thread_index, const char * __restrict__ file_data, size_t file_size, MatchList &ml) final;
};

#endif /* FILESCANNERCPP11_H_ */
