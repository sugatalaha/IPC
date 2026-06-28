#include<iostream>
#include<fstream>
#include<chrono>
#include<iomanip>

using namespace std;
using namespace chrono;

string get_timestamp()
{
    auto now=system_clock::now();
    auto t=system_clock::to_time_t(now);
    tm *tm=localtime(&t);
    ostringstream oss;
    oss<<put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void logEvent(string message)
{
    fstream log("server.log",ios::app);
    log<<"[ "+get_timestamp()+" ]"<<" "<<message<<endl;
}

void signal_handler(int sig)
{
    fstream log("server.log", ios::app);
    log<<"[ "<<get_timestamp()<<" ]"<<" "<<"Server closes"<<endl;
    log.close();
    exit(0);
}