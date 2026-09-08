/*
 * Common.hpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

// retrowave.ru died; retrowave-radio.ru is the successor and speaks a different
// API (see API.md). Only the sources layer knows these - nothing else in the app
// should mention a host.
#define RWR_ROOT_ENDPOINT "https://retrowave-radio.ru"
#define RWR_API_ENDPOINT "https://retrowave-radio.ru/api/v1"

// How many tracks one "load more" asks for. The whole catalogue is ~94 tracks,
// so this is four scrolls' worth, not a paging window.
#define RWR_PAGE_LIMIT 25

#define IMAGES "/data/images"
#define TRACKS "/data/tracks"
#define FAVORITE_TRACKS "/data/favourite.json"

// Startup tracing is DEVELOPMENT-only (see the BBT_ENV block in Retrowavers.pro).
// It stays in the tree rather than being deleted: when the app once died before
// main() - no log, no dump, nothing - this trace was the only thing that could
// have said where. In a production build it compiles away to nothing.
#ifdef RW_PRODUCTION
#define RW_TRACE(msg) do {} while (0)
#else
#define RW_TRACE(msg) qDebug() << msg
#endif

#endif /* COMMON_HPP_ */
