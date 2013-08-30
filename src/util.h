#include <math.h>

#define YEAR 31536000. //3600*24*365

void getStr(char * str, int len) {
    snprintf(str, len, "MARS TIME!");
}

typedef struct tm tm;

//MSL Landing Timeeference)
//	Curiosity (Gale Crater): 2012-218T13:49:59 = Sol-00000M00:00:00 (sol 0 reference)
//		msl 	1344199799.0
//	Spirit (Gusev Crater): 2006-299T00:44:02 = Sol-1000M00:00:00 (sol 1000 reference)
//		spirit 	1161848643.0
//	Opportunity (Meridiani Planum): 2006-320T02:16:46 = Sol-1000M00:00:00 (sol 1000 r
//		oppy 	1163672206.0
float getMslEpoch() { return 1344199799.0; }
float getSpiritEpoch() { return 1161848643.0; }
float getOppEpoch() { return 1163672206.0; }


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
	secs -= d->mins *60;
	d->secs = floor(secs);
}

/*
	Call like:
	getDurationString(text,  35, getMslEpoch());
*/
void getDurationString(char *str, int len, float epoch) {
	time_t time_msl = epoch;
	time_t now = time(NULL);
	//int now_int = now;
	delta d;
	getDeltaTm(now, time_msl, &d);

	snprintf(str, len, 
		"%i y %i d, %i:%i:%i",
		d.years, d.days, d.hours, d.mins, d.secs);
}

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