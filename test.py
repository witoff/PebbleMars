import time
import re
# conversion from UTC to local mean solar time at Gale Crater

def qm(utc):
    m2e = 1.02749125 # 1 mars second = 
    Sol0_Sec = time.mktime(time.strptime("2012-218T13:49:59","%Y-%jT%H:%M:%S")) # Sol-00000M00:00:00 at Gale Crater
    print 'sol 0 seconds: ', Sol0_Sec
    utc_sec = time.mktime(time.strptime(utc,"%Y-%jT%H:%M:%S"))
    difft_earth = utc_sec-Sol0_Sec
    difft_mars = difft_earth/m2e
    days = difft_mars/86400
    seconds_left = difft_mars%86400
    hours = seconds_left/3600
    seconds_lefth = seconds_left%3600
    minutes = seconds_lefth/60
    seconds_leftm = seconds_lefth%60
    # strip decimal values
    ds = re.split('\.',str(days))[0]
    hs = re.split('\.',str(hours))[0]
    ms = re.split('\.',str(minutes))[0]
    ss = re.split('\.',str(seconds_leftm))[0]
    #pad leading 0's
    while len(ds) < 2: ds = "0"+ds
    while len(hs) < 2: hs = "0"+hs
    while len(ms) < 2: ms = "0"+ms
    while len(ss) < 2: ss = "0"+ss
    return "Sol-"+ds+"M"+hs+":"+ms+":"+ss

qm("2013-8T12:12:12")