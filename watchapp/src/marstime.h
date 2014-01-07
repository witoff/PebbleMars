#include <pebble.h>
#include <math.h>

#define YEAR 31536000. //3600*24*365
#define M2E 1.02749125 // Mars to Earth Translation
	
// time() returns tz shifted time, the local offset is synced via JS.
#define DEFAULT_TZ_OFFSET 28800 // 8 * 3600. 
#define PERSIST_TZ_KEY_TTL 3600 // 1 hour

//MSL Landing Timeeference)
//	Curiosity (Gale Crater): 2012-218T13:49:59 = Sol-00000M00:00:00 (sol 0 reference)
//		SolRef = 0
//		msl 	1344174599.0; 
float getMslEpoch() { 
	return 1344174599.0; 
}

int getTZOffset() {
	bool refresh_tz = 0;
	int tz_offset = DEFAULT_TZ_OFFSET;
	
	if (persist_exists(PERSIST_TZ_TTL_KEY)) {
		// Check TTL of key.
		int32_t tz_age = (time(NULL) - persist_read_int(PERSIST_TZ_TTL_KEY));
		if ((int)tz_age >= PERSIST_TZ_KEY_TTL) {
			APP_LOG(APP_LOG_LEVEL_INFO, "TZ offset key has expired.");
			refresh_tz = 1;
		}
		if (persist_exists(PERSIST_TZ_KEY)) {
			// Use saved TZ offset.
			tz_offset = (int)persist_read_int(PERSIST_TZ_KEY);
		}
	} else {
		// TTL is not in storage yet so initialize it.
		APP_LOG(APP_LOG_LEVEL_WARNING, "Requesting TZ Offset for the first time.");
		refresh_tz = 1;
	}
	
	if (refresh_tz == 1) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Requesting updated TZ Offset");
	    Tuplet tuplet = TupletInteger(KEY_TZ_OFFSET, 0);
	    send_app_message(send_uint8, &tuplet);
	}

	return tz_offset;
}

void getMarsTimeString(char *str, int len, float epoch, bool use_seconds) {
	time_t time_rover = epoch;
	time_t utc_sec = time(NULL);
	int tz_offset = getTZOffset();

    int difft_earth = utc_sec-time_rover+tz_offset;
    int difft_mars = difft_earth/M2E;
    int days = floor(difft_mars/86400);
    int seconds_left = difft_mars%86400;
    int hours = seconds_left/3600;
    int seconds_lefth = seconds_left%3600;
    int minutes = seconds_lefth/60;
    int seconds_leftm = seconds_lefth%60;
    // strip decimal values
	
	if (use_seconds) {
		snprintf(str, len, 
			"Sol-%iM%02i:%02i:%02i",
			days, hours, minutes, seconds_leftm);
	} else {
		snprintf(str, len, 
			"Sol-%iM%02i:%02i",
			days, hours, minutes);
	}
}