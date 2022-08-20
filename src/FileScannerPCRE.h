/*
 * Copyright 2016 Gary R. Van Sickle (grvs@users.sourceforge.net).
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

#ifndef SRC_FILESCANNERPCRE_H_
#define SRC_FILESCANNERPCRE_H_

#include <config.h>

#include "FileScanner.h"

#if HAVE_LIBPCRE
#include <pcre.h>
#endif

/*
 *
 */
class FileScannerPCRE: public FileScanner
{
public:
	FileScannerPCRE(sync_queue<std::shared_ptr<FileID>> &in_queue,
			sync_queue<MatchList> &output_queue,
			std::string regex,
			bool ignore_case,
			bool word_regexp,
			bool pattern_is_literal);
	~FileScannerPCRE() override;


private:

	/**
	 * Scan @a file_data for matches of m_pcre_regex using libpcre.  Add hits to @a ml.
	 *
	 * @param file_data
	 * @param file_size
	 * @param ml
	 */
	void ScanFile(int thread_index, const char * __restrict__ file_data, size_t file_size, MatchList &ml) final;

#if HAVE_LIBPCRE
	/// The compiled libpcre regex.
	pcre *m_pcre_regex;

	/// The results of pcre_study()ing m_pcre_regex.
	pcre_extra *m_pcre_extra;
#endif
};

#endif /* SRC_FILESCANNERPCRE_H_ */
