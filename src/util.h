#include <math.h>

#define YEAR 31536000. //3600*24*365
#define M2E 1.02749125 // Mars to Earth Translation
void getStr(char * str, int len) {
    snprintf(str, len, "MARS TIME!");
}

typedef struct tm tm;

//MSL Landing Timeeference)
//	Curiosity (Gale Crater): 2012-218T13:49:59 = Sol-00000M00:00:00 (sol 0 reference)
//		SolRef = 0
//		msl 	1344199799.0
//	Spirit (Gusev Crater): 2006-299T00:44:02 = Sol-1000M00:00:00 (sol 1000 reference)
//		SolRef = 1000
//		spirit 	1161848643.0
//	Opportunity (Meridiani Planum): 2006-320T02:16:46 = Sol-1000M00:00:00 (sol 1000 r
//		SolRef = 1000
//		oppy 	1163672206.0
float getMslEpoch() { return 1344199799.0; }
float getSpiritEpoch() { return 1161848643.0 + 1000*86400; }
float getOppEpoch() { return 1163672206.0 + 1000*86400; }


/* Returns Formatted string from landing TM */
void getLandingString(char *str, int len) {
	time_t time_msl = getSpiritEpoch();
    struct tm *time_msl_tm = localtime(&time_msl);
	strftime(str, len, "%Y-%jT%H:%M:%S", time_msl_tm);
}

struct delta { 
	int years;
	int days;
	int hours;
	int mins;
	int secs;
};
typedef struct delta delta;

void getDeltaTm(time_t now, time_t epoch, delta *d) {
	int secs = (int)(now - epoch);

	d->years = floor(secs/YEAR);
	secs -= d->years*YEAR;
	d->days = floor(secs/3600./24.);
	secs -= d->days*3600*24;
	d->hours = floor(secs/3600.);
	secs -= d->hours * 3600;
	d->mins = floor(secs/60.);
	secs -= d->mins*60;
	d->secs = (int)(now - epoch) % 60;
}

void getDelta(float epoch, delta *d) {
	time_t time_rover = epoch;
	time_t now = time(NULL);
	//int now_int = now;
	getDeltaTm(now, time_rover, d);
}

/*
	Call like:
	getDurationString(txtTop, txtBottom,  35, getMslEpoch());
*/
void getDurationString(char *strTop, char *strBottom, int len, float epoch) {
	time_t time_msl = epoch;
	time_t now = time(NULL);
	//int now_int = now;
	delta d;
	getDeltaTm(now, time_msl, &d);

	snprintf(strTop, len, 
		"%iy %id",
		d.years, d.days);
	snprintf(strBottom, len, 
		"%i:%i:%i",
		d.hours, d.mins, d.secs);
}

/*
void getMarsTimeString(char *strTop, char *strBottom, int len, float epoch) {
	time_t time_rover = epoch;
	time_t now = time(NULL);
	int SolRef = 0;

    int difft_earth = now - time_rover;
    int difft_mars = difft_earth/m2e + SolRef*86400;

    int days = math.floor(difft_mars/86400)
    int seconds_left = difft_mars%86400
    int hours = math.floor(seconds_left/3600)
    seconds_lefth = seconds_left%3600
    minutes = math.floor(seconds_lefth/60)
    seconds_leftm = seconds_lefth%60

	//int now_int = now;
	delta d;
	getDeltaTm(now, time_msl, &d);

	snprintf(strTop, len, 
		"%iy %id",
		d.years, d.days);
	snprintf(strBottom, len, 
		"%i:%i:%i",
		d.hours, d.mins, d.secs);
}*/

///////
///////
///////

void getMslEpochString(char *str, int len) {
	tm epoch;
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