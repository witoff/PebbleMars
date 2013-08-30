
void getStr(char * str, int len) {
    snprintf(str, len, "MARS TIME!");
}

typedef struct tm tm_t;

void getMslEpoch(tm_t *epoch) {
    //strptime("2012-218T13:49:59", "%Y-%jT%H:%M:%S", &epoch);
    //time_t t;
    epoch->tm_sec = 59;
    epoch->tm_min = 49;
    epoch->tm_hour = 13;
    //epoch.tm_mday = 1;   
    //epoch.tm_mon = 6;     
    epoch->tm_year = 2012 - 1900; 
    //epoch.tm_wday = 
    epoch->tm_yday = 218;
    epoch->tm_isdst = 0;
}

void getTimeSinceLandingString(char *str, int len) {
	tm_t epoch;
	getMslEpoch(&epoch);
	
	//time_t t;
	//t = mktime(&epoch);
	
	time_t rawtime;
    struct tm * now;
    time ( &rawtime );
    now = gmtime ( &rawtime );

	//TODO: Better comparison
	int years = now->tm_year - epoch.tm_year;
	int days = now->tm_yday - epoch.tm_yday;
	int hours = now->tm_hour - epoch.tm_hour;
	int mins = now->tm_min - epoch.tm_min;
	int secs = now->tm_sec - epoch.tm_sec;


	snprintf(str, len, "%i Year, %i Days, %i:%i:%i", years, days, hours, mins, secs);

	//tm_t now;
	//localtime(&now);

    //strftime(str, len, "%Y-%jT%H:%M:%S", &epoch);
  	//snprintf(str, len, "MARS TIME:  %i", t);
}

void getMslEpochString(char *str, int len) {
	tm_t epoch;
	getMslEpoch(&epoch);
	strftime(str, len, "%Y-%jT%H:%M:%S", &epoch);
}

void getCurrentTimeString(char *str, int len) {
	time_t rawtime;
    struct tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );
    // rawtime 	-> time_t
    // ptm 		-> tm
    strftime(str, len, "%Y-%jT%H:%M:%S", ptm);
}

void getMarsTime(char * str, int len) {
    time_t rawtime;
    struct tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );

    //float m2e = 1.02749125; //1 mars second = 1.02… earth seconds
    //Sol-00000M00:00:00 at Gale Crater
    //Sol0_Sec = time.mktime(
    //	time.strptime("2012-218T13:49:59","%Y-%jT%H:%M:%S")
    //	);

    snprintf(str, len, "MARS TIME! %i", ptm->tm_sec);
}

/*
void getMslEpoch() {
	
	struct tm epoch;
    //time_t t;

    epoch.tm_sec = 59;
    epoch.tm_min = 49;
    epoch.tm_hour = 13;
    //epoch.tm_mday = 1;   
    //epoch.tm_mon = 6;
    epoch.tm_year = 2012 - 1900;
    //epoch.tm_wday = 
    epoch.tm_yday = 218;
    epoch.tm_isdst = 0;         
    t = mktime( &epoch );

    //return t;
}
*/